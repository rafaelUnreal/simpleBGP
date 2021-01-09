#include <stdlib.h>
#include <stdint.h>
#ifndef PACKET_H
#define PACKET_H

#define BUFLEN 65535 // max buffer length
#define PORT 179    // hardcoded port
#define BACKLOG 10


// Generic Buffer Packet Struct
struct packet 
{
	unsigned char * data;
	u_int16_t index;
	u_int16_t data_size;
	u_int16_t size;

};

struct __attribute__((__packed__)) bgp_hdr
{
    u_int8_t    bgp_marker[16];
	u_int16_t	bgp_len;
	u_int8_t	bgp_type;
};

struct __attribute__((__packed__)) bgp_open
{
	
	u_int8_t	open_version;
	u_int16_t	open_asn;
	u_int16_t	open_holdTimer;
	u_int32_t	open_id;
	u_int8_t	optional_param_len;
	
};

#endif

