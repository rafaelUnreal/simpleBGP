#ifndef PACKET_H
#define PACKET_H

#include <stdlib.h>
#include <stdint.h>


#define BUFLEN 4096 // max buffer length
#define PORT 179    // hardcoded port
#define BACKLOG 10



// Generic Buffer Packet Struct
struct packet 
{
	unsigned char * data;
	u_int16_t index;
	u_int16_t data_size;
	u_int16_t size;
	int status;

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

struct __attribute__((__packed__)) bgp_path_attribute
{
    u_int8_t    flags;
	u_int8_t	type;
	u_int8_t	len;
};

struct __attribute__((__packed__)) origin
{
    u_int8_t    origin;
};

struct __attribute__((__packed__)) as_segment
{
    u_int8_t    type;
	u_int8_t	len;
};

struct __attribute__((__packed__)) as
{
    u_int16_t    as;
};




#endif

