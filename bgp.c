#include <stdio.h>      // printf
#include <string.h>     // memset
#include <stdlib.h>     // exit(0);
#include <arpa/inet.h>  // inet_ntoa, ntohs
#include <sys/socket.h> // bind, recvfrom
#include <sys/queue.h>  // for Linkedlist
#include <unistd.h>     // close();
#include <sys/queue.h>  // for Linkedlist
#include <stdint.h>
#include "config.h"
#include "packet.h"

enum bgp_fsm 
{
	IDLE=0,
	CONNECT,
	ACTIVE,
	OPEN_SENT,
	OPEN_CONFIRM,
	ESTABLISHED
	
};

/* TODO: Implement neighbors in a hash map */
STAILQ_HEAD(bgp_neighbor_list, bgp_neighbor) neighbor_head = STAILQ_HEAD_INITIALIZER(neighbor_head);

struct bgp_neighbor
{
	u_int32_t	ASN;
	u_int32_t	ip_address;
	u_int32_t	bgp_id;
	u_int16_t	hold_time;
	u_int8_t	version;
	u_int8_t	state;
	STAILQ_ENTRY(bgp_neighbors) pointers;

};

void BGP_new_connection(uint32_t ipv4_address){
	
	struct bgp_neighbor *neighbor;
	neighbor = malloc(sizeof(struct bgp_neighbor));
	
	if(neighbor!=NULL){
		neighbor->ip_address = ipv4_address;
		neighbor->state = ACTIVE;
		STAILQ_INSERT_TAIL(&neighbor_head, neighbor, pointers);
	}
	
	
}

int BGP_open_message_check(struct bgp_open *msg, uint32_t ipv4_address){
	
	u_int8_t	neighbor_id;
	u_int8_t	open_version;
	u_int16_t	open_asn;
	u_int16_t	open_holdTimer;
	u_int32_t	open_id;
	
	printf("BGP_open_message_check()\n");
	
	if( neighbor_id = config_get_neighbor(ipv4_address) == 0){
		printf("BGP_open_message_check(): no valid neighbor config\n");
		return -1;	
	}
	
	open_asn = config_get_asn();
	open_version = config_get_version();
	open_id = config_get_id();
	
	if(open_version != msg->open_version){
		printf("BGP_open_message_check(): invalid open message version\n");
		return -1;
	}
	if(open_id == msg->open_id){
		printf("BGP_open_message_check(): same router id\n");
		return -1;
	}
	
	return 1;
}

struct packet* BGP_active(struct packet *p,struct bgp_neighbor *neighbor_p){
	
	printf("BGP_active()\n");
	struct bgp_hdr hdr = {0};
	decode_bgp_header(p,&hdr);
	
	if(hdr.bgp_type == 0x1){
		
		struct bgp_open open_message = {0};
		
		decode_bgp_open(p,&open_message);
		printf("BGP Open\n");
		printf("BGP Open ASN =%02X\n",open_message.open_asn);
		printf("BGP Open Hold Time =%02X\n",open_message.open_holdTimer);
		
		
		if(BGP_open_message_check(&hdr,neighbor_p->ip_address)){
			
			
		}
		
		
	}
	
	
	return NULL;
	
}
struct packet* BGP_open_confirm(struct packet *p,struct bgp_neighbor *neighbor_p){
	
	
	
}


struct packet* BGP_established(struct packet *p,struct bgp_neighbor *neighbor_p){
	
	
	
}
/*
struct packet *p BGP_event_send(struct bgp_neighbor *neighbor_p){
	
	
	
	
}
*/

struct packet* BGP_event_receive(struct packet *p,uint32_t ipv4_address){
	
	struct bgp_neighbor *neighbor_p;
	
	printf("BGP_event_receive()\n");
	STAILQ_FOREACH(neighbor_p, &neighbor_head, pointers) {
		
		if(neighbor_p->ip_address == ipv4_address){
			printf("neighbor found\n");
			
			
			switch(neighbor_p->state){
				
				case IDLE:
				
				
				break;
				case ESTABLISHED:
				
				case ACTIVE:
					return BGP_active(p,neighbor_p);
				break;
				
				break;


				case OPEN_CONFIRM:
				
				break;
				
				case OPEN_SENT:
				
				
				break;
				
				
				
				
			}
			
			
			
		}
		
		
	}
		
}

void BGP_event_send(){
	
	
	printf("BGP_event_send()\n");
	
}



int main(void)
{
	printf("Simple BGP by Rafael P.\n");
	if(init_config()){ perror("init_config() error\n"); return 0; }
	connectionPool();
    return 0;
}
