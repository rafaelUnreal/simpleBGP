#include <stdio.h>      // printf
#include <string.h>     // memset
#include <stdlib.h>     // exit(0);
#include <arpa/inet.h>  // inet_ntoa, ntohs
#include <sys/socket.h> // bind, recvfrom
#include <sys/queue.h>  // for Linkedlist
#include <unistd.h>     // close();
#include <sys/queue.h>  // for Linkedlist
#include <time.h>
#include <stdint.h>
#include "trie.h"
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
	int			s_other;
	time_t 		event_keepalive;
	time_t 		event_hold_time;
	STAILQ_ENTRY(bgp_neighbors) pointers;

};

void BGP_new_connection(int s_other, uint32_t ipv4_address){
	
	struct bgp_neighbor *neighbor;
	neighbor = malloc(sizeof(struct bgp_neighbor));
	
	if(neighbor!=NULL){
		neighbor->ip_address = ipv4_address;
		neighbor->s_other = s_other;
		neighbor->state = ACTIVE;
		printf("ACTIVE\n");
		STAILQ_INSERT_TAIL(&neighbor_head, neighbor, pointers);
	}
	
	
}
/* Getters and Setters */

int BGP_get_optional_param_len(){
	
	return 0;
}


int BGP_set_open_message(struct bgp_hdr *hdr, struct bgp_open *open){
	
	unsigned int _optional_param_len = BGP_get_optional_param_len();
	
	memcpy(hdr->bgp_marker,"\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF",16);
	hdr->bgp_len = sizeof(struct bgp_hdr) + sizeof(struct bgp_open) + _optional_param_len;
	hdr->bgp_type = 0x1;
	
	open->open_version = config_get_version();
	open->open_asn = config_get_asn();
	open->open_holdTimer = config_get_hold_time();
	open->open_id = config_get_id();
	open->optional_param_len = _optional_param_len;
	
	
}

int BGP_set_keepalive(struct bgp_hdr *keepalive){
	
	memcpy(keepalive->bgp_marker,"\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF",16);
	keepalive->bgp_len = sizeof(struct bgp_hdr);
	keepalive->bgp_type = 0x4;
	
}

int event_hold_time(struct bgp_neighbor *neighbor_p, int max_elapsed_time){
	
	time_t event_hold_time; 
	time_t elapsed_hold_time;
	
	time(&event_hold_time);
	elapsed_hold_time = event_hold_time - neighbor_p->event_hold_time;

	if(elapsed_hold_time >= max_elapsed_time){
		
		printf("event_hold_time(): No reply from remote peer\n");
		return 1;
		
	}
	return 0;	
}

int BGP_open_message_check(struct bgp_open *msg, uint32_t ipv4_address){
	
	u_int8_t	neighbor_id;
	u_int8_t	open_version;
	u_int16_t	open_asn;
	u_int16_t	open_holdTimer;
	u_int32_t	open_id;
	
	printf("BGP_open_message_check()\n");
	
	if((neighbor_id = config_get_neighbor(ipv4_address)) == -1){
		printf("BGP_open_message_check(): no valid neighbor config\n");
		return 0;	
	}
	
	open_asn = config_get_asn();
	open_version = config_get_version();
	//open_id = config_get_id();
	
	if(open_version != msg->open_version){
		printf("BGP_open_message_check(): invalid open message version; local_version=%d remote_version=%d\n",open_version,msg->open_version);
		return 0;
	}
	/*
	if(open_id == msg->open_id){
		printf("BGP_open_message_check(): same router id\n");
		return -1;
	}
	*/
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
		printf("BGP Open ASN=%02X\n",open_message.open_asn);
		printf("BGP Open Hold Time=%02X\n",open_message.open_holdTimer);
		printf("BGP Open Version=%02X\n",open_message.open_version);
		
		if(BGP_open_message_check(&open_message,neighbor_p->ip_address)){
			
			struct bgp_hdr hdr_reply = {0};
			struct bgp_open open_reply = {0};
			struct bgp_hdr keep_alive = {0};
			
			BGP_set_open_message(&hdr_reply,&open_reply);
			BGP_set_keepalive(&keep_alive);
			
			
			struct packet *BGP_response = malloc(sizeof(struct packet));
			unsigned int BGP_response_size = sizeof(hdr_reply) + sizeof(open_reply)+ open_reply.optional_param_len + sizeof(keep_alive);
			BGP_response->data = malloc(BGP_response_size);
			BGP_response->index = 0;
			BGP_response->status = 0;
			BGP_response->size = BGP_response_size; 
			
			encode_bgp_header(BGP_response,&hdr_reply);
			encode_bgp_open(BGP_response,&open_reply);
			encode_bgp_header(BGP_response,&keep_alive);
			
			printf("starting bgp keepalive_event()\n");
			time(&(neighbor_p->event_keepalive));
			printf("starting bgp hold_time_event()\n");
			time(&(neighbor_p->event_hold_time));
			
			printf("ACTIVE-> OPEN_CONFIRM\n");
			neighbor_p->state = OPEN_CONFIRM;
			return BGP_response;
			
		}
		
		
	}
	
	
	return NULL;
	
}
struct packet* BGP_open_confirm(struct packet *p,struct bgp_neighbor *neighbor_p){
	
	printf("BGP_open_confirm()\n");
	struct bgp_hdr hdr = {0};
	struct packet *BGP_response;
	unsigned int BGP_response_size;
	
	if(p!=NULL){
		decode_bgp_header(p,&hdr);
	
		if(hdr.bgp_type == 0x4){
			printf("BGP_open_confirm(); BGP keepalive\n");
			struct bgp_hdr keep_alive = {0};
			BGP_set_keepalive(&keep_alive);
		
			BGP_response = malloc(sizeof(struct packet));
			BGP_response_size = sizeof(keep_alive);
			BGP_response->data = malloc(BGP_response_size);
			BGP_response->index = 0;
			BGP_response->status = 0;
			BGP_response->size = BGP_response_size; 
			
			/* encode reply */
			encode_bgp_header(BGP_response,&keep_alive);
			printf("OPEN_CONFIRM -> ESTABLISHED\n");
			neighbor_p->state = ESTABLISHED;
		
			return BGP_response;
		}
	}

	return NULL;
}

int event_keepalive(struct bgp_neighbor *neighbor_p, int max_elapsed_time){
	
	time_t event_keepalive; 
	time_t elapsed_keepalive;
	
	time(&event_keepalive);
	elapsed_keepalive = event_keepalive - neighbor_p->event_keepalive;

	if(elapsed_keepalive >= max_elapsed_time){
		
		printf("event_keepalive() triggered\n");
		return 1;
		
	}
	
	return 0;
	
}

struct packet* send_keepalive(){
	
	struct packet *BGP_response;
	unsigned int BGP_response_size;
	struct bgp_hdr keep_alive = {0};

	BGP_set_keepalive(&keep_alive);
		
	BGP_response = malloc(sizeof(struct packet));
	BGP_response_size = sizeof(keep_alive);
	BGP_response->data = malloc(BGP_response_size);
	BGP_response->index = 0;
	BGP_response->status = 0;
	BGP_response->size = BGP_response_size; 
			
	encode_bgp_header(BGP_response,&keep_alive);
	printf("send keepalive()\n");
	
	return BGP_response;
}


struct packet* BGP_established(struct packet *p,struct bgp_neighbor *neighbor_p){
	
	struct in_addr ip_addr;
	ip_addr.s_addr = neighbor_p->ip_address;
	printf("BGP_established(): neighbor: %s\n",inet_ntoa(ip_addr));
	struct packet *BGP_response;
	
	struct bgp_hdr hdr = {0};
	unsigned int BGP_response_size;

	if(p!=NULL){		
		decode_bgp_header(p,&hdr);
		
		if(hdr.bgp_type == 0x4){
			printf("Received BGP keepalive: neighbor: %s\n",inet_ntoa(ip_addr));
			printf("event_hold_time() restarted \n");
			time(&(neighbor_p->event_hold_time));
			
		}
		
		
		return NULL;
	}
	
	if(event_hold_time(neighbor_p,180)){
		BGP_response = malloc(sizeof(struct packet));
		BGP_response->status = -1;
		BGP_response->size = 0;
		BGP_response->index = 0;
		STAILQ_REMOVE_HEAD(&neighbor_head,pointers);
		return BGP_response;
	}
	
	
	if(event_keepalive(neighbor_p,30)){
		printf("send_keepalive event reseted\n");
		time(&(neighbor_p->event_keepalive));
		return send_keepalive();
	}

	return NULL;
	
}

struct packet* BGP_event_send(int fd){
	
	//printf("BGP_event_send()\n");
	
	struct bgp_neighbor *neighbor_p;
	struct in_addr ip_addr;
	STAILQ_FOREACH(neighbor_p, &neighbor_head, pointers) {
		
		if(neighbor_p->s_other == fd){
			ip_addr.s_addr = neighbor_p->ip_address;
			printf("BGP_event_send(): neighbor: %s\n",inet_ntoa(ip_addr));
			
		switch(neighbor_p->state){
				
		case IDLE:
				
		break;
		
		case ESTABLISHED:
			return BGP_established(NULL,neighbor_p);
				
		break;

			}
	
		}
		
		
	}
	
	return NULL;
}


struct packet* BGP_event_receive(struct packet *p,uint32_t ipv4_address){
	
	struct in_addr ip_addr;
	ip_addr.s_addr = ipv4_address;
	
	printf("BGP_event_receive(): neighbor: %s\n",inet_ntoa(ip_addr));;
	struct bgp_neighbor *neighbor_p;
	
	STAILQ_FOREACH(neighbor_p, &neighbor_head, pointers) {
		
		if(neighbor_p->ip_address == ipv4_address){

			
			switch(neighbor_p->state){
				
				case IDLE:
				
			
				break;

				
				case ACTIVE:
					return BGP_active(p,neighbor_p);
				break;
				
				case OPEN_SENT:

				break;
				
				case OPEN_CONFIRM:
					return BGP_open_confirm(p,neighbor_p);
				
				break;
				
				case ESTABLISHED:
					return BGP_established(p,neighbor_p);
				
				break;

			}
	
		}

	}	
	return NULL;
}



int main(void)
{
	printf("Simple BGP. Author Rafael P.\n");
	if(init_config()){ perror("init_config() error\n"); return 0; }
	init_trie();
	connectionPool();
    return 0;
}
