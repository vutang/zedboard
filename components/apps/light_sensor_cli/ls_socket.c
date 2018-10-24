#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */

#include <string.h>
#include <error.h>
#include <errno.h>
#include "ls_socket.h"
#include "logger.h"

#define SERVER_UDP_PORT 6666
extern char server_ip[INET_ADDRSTRLEN];

str_udpskt_id_t gcli_skt;

en_skt_ret_t cli_udpskt_open(int16_t udpPort) {
	/*Init Socket ID*/
	gcli_skt.socket_fd = -1;
	gcli_skt.socket_addr.sin_family = AF_INET;
	gcli_skt.socket_addr.sin_port = 0;
	gcli_skt.socket_addr.sin_addr.s_addr = 0;
	gcli_skt.socket_isOpen = 0;
	memset(&(gcli_skt.socket_addr.sin_zero), 0, 8);

	/*Open an Socket*/
	gcli_skt.socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (gcli_skt.socket_fd < 0) {
		LOG_ERROR("Socket open fail errno = %s", (char *) strerror(errno));
		return -SOCKET_RET_ERROR;
	}

	gcli_skt.socket_addr.sin_port = htons(udpPort);
	gcli_skt.socket_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	/*Bind*/
	if (bind(gcli_skt.socket_fd, (struct sockaddr *) &gcli_skt.socket_addr, \
		sizeof(struct sockaddr)) < 0) {
		LOG_ERROR("Socket bind fail errno = %s", strerror(errno));
		return -SOCKET_RET_ERROR;
	}
	return SOCKET_RET_OK;
}

en_skt_ret_t cli_udpskt_send(str_udpskt_id_t *pskt_id, void *psend_buf, \
		int send_buf_len, u_int32_t dest_ip, u_int16_t dest_port, \
		u_int32_t *p_sent_dat_len) {

	struct sockaddr_in receiver_addr;
	int32_t num_byte;

	receiver_addr.sin_family = AF_INET;
	receiver_addr.sin_port = htons(dest_port);
	receiver_addr.sin_addr.s_addr = dest_ip;
	memset(&(receiver_addr.sin_zero), 0, 8);

	if ((num_byte = sendto(pskt_id->socket_fd, psend_buf, send_buf_len, 0,\
		(struct sockaddr *) &receiver_addr, sizeof(struct sockaddr))) < 0) {
		LOG_ERROR("sendto socket fail errno = %s", (char *) strerror(errno));
		return -SOCKET_RET_ERROR;
	}
	return SOCKET_RET_OK;
}

void send_udp_test_pattern(void) {
	char buf[128];
	int i = 0, ret, sent_len = -1;
	for (i = 0; i < 128; i++)
		buf[i] = i;
	LOG_DEBUG("Send test msg to %s", server_ip);
	ret = cli_udpskt_send(&gcli_skt, buf, i, inet_addr(server_ip), SERVER_UDP_PORT, \
		&sent_len);
	if (ret < 0) {
		LOG_ERROR("cli_udpskt_send fail");
		return;
	}
	return;
}

void send_udp_hw_report(str_sensor_data_t *hw_sensor_dat) {
	int ret, sent_len, len = sizeof(str_sensor_data_t), i, cnt = 0;
	char *ptr = (char *) hw_sensor_dat;
	char hex[100];

	LOG_DEBUG("Send report msg to %s", server_ip);
	LOG_DEBUG("\t si1145_ps1: %d", hw_sensor_dat->si1145_ps1);
	LOG_DEBUG("\t si1145_als: %d", hw_sensor_dat->si1145_als);
	LOG_DEBUG("\t si1152_uv: %d", hw_sensor_dat->si1152_uv);
	LOG_DEBUG("\t tsl2591_als0: %d", hw_sensor_dat->tsl2591_als0);
	LOG_DEBUG("\t tsl2591_als1: %d", hw_sensor_dat->tsl2591_als1);

	/*Dump UDP Payload*/
	for (i = 0; i < len; i++) {
		cnt += sprintf(hex + cnt, "%02x ", *(ptr + i));
	}
	LOG_DEBUG("UDP payload (hex): %s", hex);

	ret = cli_udpskt_send(&gcli_skt, (void *) hw_sensor_dat, sizeof(str_sensor_data_t), \
		inet_addr(server_ip), SERVER_UDP_PORT, &sent_len);
	if (ret < 0) {
		LOG_ERROR("cli_udpskt_send fail");
		return;
	}
	return;
}