/*
 * Copyright (c) 2012 Xilinx, Inc.  All rights reserved.
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>

#define OLED_BASE 0x43C00000
#define DELAY 1000

int set_core_value(uint64_t offset, long value){
    int fd;
    uint64_t base;
    volatile uint8_t *mm;
    size_t page_size = ((size_t)getpagesize());
    uint64_t page_mask = ((uint64_t)(long)~(page_size - 1));

    fd = open("/dev/mem", O_RDWR | O_SYNC );
    if (fd < 0) {
        printf("open(/dev/mem) failed (%d)\n", errno);
        return -1;
    }

    base = offset & page_mask;
    offset &= ~page_mask;

    mm = mmap(NULL, page_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, base);
    if (mm == MAP_FAILED) {
        printf("mmap64(0x%x@0x%lx) failed (%d)\n",
                page_size, (long unsigned int) base, errno);
        goto map_err;
    }

    *(volatile uint32_t *)(mm + offset) = value;

    munmap((void *)mm, page_size);

map_err:
    close(fd);

    return 0;
}

unsigned get_core_value(uint64_t offset){
    int fd;
    void *ptr;
    unsigned page_addr, page_offset, value = 0;
    unsigned page_size=sysconf(_SC_PAGESIZE);

    fd=open("/dev/mem",O_RDONLY);
    if(fd<1) {
        printf("/dev/mem has not opened yet\n");
        return 0;
    }
    page_addr=(offset & ~(page_size-1));
    page_offset=offset-page_addr;

    ptr=mmap(NULL,page_size,PROT_READ,MAP_SHARED,fd,(offset & ~(page_size-1)));
    if((int)ptr==-1) {
        goto map_err;
    }

    value = *((unsigned *)(ptr+page_offset));

    munmap((void *)ptr, page_size);
map_err:    
    close(fd);

    return value;
}

static int int_seq [64];

void clear(void) {
	int i=0;
	for (i=0;i<=60; i=i+4)
		set_core_value(OLED_BASE + i, 0x00000000);

	set_core_value(OLED_BASE + 64, 1);
	usleep(10000);
	set_core_value(OLED_BASE + 64, 0);
	usleep(10000);
}

int print_char( char char_seq , unsigned int page, unsigned int position) {
	unsigned int i = 0, ascii_value, shifter;

	if (position > 15)		{
		printf(" Wrong position, position should be between (0-15).\n");
		return (0);
	}

	if (page < 0 || page > 3)
		return -1;

	ascii_value= (int) char_seq;
	ascii_value = ascii_value << ((position % 4) * 8);
	int_seq[(position - (position % 4)) + 16 * page] |= ascii_value;

	for (i = 0; i <= 60; i = i+4)
		set_core_value(OLED_BASE + i, int_seq[i]);
	
	set_core_value(OLED_BASE + 64, 1);
	usleep(10000);
	set_core_value(OLED_BASE + 64, 0);
	usleep(10000);
	return 1;
}

int print_message(char *start , unsigned int page) {
	unsigned int ln,i;
	char *char_pointer;

	char_pointer = start;
	ln = strlen(start);

	for (i=0; i<ln; i++){
		print_char(*char_pointer++, page, i);
	}
	return (1);
}

int main() {
    printf("Hello World\n");
    clear();
	print_message("OLED Demo",0);
	print_message("Vu Tang - VTTEK",2);
    return 0;
}
