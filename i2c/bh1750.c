/* bh1750_chrdrv.c */

#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>

#define BH1750_ADDR      0x23
#define CMD_POWER_ON     0x01
#define CMD_RESET        0x07
#define CMD_CONT_HIGHRES 0x10

#define DEVICE_NAME      "bh1750"
#define CLASS_NAME       "sensor"

struct bh1750_data {
    struct i2c_client *client;
    struct cdev cdev;
};

static dev_t dev_number;
static struct class *bh1750_class;

static int bh1750_read_lux(struct i2c_client *client, char *buf)
{
    u8 data[2];
    int ret;
    u16 raw;
    u32 lux_x100;

    /* 전원 ON, 리셋, 연속 고해상도 모드 */
    i2c_smbus_write_byte(client, CMD_POWER_ON);
    i2c_smbus_write_byte(client, CMD_RESET);
    i2c_smbus_write_byte(client, CMD_CONT_HIGHRES);
    msleep(180);

    ret = i2c_smbus_read_i2c_block_data(client,
                                        CMD_CONT_HIGHRES,
                                        2, data);
    if (ret < 0)
        return ret;

    raw = (data[0] << 8) | data[1];
    lux_x100 = (raw * 1000U + 6) / 12;

    return sprintf(buf, "%u.%02u\n",
                   lux_x100 / 100,
                   lux_x100 % 100);
}

static ssize_t bh1750_char_read(struct file *filp,
                                char __user *user_buf,
                                size_t count,
                                loff_t *ppos)
{
    struct bh1750_data *d = filp->private_data;
    char tmp[16];
    int len;

    if (*ppos > 0)
        return 0; /* EOF */

    len = bh1750_read_lux(d->client, tmp);
    if (len < 0)
        return len;

    if (copy_to_user(user_buf, tmp, len))
        return -EFAULT;

    *ppos += len;
    return len;
}

static int bh1750_char_open(struct inode *inode, struct file *filp)
{
    struct bh1750_data *d = container_of(inode->i_cdev,
                                         struct bh1750_data, cdev);
    filp->private_data = d;
    return 0;
}

static const struct file_operations bh1750_fops = {
    .owner   = THIS_MODULE,
    .open    = bh1750_char_open,
    .read    = bh1750_char_read,
};

static int bh1750_probe(struct i2c_client *client)
{
    struct bh1750_data *d;
    int ret;

    /* 드라이버 데이터 할당 */
    d = devm_kzalloc(&client->dev, sizeof(*d), GFP_KERNEL);
    if (!d)
        return -ENOMEM;
    d->client = client;

    /* 캐릭터 디바이스 번호 및 클래스 생성 */
    if (!bh1750_class) {
        ret = alloc_chrdev_region(&dev_number, 0, 1, DEVICE_NAME);
        if (ret < 0)
            return ret;

        bh1750_class = class_create(CLASS_NAME);
        if (IS_ERR(bh1750_class)) {
            unregister_chrdev_region(dev_number, 1);
            return PTR_ERR(bh1750_class);
        }
    }

    /* cdev 초기화 및 등록 */
    cdev_init(&d->cdev, &bh1750_fops);
    d->cdev.owner = THIS_MODULE;
    ret = cdev_add(&d->cdev, dev_number, 1);
    if (ret) {
        class_destroy(bh1750_class);
        unregister_chrdev_region(dev_number, 1);
        return ret;
    }

    /* /dev 노드 생성 */
    device_create(bh1750_class, NULL, dev_number, NULL, DEVICE_NAME);

    i2c_set_clientdata(client, d);
    dev_info(&client->dev, "%s: registered char device /dev/%s\n",
             DEVICE_NAME, DEVICE_NAME);
    return 0;
}

static void bh1750_remove(struct i2c_client *client)
{
    struct bh1750_data *d = i2c_get_clientdata(client);

    device_destroy(bh1750_class, dev_number);
    cdev_del(&d->cdev);
    class_destroy(bh1750_class);
    unregister_chrdev_region(dev_number, 1);
}

static const struct i2c_device_id bh1750_id[] = {
    { "bh1750", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, bh1750_id);

static struct i2c_driver bh1750_driver = {
    .driver   = { .name = "bh1750" },
    .probe    = bh1750_probe,
    .remove   = bh1750_remove,
    .id_table = bh1750_id,
};

module_i2c_driver(bh1750_driver);

MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("BH1750 I2C Light Sensor with /dev Interface");
MODULE_LICENSE("GPL");
