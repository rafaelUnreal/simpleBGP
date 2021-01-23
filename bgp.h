#ifndef BGP_H
#define BGP_H

#include <stdlib.h>     // exit(0);
#include <arpa/inet.h>  // inet_ntoa, ntohs
#include <sys/queue.h>  // for Linkedlist
#include <unistd.h>     // close();
#include <sys/queue.h>  // for Linkedlist
#include <time.h>
#include <stdint.h>
#include "packet.h"


#define TRANSITIVE_ATTRIBUTE 0x40
#define ORIGIN	0x01
#define AS_PATH	0x02
#define AS_SEQUENCE 0x02
#define NEXT_HOP 0x03
#define UPDATE	0x02
#define BGP_HDR_SIZE	19



//BGP message type codes

#define KEEP_ALIVE 0x4
#define OPEN_MESSAGE 0x1
#define UPDATE 0x2

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
	LIST_ENTRY(nlri) pointers;
	
};

struct bgp_advertisment {
	
	struct bgp_attr update_attr;
	LIST_HEAD(nlri_list, nlri) update_nlri;
	LIST_ENTRY(bgp_advertisment) pointers;
};

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
	u_int8_t	pending_update;
	
	LIST_HEAD(bgp_list_out, bgp_advertisment) bgp_adj_rib_out;
	LIST_HEAD(bgp_list_in, bgp_advertisment) bgp_adj_rib_in;
	STAILQ_ENTRY(bgp_neighbors) pointers;

};

void init_local_advertisement(struct bgp_neighbor *neighbor);
void load_neighbor_rib(struct bgp_neighbor *neighbor);
void load_neighbor(struct bgp_neighbor *neighbor);
void BGP_new_connection(int s_other, uint32_t ipv4_address);

//getters and setters
int BGP_get_optional_param_len();
int BGP_set_open_message(struct bgp_hdr *hdr, struct bgp_open *open);
int BGP_set_keepalive(struct bgp_hdr *keepalive);
void BGP_set_local_attr(struct bgp_attr *igp_attr);

//Events
struct packet* BGP_established(struct packet *p,struct bgp_neighbor *neighbor_p);
struct packet* BGP_event_send(int fd);
struct packet* BGP_event_receive(struct packet *p,uint32_t ipv4_address);
int event_hold_time(struct bgp_neighbor *neighbor_p, int max_elapsed_time);
int event_keepalive(struct bgp_neighbor *neighbor_p, int max_elapsed_time);

int BGP_open_message_check(struct bgp_open *msg, uint32_t ipv4_address);

//States
struct packet* BGP_active(struct packet *p,struct bgp_neighbor *neighbor_p);
struct packet* BGP_open_confirm(struct packet *p,struct bgp_neighbor *neighbor_p);

//messages
struct packet* send_keepalive();
struct packet* send_update(struct bgp_neighbor *neighbor_p);




























#endif
