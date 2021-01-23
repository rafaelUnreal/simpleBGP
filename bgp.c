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
#include "bgp.h"
#include "trie.h"
#include "config.h"
#include "packet.h"

/*
struct bgp_attr{
	
	u_int8_t origin;
	u_int16_t as_path[255];
	u_int8_t as_path_size;
	u_int32_t next_hop;
	u_int32_t med;
	u_int32_t local_pref;
	
};

struct nlri {
	
	u_int32_t	prefix;
	u_int32_t	prefix_len;
	
};

struct bgp_advertisment {
	
	struct bgp_attr update_attr;
	struct nlri		update_nlri;
	LIST_ENTRY(bgp_advertisment) pointers;
};
*/


void init_local_advertisement(struct bgp_neighbor *neighbor){
	
	printf("init_local_advertisement()\n");
	unsigned int num_prefix;
	int prefix_id;
	struct bgp_advertisment *bgp_update_p;
	struct nlri *nlri_p;
	
	bgp_update_p = calloc(0,sizeof(struct bgp_advertisment)); 
	
	LIST_INIT(&(bgp_update_p->update_nlri));
	num_prefix = get_num_prefixes(neighbor->ip_address);
		
	for(prefix_id=0;prefix_id<num_prefix;prefix_id++){
		
		nlri_p = calloc(0,sizeof(struct nlri));
		nlri_p->prefix = htonl(get_prefix(neighbor->ip_address, prefix_id));
		nlri_p->prefix_len = get_prefix_len(neighbor->ip_address, prefix_id);
		
	
		LIST_INSERT_HEAD(&(bgp_update_p->update_nlri),nlri_p , pointers);
		//printf("prefix=%d\n",get_prefix(neighbor->ip_address, prefix_id));
		//printf("prefix_len=%d\n",get_prefix_len(neighbor->ip_address, prefix_id));
	}
	BGP_set_local_attr(&(bgp_update_p->update_attr));
	LIST_INSERT_HEAD(&(neighbor->bgp_adj_rib_out), bgp_update_p, pointers);
	
}

void load_neighbor_rib(struct bgp_neighbor *neighbor){
	
	printf("load_neighbor_rib()\n");

	LIST_INIT(&(neighbor->bgp_adj_rib_out));
	//LIST_INIT(&(neighbor->bgp_adj_rib_out.update_nlri));
	LIST_INIT(&(neighbor->bgp_adj_rib_in));
	
}

void load_neighbor(struct bgp_neighbor *neighbor){
	
	printf("load_neighbor()\n");

	load_neighbor_rib(neighbor);
	init_local_advertisement(neighbor);
	
}

void BGP_new_connection(int s_other, uint32_t ipv4_address){
	
	struct bgp_neighbor *neighbor;
	neighbor = malloc(sizeof(struct bgp_neighbor));

	if(neighbor!=NULL){
		neighbor->ip_address = ipv4_address;
		neighbor->s_other = s_other;
		neighbor->state = ACTIVE;
		printf("ACTIVE\n");
		
		load_neighbor(neighbor);
		
		STAILQ_INSERT_TAIL(&neighbor_head, neighbor, pointers);
	}
	
	
}
/* Getters and Setters */

int BGP_get_optional_param_len(){
	
	return 0;
}

void BGP_set_igp_attr(struct bgp_attr *igp_attr){
	
	
	
}

void BGP_set_local_attr(struct bgp_attr *igp_attr){
	
	igp_attr->origin = 0 ;
	igp_attr->as_path[0] = config_get_asn();
	igp_attr->as_path_size = 1;
	igp_attr->next_hop = config_get_id(); // next hop is the local router id for now.
	igp_attr->med = 0;
	igp_attr->local_pref = 100;
	
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
	
	if(hdr.bgp_type == OPEN_MESSAGE){
		
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
			
			
			struct packet *BGP_response = calloc(0,sizeof(struct packet));
			unsigned int BGP_response_size = sizeof(hdr_reply) + sizeof(open_reply)+ open_reply.optional_param_len + sizeof(keep_alive);
			BGP_response->data = malloc(BGP_response_size);
			BGP_response->index = 0;
			BGP_response->status = 0;
			
			printf("BGP Response Size = %d\n",BGP_response_size);
			
			encode_bgp_header(BGP_response,&hdr_reply);
			encode_bgp_open(BGP_response,&open_reply);
			encode_bgp_header(BGP_response,&keep_alive);
			
			printf("starting bgp keepalive_event()\n");
			time(&(neighbor_p->event_keepalive));
			printf("starting bgp hold_time_event()\n");
			time(&(neighbor_p->event_hold_time));
			printf("ACTIVE size=%d\n",BGP_response->size);
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
	
		if(hdr.bgp_type == KEEP_ALIVE){
			printf("BGP_open_confirm(); BGP keepalive\n");
			struct bgp_hdr keep_alive = {0};
			BGP_set_keepalive(&keep_alive);
		
			BGP_response = calloc(0,sizeof(struct packet));
			//BGP_response_size = sizeof(struct keepalive);
			BGP_response->data = calloc(0,sizeof(struct bgp_hdr));
			
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
		
	BGP_response = calloc(0,sizeof(struct packet));
	BGP_response_size = sizeof(keep_alive);
	BGP_response->data = malloc(BGP_response_size);
			
	encode_bgp_header(BGP_response,&keep_alive);
	printf("send keepalive()\n");
	
	return BGP_response;
}


struct packet* send_update(struct bgp_neighbor *neighbor_p){
	
	printf("send_update()\n");
	struct packet *BGP_message;
	struct bgp_advertisment *update;
	struct nlri *nlri_p;
	unsigned int BGP_message_size;
	struct bgp_hdr hdr = {0};
	struct bgp_path_attribute _bgp_path_attribute= {0};
	unsigned int as_id;
	unsigned int total_attr_len;
	u_int16_t withdrawn_route_len = 0;
	u_int8_t as_segment_type = AS_SEQUENCE;

	BGP_message = calloc(0,sizeof(struct packet));
	BGP_message->data = malloc(4096);

	
	BGP_message->index = 23;  //TEST

	update = LIST_FIRST(&(neighbor_p->bgp_adj_rib_out));

	_bgp_path_attribute.flags = TRANSITIVE_ATTRIBUTE;
	_bgp_path_attribute.type = ORIGIN;
	_bgp_path_attribute.len = 1;
	encode_bgp_attr(BGP_message,&_bgp_path_attribute);
	encode_bgp_origin(BGP_message,&(update->update_attr.origin));

	_bgp_path_attribute.flags = TRANSITIVE_ATTRIBUTE;
	_bgp_path_attribute.type = AS_PATH;
	_bgp_path_attribute.len = (update->update_attr.as_path_size * 2) + 2;
	encode_bgp_attr(BGP_message,&_bgp_path_attribute);
	encode_bgp_as_segment_type(BGP_message,&as_segment_type);
	encode_bgp_as_segment_len(BGP_message,&(update->update_attr.as_path_size));


	for(as_id = 0; as_id < update->update_attr.as_path_size; as_id++){
		u_int16_t as_path = update->update_attr.as_path[as_id];
		encode_bgp_as(BGP_message,&as_path);
			
	}
	

	_bgp_path_attribute.flags = TRANSITIVE_ATTRIBUTE;
	_bgp_path_attribute.type = NEXT_HOP;
	_bgp_path_attribute.len = 4;
	encode_bgp_attr(BGP_message,&_bgp_path_attribute);
	encode_bgp_next_hop(BGP_message,&(update->update_attr.next_hop));
	total_attr_len = BGP_message->size;

	LIST_FOREACH(nlri_p, &(update->update_nlri),pointers){
		
		encode_prefix_len(BGP_message,&(nlri_p->prefix_len));
		encode_prefix(BGP_message,&(nlri_p->prefix));
		
	}

	//LIST_FOREACH(update, &(neighbor_p->bgp_adj_rib_out) , pointers){
		
		
	//}
	
	memcpy(hdr.bgp_marker,"\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF",16);
	hdr.bgp_type = UPDATE;
	hdr.bgp_len = BGP_message->size + 23;
	printf("BGP message %d\n",BGP_message->size);
	BGP_message->index = 0;
	encode_bgp_header(BGP_message,&hdr);
	encode_len(BGP_message,&withdrawn_route_len);
	encode_len(BGP_message,&total_attr_len);
	
	printf("send update()\n");
	LIST_REMOVE(update,pointers);
	return BGP_message;
	
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
	
	if(!LIST_EMPTY(&(neighbor_p->bgp_adj_rib_out))){
		printf("bgp_adj_rib_out()\n");
		return send_update(neighbor_p);
	}
	
	if(event_hold_time(neighbor_p,180)){
		BGP_response = malloc(sizeof(struct packet));
		BGP_response->status = -1;
		//BGP_response->size = 0;
		BGP_response->data_size = 0;
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
		
		default:
		
			return NULL;
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
