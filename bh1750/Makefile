obj-m := bh1750.o
KDIR := $(HOME)/project/linux
PWD  := $(shell pwd)

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean
