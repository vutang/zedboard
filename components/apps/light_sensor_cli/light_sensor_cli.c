/*
* @Author: vutt6
* @Date:   2018-10-10 17:05:30
* @Last Modified by:   vutang
* @Last Modified time: 2018-10-12 17:22:48
*/
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */

#include <string.h>
#include <error.h>
#include <errno.h>

#include <getopt.h>

#include "ls_socket.h"
#include "logger.h"
#include "si1145.h"

#define LOGDIR "./lightSensor.log"
#define CLIENT_UDP_PORT 7777

/*For timer*/
typedef enum {
	TIMER_ID_SEND_TEST_ETH,
	TIMER_ID_SEND_REPORT,
	TIMER_ID_MAX
} en_timer_id_t;

typedef struct {
	unsigned long start;
	unsigned long end;
} ls_timer_t;

/*Report structure*/
typedef struct str_sensor_data {
	int si1145_ps1;
	int si1145_als;
	int si1152_uv;
} str_sensor_data_t;

pthread_t gtid_timer, gtid_hwmon; /*For timer thread*/
int8_t gudpskt_test_flag = 0, g_serip_change = 0;
char server_ip[INET_ADDRSTRLEN] = "192.168.120.100";

ls_timer_t timer[TIMER_ID_MAX];
str_sensor_data_t ghw_sensor_dat;

int timer_start(int timeInSecond,unsigned int timerId);

void testLog() {
	LOG_WARN("LOG_WARN");
	LOG_DEBUG("LOG_DEBUG");
	LOG_INFO("LOG_INFO");
	LOG_ERROR("LOG_ERROR");
	LOG_FATAL("LOG_FATAL");
}

int check_timer_expiration() {
	int i;
	struct timespec tp;

	clock_gettime(CLOCK_MONOTONIC,&tp);
}

void timer_expired(int timerid) {
	switch (timerid) {
		case TIMER_ID_SEND_TEST_ETH:
			send_udp_test_pattern();
			timer_start(1, TIMER_ID_SEND_TEST_ETH);
			break;
		case TIMER_ID_SEND_REPORT:
			send_udp_hw_report((char *) &ghw_sensor_dat, sizeof(str_sensor_data_t));
			timer_start(1, TIMER_ID_SEND_REPORT);
		default:
			break;
	}
}
int check_expiration() {
	int i;
	struct timespec tp;

	// read current time.
	clock_gettime(CLOCK_MONOTONIC,&tp);
	for (i=0; i<TIMER_ID_MAX; i++) {
		if (timer[i].end!=0) {
			if (((timer[i].start <= timer[i].end) && (timer[i].end <= tp.tv_sec)) ||
				((timer[i].start > timer[i].end) && (timer[i].end >= tp.tv_sec)))  {
				timer[i].end = 0;
				timer[i].start =0;
				timer_expired(i);
			}
		}
	}; // end for
	return 0;
} 

void *timer_check(void *p) {
	while (1) {
		check_expiration();
		sleep(1);
	}
}

void *hw_mon(void *p) {
	int tmp = -1, ret;
	while (1) {
		/*Force prox and als: as example code*/
		ret = si1145_dev_write_byte(0x18, 0x05);
		if (ret < 0) {
			LOG_WARN("Force prox and als meas fail");
		}

		/*Read LSB of PS1*/
		ret = si1145_get_ps1(&tmp);
		if (!ret)
			ghw_sensor_dat.si1145_ps1 = tmp;
		else
			ghw_sensor_dat.si1145_ps1 = -1;

		/*Read ALS*/

		ret = si1145_get_als(&tmp);
		if (!ret)
			ghw_sensor_dat.si1145_als = tmp;
		else
			ghw_sensor_dat.si1145_als = -1;
		/*Read UV*/
		ret = si1145_get_uvdata(&tmp);
		if (!ret) 
			ghw_sensor_dat.si1152_uv = tmp;
		else 
			ghw_sensor_dat.si1152_uv = -1;

		sleep(1);
	}
}

void open_timer_thd(void) {
	pthread_attr_t pthread_attr;
	pthread_attr_init(&pthread_attr); 
	pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_DETACHED);

	if (pthread_create(&gtid_timer, &pthread_attr, &timer_check, NULL) != 0) {
		LOG_ERROR("Create timer_check thread fail");
	}
	pthread_attr_destroy(&pthread_attr);
}

void open_hw_mon(void) {
	pthread_attr_t pthread_attr;
	pthread_attr_init(&pthread_attr); 
	pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_DETACHED);

	if (pthread_create(&gtid_hwmon, &pthread_attr, &hw_mon, NULL) != 0) {
		LOG_ERROR("Create tid_hwmon thread fail");
	}
	pthread_attr_destroy(&pthread_attr);
}

void init_timer() {
	int i;
	for (i = 0; i < TIMER_ID_MAX; i++)
		timer[i].end = 0;
}
 
int timer_start(int timeInSecond,unsigned int timerId) {
	struct timespec tp;
	if ((timerId > TIMER_ID_MAX)||(timeInSecond<=0)) 
		return 1;
	// if timeInSecond = 0, just next cycle it will expired.
	clock_gettime(CLOCK_MONOTONIC,&tp);
	timer[timerId].end = tp.tv_sec  + timeInSecond;
	timer[timerId].start = tp.tv_sec;
	return 0;
}

void pri_usage(int argc, char **argv) {
	printf("usage: %s --opt [arg]\n", argv[0]);
	printf("\t--help: print usage\n");
	printf("\t--serip <ipserver>\n");
	printf("\t--test/-t\n");
}

int is_valid_ipaddr(char *ip_addr) {
	struct in_addr addr;
	if (inet_aton(ip_addr, &addr) == 0) {
		return 0;
	}
	strcpy(server_ip, inet_ntoa(addr));
	return 1;
}

void get_args(int argc, char **argv) {
	int c;

	while (1) {
		static struct option long_options[] = {
			{"help", no_argument, 0, 'h'},
			{"test", no_argument, 0, 't'},
			{"serip", required_argument, 0, 's'},
			{0, 0, 0, 0}
		};
	      /* getopt_long stores the option index here. */
		int option_index = 0;
		c = getopt_long (argc, argv, "hts:", long_options, &option_index);
		/* Detect the end of the options. */
		if (c == -1)
			break;

		switch (c) {
			case 'h':
				pri_usage(argc, argv);
				exit(0);
				break;
			case 't':
				gudpskt_test_flag = 1;
				break;
			case 's':
				printf ("option -s with vl `%s'\n", optarg);
				if (is_valid_ipaddr(optarg) == 1) {
					LOG_INFO("Get server ip: %s", server_ip);
					g_serip_change = 1;
				}
				else {
					LOG_ERROR("Invalid ipaddr: \"%s\"", optarg);
					exit(0);
				}
				break;
			case '?':
				/* getopt_long already printed an error message. */
				pri_usage(argc, argv);
				break;
			default:
				pri_usage(argc, argv);
				abort();
		}
	}
}

int main(int argc, char **argv) {
	int ret;
	config_log(LOGDIR, 0x1f, 3);
	get_args(argc, argv);
	if (!g_serip_change)
		LOG_INFO("Run with default serverip: %s", server_ip);
	/*Test Log*/
	// testLog();

	/*Open Socket*/
	LOG_DEBUG("Open UDP socket");
	ret = cli_udpskt_open(CLIENT_UDP_PORT);
	if (ret < 0) {
		LOG_ERROR("create cli_udpskt_open fail");
		return 1;
	}

	/*Setup hw*/
	LOG_DEBUG("Open si1145 dev");
	ret = si1145_dev_open();
	if (ret < 0) {
		LOG_ERROR("Open si1145 dev fail");
	} 
	LOG_INFO("Open timer thread");
	open_timer_thd();

	LOG_INFO("Init timer");
	init_timer();

	if (!strcmp(argv[1], "test")) {
		LOG_INFO("Get command Test");
		gudpskt_test_flag = 1;
	}

	LOG_INFO("Setup si1145");


	/*Run Test Ethernet to test udp send function*/
	if (gudpskt_test_flag == 1)
		timer_start(1, TIMER_ID_SEND_TEST_ETH);
	else {
		si1145_setup();
		timer_start(1, TIMER_ID_SEND_REPORT);
	}
	while(1);
	return 0;
}