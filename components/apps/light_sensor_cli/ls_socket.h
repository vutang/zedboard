
typedef enum {
	SOCKET_RET_OK,
	SOCKET_RET_ERROR
} en_skt_ret_t;

typedef struct _str_udpskt_id_t {
	int socket_fd;
	struct sockaddr_in socket_addr;
	int8_t socket_isOpen;
} str_udpskt_id_t;

/*Report structure*/
typedef struct str_sensor_data {
	int si1145_ps1;
	int si1145_als;
	int si1152_uv;
	short tsl2591_als0;
	short tsl2591_als1;
} str_sensor_data_t;

en_skt_ret_t cli_udpskt_open(int16_t udpPort);
en_skt_ret_t cli_udpskt_send(str_udpskt_id_t *pskt_id, void *psend_buf, \
		int send_buf_len, u_int32_t dest_ip, u_int16_t dest_port, \
		u_int32_t *p_sent_dat_len);