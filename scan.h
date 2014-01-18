#ifndef SCAN_H
#define SCAN_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <stddef.h>

#define BUFFSIZE 1024
#define	FL_BROADCAST		0x0010
#define NB_DGRAM		137

struct nbname {
	char ascii_name [16] ;
	uint16_t rr_flags;
};
struct nbname_request {
        uint16_t transaction_id;
        uint16_t flags;
        uint16_t question_count;
        uint16_t answer_count;
        uint16_t name_service_count;
        uint16_t additional_record_count;
        char question_name[34]; 
        uint16_t question_type;
        uint16_t question_class;
};

struct nb_host_info {
	uint8_t number_of_names;
	struct nbname* names;
	uint8_t adapter_address [6];
	int is_broken;
};

// char* getnbservicename(uint8_t service, int unique, char* name);
struct nb_host_info* parse_response(char* buff, int buffsize);
void send_query(int sock, struct in_addr dest_addr, uint32_t rtt_base);
int print_hostinfo(struct in_addr addr, struct nb_host_info* hostinfo);
char* scan(char* my_addr);


#endif
