#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include "packet.h"
#include "serialize.h"

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0') 

enum enconding_type {
	
	U_INT_8 = 8,
	U_INT_16 = 16,
	U_INT_32 = 32,
	U_INT_64 = 64,
	MARKER = 17,
	CHUNK_DATA //Variable size

};

#define MARKER 17	
struct enconding_rule {
	
	enum enconding_type type;
	u_int32_t offset;
	
};


static struct enconding_rule encoding_bgp_hdr[] = {
	
	{ MARKER,		offsetof(struct bgp_hdr, bgp_marker)},
	{ U_INT_16,		offsetof(struct bgp_hdr, bgp_len)	},
	{ U_INT_8,		offsetof(struct bgp_hdr, bgp_type)	}
	
};



static struct enconding_rule encoding_bgp_open[] = {
	
	{ U_INT_8,		offsetof(struct bgp_open, open_version)},
	{ U_INT_16,		offsetof(struct bgp_open, open_asn)	},
	{ U_INT_16,		offsetof(struct bgp_open, open_holdTimer)},
	{ U_INT_32,		offsetof(struct bgp_open, open_id)}
	
};



void encodeFields(struct packet *p, int field, u_int32_t offset, void *s ){
	
	unsigned char *byteArray = (unsigned char *) s;
	int i;
	switch(field){
		
	
	case U_INT_8:
		p->data[p->index] = byteArray[offset];
		p->index = p->index + 1;
		p->data_size = p->data_size + 1;
	
	break;
	
	case U_INT_16:
		//printf("pdata0 ENCODE: %02X\n" , byteArray[offset]	);
		//printf("pdata1 ENCODE: %02X\n" ,byteArray[offset+1]);
	
		packi16( &(p->data[p->index]), *(u_int16_t *)&byteArray[offset]);
		//printf("pdata0 ENCODE 2: %02X\n" , p->data[p->index]	);
		//printf("pdata1 ENCODE 2: %02X\n" ,p->data [p->index+1]);
		p->index = p->index + 2;

		p->data_size = p->data_size + 2;
	break;
	case U_INT_32:
	
		//printf(" " BYTE_TO_BINARY_PATTERN,  BYTE_TO_BINARY(byteArray[offset]));
		//printf(" " BYTE_TO_BINARY_PATTERN,   BYTE_TO_BINARY(byteArray[offset+1]));
		//printf(" " BYTE_TO_BINARY_PATTERN,  BYTE_TO_BINARY(byteArray[offset+2]));
		//printf(" " BYTE_TO_BINARY_PATTERN,  BYTE_TO_BINARY(byteArray[offset+3]));
		//printf("\n");
	
	//printf( " ALL BYTES %d \n", *(u_int32_t *)&byteArray[offset]);
		packi32( &(p->data[p->index]), *(u_int32_t *)&byteArray[offset]);
		p->index = p->index + 4;
		p->data_size = p->data_size + 4;
	break;
	
	case U_INT_64:
	
	//printf(" " BYTE_TO_BINARY_PATTERN,  BYTE_TO_BINARY(byteArray[offset]));
	//printf(" " BYTE_TO_BINARY_PATTERN,  BYTE_TO_BINARY(byteArray[offset+1]));
	//printf(" " BYTE_TO_BINARY_PATTERN,  BYTE_TO_BINARY(byteArray[offset+2]));
	//printf(" " BYTE_TO_BINARY_PATTERN,  BYTE_TO_BINARY(byteArray[offset+3]));
	//printf("\n");
	
	//printf( " ALL BYTES %d \n", *(u_int32_t *)&byteArray[offset]);
		packi64( &(p->data[p->index]), *(u_int64_t *)&byteArray[offset]);
		p->index = p->index + 8;
		p->data_size = p->data_size + 8;
	
	break;
	
	case MARKER:
		for(i =0; i<16; i++){
			p->data[p->index+i] =  byteArray[offset+i];
		}

		p->index = p->index + 16;
		p->data_size = p->data_size + 16;
	break;
		
	}	
}

void encodeBGPHeader(struct packet *p, struct bgp_hdr *bgp)
{
	u_int16_t size;
	u_int16_t i;
	size = sizeof(encoding_bgp_hdr) / sizeof(encoding_bgp_hdr[0]); 
	
	for(i=0; i < size; i++){
		//printf("offset of %d \n" ,  encodings[i].offset);
		//printf("isa length: %d \n", isa->isa_length);
		encodeFields(p,encoding_bgp_hdr[i].type, encoding_bgp_hdr[i].offset, bgp);
		
	}	
}

void encodeBGPOpen(struct packet *p, struct bgp_open *bgp)
{
	u_int16_t size;
	u_int16_t i;
	size = sizeof(encoding_bgp_open) / sizeof(encoding_bgp_open[0]); 
	
	for(i=0; i < size; i++){
		//printf("offset of %d \n" ,  encodings[i].offset);
		//printf("isa length: %d \n", isa->isa_length);
		encodeFields(p,encoding_bgp_open[i].type, encoding_bgp_open[i].offset, bgp);
		
	}	
}



void encodeChunk(struct packet *p, unsigned char *chunkData, u_int16_t size)
{
		memcpy(&(p->data[p->index]), chunkData , size);
		p->index = p->index + size;		
		p->data_size = p->data_size + size;
}


void decodeFields(struct packet *p, int field, u_int32_t offset, void *s ){
	
	unsigned char *byteArray = (unsigned char *) s;
	int i;
	switch(field){
		
	case U_INT_8:
		byteArray[offset] = p->data[p->index];
		p->index = p->index + 1;
		//p->size = p->size + 1;
	
	break;
	
	case U_INT_16:
		*(u_int16_t *)(s+offset) = unpacku16(&(p->data[p->index]));
		//memcpy(byteArray[offset], unpacku16(&(p->data[p->index])),2);
		//printf("CASE %02X\n", unpacku16(&(p->data[p->index])));
		p->index = p->index + 2;
		//printf(" " BYTE_TO_BINARY_PATTERN,  BYTE_TO_BINARY(byteArray[offset]));
		//printf(" " BYTE_TO_BINARY_PATTERN,   BYTE_TO_BINARY(byteArray[offset+1]));
		//p->size = p->size + 2;
	break;
	case U_INT_32:
	

		*(u_int32_t *)(s+offset) = unpacku32(&(p->data[p->index]));
		p->index = p->index + 4;
		//p->size = p->size + 4;
	break;
	
	case U_INT_64:
	
		*(u_int64_t *)(s+offset)  = unpacku64(&(p->data[p->index]));
		p->index = p->index + 8;
		//p->size = p->size + 8;
	
	break;
	
	case MARKER:
		//unsigned int *x;
		//x =  (unsigned int*) 
		for(i =0; i<16; i++){
			byteArray[offset+i] =  p->data[p->index+i];
		}
		p->index = p->index + 16;
		break;
		
	}	
}



void decodeBGPHeader(struct packet *p, struct bgp_hdr *bgp)
{
	u_int16_t size;
	u_int16_t i;
	size = sizeof(encoding_bgp_hdr) / sizeof(encoding_bgp_hdr[0]); 
	
	for(i=0; i < size; i++){
		//printf("offset of %d \n" ,  encoding_bgp_hdr[i].offset);
		//printf("isa length: %d \n", bgp->bgp_len);
		decodeFields(p,encoding_bgp_hdr[i].type, encoding_bgp_hdr[i].offset, bgp);
		
	}	
}

void decodeBGPOpen(struct packet *p, struct bgp_open *bgp)
{
	u_int16_t size;
	u_int16_t i;
	size = sizeof(encoding_bgp_open) / sizeof(encoding_bgp_open[0]); 
	
	for(i=0; i < size; i++){
		//printf("offset of %d \n" ,  encoding_bgp_hdr[i].offset);
		//printf("isa length: %d \n", bgp->bgp_len);
		decodeFields(p,encoding_bgp_open[i].type, encoding_bgp_open[i].offset, bgp);
		
	}	
}



