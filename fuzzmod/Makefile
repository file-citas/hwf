obj-m += fuzzmod.o
KDIR = /home/file/project/hwcodec/lineage/android_kernel_samsung_exynos7420
CROSS_COMPILE = /home/file/toolchains/gcc-linaro-4.9-2014.11-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-

all: fuzzmod.c
	make arch=arm64 -C $(KDIR) M=$(PWD) modules

install: fuzzmod.ko
	adb push fuzzmod.ko /sdcard/fm/

clean:
	make arch=arm64 -C $(KDIR) M=$(PWD) clean
