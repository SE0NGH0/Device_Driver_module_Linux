obj-m := char_dev.o
KDIR := $(HOME)/project/linux
PWD  := $(shell pwd)

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean
