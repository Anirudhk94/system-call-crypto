obj-m += sys_cpenc.o

INC=/lib/modules/$(shell uname -r)/build/arch/x86/include

all: xhw1 xcpenc cpenc

xhw1: xhw1.c
	gcc -Wall -Werror -I$(INC)/generated/uapi -I$(INC)/uapi xhw1.c -g -o xhw1

xcpenc: xcpenc.c
	gcc -Wall -Werror -I$(INC)/generated/uapi -I$(INC)/uapi xcpenc.c -g -o xcpenc -lcrypto

cpenc:
	make -Wall -Werror -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f xhw1
