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
#include <getopt.h>
#include <string.h>
#include <iio.h>
#include <errno.h>
#include <time.h>

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
	usleep(1000);
	set_core_value(OLED_BASE + 64, 0);
	usleep(1000);
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

    /*Only update for current page and remain others pages*/
	for (i = page * 16; i <= page * 16 + 15; i = i+4)
		set_core_value(OLED_BASE + i, int_seq[i]);
	
	set_core_value(OLED_BASE + 64, 1);
	usleep(1000);
	set_core_value(OLED_BASE + 64, 0);
	usleep(1000);
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

int print_flag = 0, clear_flag = 0, header = 0, temp_flag = 0, vccint_flag = 0, line = 0, time_flag = 0;
char msg[15];

void usage(char **argv) {
    printf("%s's usage:\n", *argv);
    printf("    -h/--help: help\n");
    printf("    -p/--print [message]: print message to OLED\n");
    printf("    -l/--line [0/1]: specify line number for only -p command\n");
    printf("    -c/--clear: clear OLED\n");
    printf("    -f/--temp: show FPGA Temperature\n");
    printf("    -v/--vccint: show VCCINT (XADC Channel 0)\n");
    printf("    -t/--time: show time\n");
}

int handle_args(int argc, char **argv) {
    /*For getting agurments*/
    int c;

    while (1) {
        static struct option long_options[] = {
            {"clear",   no_argument,       0, 'c'},
            {"print",  required_argument,   0, 'p'},
            {"temp",  no_argument,   0, 'f'},
            {"help",  no_argument,   0, 'h'},
            {"vccint",  no_argument,   0, 'v'},
            {"line",  required_argument,   0, 'l'},
            {"time",  no_argument,   0, 't'},
            {0, 0, 0, 0}
        };
        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long (argc, argv, "cp:hfvl:t",
                   long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c) {
            case 'c':
                clear_flag = 1;
                break;
            case 'p':
                print_flag = 1;
                strncpy(msg, optarg, 15);
                break;
            case 'f':
                temp_flag = 1;
                break;
            case 'v':
                vccint_flag = 1;
                break;
            case 't':
                time_flag = 1;
                break;
            case 'l':
                line = strtol(optarg, NULL, 0);
                if (line != 0 && line != 1) {
                    line = 0;
                }
                line *= 2;
                break;
            case 'h':
                usage(argv);
                break;
              /* getopt_long already printed an error message. */
            default:
              return -1;
        }
    }

    /* Print any remaining command line arguments (not options). */
    if (optind < argc) {
        printf("non-option ARGV-elements: ");
        while (optind < argc)
           printf("%s ", argv[optind++]);
        putchar('\n');
    }
    return 0;
}

double xadc_get_channel_attr_value(struct iio_channel * ch, const char* attr){
  double ret = 0, val;
  ret = iio_channel_attr_read_double(ch, attr, &val);
  if (ret >= 0){
    //LOG_DEBUG("%s value: %f\n", attr, val);
    return val;
  }else if (ret == -ENOSYS){
    printf("\t\t\t\tattr %u: %s\n", 0, attr);
  }else{
    printf("Unable to read attribute %s\n", attr);
  }
  return ret;
}

int get_xadc_temp(double *temp) {
    static struct iio_context *ctx;
    ctx = iio_create_local_context();
    if (!ctx) {
        printf("Unable to create context\n");
        return -EXIT_FAILURE;
    }

    char str[64];
    double raw, scale, offset;
    struct iio_channel *ch;
    struct iio_device  *dev = iio_context_find_device(ctx, "xadc");
      if (!dev) {
          printf("Unable to find device %s\n", "xadc");
          return NULL;
      }

    snprintf(str, sizeof(str), "%s%d", "temp", 0);

    ch = iio_device_find_channel(dev, str, 0);
    raw = xadc_get_channel_attr_value(ch, "raw");
    scale = xadc_get_channel_attr_value(ch, "scale");
    offset = xadc_get_channel_attr_value(ch, "offset");

    *temp = (raw + offset) * scale / 1000.0f;
    
    iio_context_destroy(ctx);
}

int get_xadc_channel(int channel, double *val) {
    static struct iio_context *ctx;
    ctx = iio_create_local_context();
    if (!ctx) {
        printf("Unable to create context\n");
        return -EXIT_FAILURE;
    }
    char str[64];
    double raw, scale, offset;
    struct iio_channel *ch;
    struct iio_device  *dev = iio_context_find_device(ctx, "xadc");
    if (!dev) {
        printf("Unable to find device %s\n", "xadc");
        return -1;
    }

    snprintf(str, sizeof(str), "%s%d", "voltage", channel);

    ch = iio_device_find_channel(dev, str, 0);
    raw = xadc_get_channel_attr_value(ch, "raw");
    scale = xadc_get_channel_attr_value(ch, "scale");
    *val = raw * scale / 1000.0f;
    iio_context_destroy(ctx);
}

int main(int argc, char **argv) {
    int ret;
    double temp;
    ret = handle_args(argc, argv);
    if (ret) {
        printf("handle_args failed\n");
        exit(1);
    }

    if (clear_flag) 
        clear();
    else if (print_flag)
        print_message(msg, line);
    else if (temp_flag) {
        get_xadc_temp(&temp);
        sprintf(msg, "%.3lf oC", temp);
        print_message("FPGA TEMP:", 0);
        print_message(msg, 2);

        printf("FPGA TEMP: %s\n", msg);
    }
    else if (vccint_flag) {
        get_xadc_channel(0, &temp);
        sprintf(msg, "%.3lf V", temp);

        print_message("VCCINT:", 0);
        print_message(msg, 2);

        printf("VCCINT: %s\n", msg);
    }
    else if (time_flag) {
        struct timeval  tv;
        struct tm      *tm;
        time_t ltime;
        gettimeofday(&tv, NULL);
        ltime = time(NULL);
        tm = localtime(&ltime);
        sprintf(msg, "%04d-%02d-%02d",tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday);
        print_message(msg, 0);
        sprintf(msg, "%02d:%02d:%02d",tm->tm_hour,tm->tm_min,tm->tm_sec);
        print_message(msg, 2);
    }
    return 0;
}
