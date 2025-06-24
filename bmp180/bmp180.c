// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/sysfs.h>
#include <linux/of.h>
#include <linux/of_device.h>

#define BMP180_REG_CALIB   0xAA
#define BMP180_REG_CTRL    0xF4
#define BMP180_REG_DATA    0xF6
#define BMP180_CMD_TEMP    0x2E
#define BMP180_CMD_PRESS   0x34

struct bmp180_data {
    struct i2c_client *client;
    struct mutex      lock;
    /* Calibration coefficients */
    s16  ac1, ac2, ac3, b1, b2, mb, mc, md;
    u16  ac4, ac5, ac6;
    int  oss;
};

static int bmp180_read_calib(struct bmp180_data *d)
{
    u8 buf[22];
    int ret;

    ret = i2c_smbus_read_i2c_block_data(d->client,
                                        BMP180_REG_CALIB,
                                        sizeof(buf), buf);
    if (ret < 0)
        return ret;

    d->ac1 = (buf[0] << 8) | buf[1];
    d->ac2 = (buf[2] << 8) | buf[3];
    d->ac3 = (buf[4] << 8) | buf[5];
    d->ac4 = (buf[6] << 8) | buf[7];
    d->ac5 = (buf[8] << 8) | buf[9];
    d->ac6 = (buf[10] << 8) | buf[11];
    d->b1  = (buf[12] << 8) | buf[13];
    d->b2  = (buf[14] << 8) | buf[15];
    d->mb  = (buf[16] << 8) | buf[17];
    d->mc  = (buf[18] << 8) | buf[19];
    d->md  = (buf[20] << 8) | buf[21];
    return 0;
}

static int bmp180_read_raw_temp(struct bmp180_data *d, int *ut)
{
    int ret;
    u8 msb, lsb;

    /* Send temperature measurement command */
    ret = i2c_smbus_write_byte_data(d->client,
                                    BMP180_REG_CTRL,
                                    BMP180_CMD_TEMP);
    if (ret < 0)
        return ret;
    msleep(5);

    /* Read MSB and LSB directly from DATA registers */
    msb = i2c_smbus_read_byte_data(d->client, BMP180_REG_DATA);
    lsb = i2c_smbus_read_byte_data(d->client, BMP180_REG_DATA + 1);
    if ((int)msb < 0 || (int)lsb < 0)
        return -EIO;

    *ut = (msb << 8) | lsb;
    return 0;
}

static int bmp180_read_raw_press(struct bmp180_data *d, int *up)
{
    int ret;
    u8 msb, lsb, xlsb;

    ret = i2c_smbus_write_byte_data(d->client,
                                    BMP180_REG_CTRL,
                                    BMP180_CMD_PRESS + (d->oss << 6));
    if (ret < 0)
        return ret;
    msleep(5 + (3 << d->oss));

    msb  = i2c_smbus_read_byte_data(d->client, BMP180_REG_DATA);
    lsb  = i2c_smbus_read_byte_data(d->client, BMP180_REG_DATA + 1);
    xlsb = i2c_smbus_read_byte_data(d->client, BMP180_REG_DATA + 2);
    if ((int)msb < 0 || (int)lsb < 0 || (int)xlsb < 0)
        return -EIO;

    *up = ((msb << 16) | (lsb << 8) | xlsb) >> (8 - d->oss);
    return 0;
}

static long bmp180_calc_temp(struct bmp180_data *d, int ut, int *t)
{
    long x1 = ((long)ut - d->ac6) * d->ac5 >> 15;
    long x2 = ((long)d->mc << 11) / (x1 + d->md);
    long b5 = x1 + x2;

    *t = (b5 + 8) >> 4;  /* 0.1°C unit */
    return b5;
}

static int bmp180_calc_press(struct bmp180_data *d, int up,
                             long b5, int *p)
{
    long b6 = b5 - 4000;
    long x1 = (d->b2 * (b6 * b6 >> 12)) >> 11;
    long x2 = (d->ac2 * b6) >> 11;
    long x3 = x1 + x2;
    long b3 = ((((long)d->ac1 * 4 + x3) << d->oss) + 2) >> 2;
    x1 = (d->ac3 * b6) >> 13;
    x2 = (d->b1 * (b6 * b6 >> 12)) >> 16;
    x3 = ((x1 + x2) + 2) >> 2;
    long b4 = (d->ac4 * (unsigned long)(x3 + 32768)) >> 15;
    long b7 = ((unsigned long)up - b3) * (50000 >> d->oss);
    long p_raw = (b7 < 0x80000000 ?
                  (b7 << 1) / b4 :
                  (b7 / b4) << 1);
    x1 = (p_raw >> 8) * (p_raw >> 8);
    x1 = (x1 * 3038) >> 16;
    x2 = (-7357 * p_raw) >> 16;
    *p = p_raw + ((x1 + x2 + 3791) >> 4);

    return 0;
}

/* sysfs show functions */
static ssize_t show_temp(struct device *dev,
                         struct device_attribute *attr,
                         char *buf)
{
    struct bmp180_data *d = dev_get_drvdata(dev);
    int ut, t, ret;
    long b5;

    mutex_lock(&d->lock);
    ret = bmp180_read_raw_temp(d, &ut);
    if (ret < 0)
        goto out;
    b5 = bmp180_calc_temp(d, ut, &t);
out:
    mutex_unlock(&d->lock);

    if (ret < 0)
        return ret;
    return sprintf(buf, "%d\n", t);
}
static DEVICE_ATTR(temp, 0444, show_temp, NULL);

static ssize_t show_press(struct device *dev,
                          struct device_attribute *attr,
                          char *buf)
{
    struct bmp180_data *d = dev_get_drvdata(dev);
    int ut, up, p, ret;
    long b5;

    mutex_lock(&d->lock);
    ret = bmp180_read_raw_temp(d, &ut);
    if (ret < 0)
        goto out;
    b5 = bmp180_calc_temp(d, ut, &p);
    ret = bmp180_read_raw_press(d, &up);
    if (ret < 0)
        goto out;
    ret = bmp180_calc_press(d, up, b5, &p);
out:
    mutex_unlock(&d->lock);

    if (ret < 0)
        return ret;
    return sprintf(buf, "%d\n", p);
}
static DEVICE_ATTR(pressure, 0444, show_press, NULL);

static struct attribute *bmp180_attrs[] = {
    &dev_attr_temp.attr,
    &dev_attr_pressure.attr,
    NULL,
};
static const struct attribute_group bmp180_group = {
    .attrs = bmp180_attrs,
};

static const struct of_device_id bmp180_of_match[] = {
    { .compatible = "bosch,bmp180" },
    { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, bmp180_of_match);

static const struct i2c_device_id bmp180_id[] = {
    { "bmp180", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, bmp180_id);

static int bmp180_probe(struct i2c_client *client)
{
    struct bmp180_data *d;
    int ret;

    d = devm_kzalloc(&client->dev,
                     sizeof(*d), GFP_KERNEL);
    if (!d)
        return -ENOMEM;

    mutex_init(&d->lock);
    d->client = client;
    d->oss    = 0;
    i2c_set_clientdata(client, d);

    ret = bmp180_read_calib(d);
    if (ret < 0) {
        dev_err(&client->dev,
                "calibration read failed: %d\n", ret);
        return ret;
    }

    ret = sysfs_create_group(&client->dev.kobj,
                             &bmp180_group);
    if (ret)
        dev_err(&client->dev,
                "sysfs group create failed: %d\n", ret);

    dev_info(&client->dev, "BMP180 sensor probed\n");
    return ret;
}

static void bmp180_remove(struct i2c_client *client)
{
    sysfs_remove_group(&client->dev.kobj,
                       &bmp180_group);
}

static struct i2c_driver bmp180_driver = {
    .driver = {
        .name           = "bmp180",
        .of_match_table = of_match_ptr(bmp180_of_match),
    },
    .probe    = bmp180_probe,
    .remove   = bmp180_remove,
    .id_table = bmp180_id,
};
module_i2c_driver(bmp180_driver);

MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("BMP180 pressure/temperature sensor driver");
MODULE_LICENSE("GPL v2");
