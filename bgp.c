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

struct packet* BGP_active(struct packet *p,struct bgp_neighbor *neighbor_p){
	
	printf("BGP_active()\n");
	struct bgp_hdr hdr = {0};

	decodeBGPHeader(p,&hdr);

	printf("BGP header:\n");
	printf("BGP len=%02X\n",hdr.bgp_len);
	printf("BGP type=%02X\n",hdr.bgp_type);
	
	if(hdr.bgp_type == 1){
		
		struct bgp_open open_message = {0};
		
		decodeBGPOpen(p,&open_message);
		
		printf("BGP Open\n");
		printf("BGP Open ASN =%02X\n",open_message.open_asn);
		printf("BGP Open Hold Time =%02X\n",open_message.open_holdTimer);
		//decode
		
	}
	return NULL;
	
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

struct packet* processPacket(struct packet *p, unsigned int size){
	
	struct bgp_hdr bgp = {0};
	printf("decode\n");
	decodeBGPHeader(p,&bgp);
	printf("decode end \n");
	printf("BGP header:\n");
	printf("BGP len=%02X\n",bgp.bgp_len);
	printf("BGP type=%02X\n",bgp.bgp_type);
	
	return NULL;
	
}


int main(void)
{
	printf("Simple BGP by Rafael P.\n");
	//if(initConfig()){ perror("Config File Error\n"); return 0; }
	connectionPool();
    return 0;
}
