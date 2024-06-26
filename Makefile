SOURCE := mymodule
obj-m += $(SOURCE).o
mymodule-y := main.o ff.o
ccflags-y := -Wall -O2
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
test:
	-sudo rmmod $(SOURCE) # put a — to tell make to ignore an error in case the module isn’t loaded
	sudo dmesg -C # Clear the kernel log without echo
	sudo insmod $(SOURCE).ko # Insert the module
	sudo dmesg # Display the kernel log
