#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>  // inet_ntoa, ntohs
#include <libconfig.h>


//libconfig documentation: https://hyperrealm.github.io/libconfig/libconfig_manual.html#Configuration-Files


int init_config(){
	
	const config_setting_t *cf_setting;
	const char *base = NULL;
	int count, n;
	long long int enabled;
	
	cf = &cfg;
	config_init(cf);
	
	if (!config_read_file(cf, "bgp.cfg")) {
		fprintf(stderr, "%s:%d - %s\n",
			config_error_file(cf),
			config_error_line(cf),
			config_error_text(cf));
		config_destroy(cf);
		return(EXIT_FAILURE);
	}
	return 0;
	
}

int config_get_neighbor(u_int32_t ip_address){
	
	printf("config_get_neighbor()\n");
	config_setting_t *setting;
	int neighbor_id;
	setting = config_lookup(&cfg, "router-bgp.neighbors");
	char *string;
	u_int32_t cfg_ip;
	
	if(setting != NULL){
		int count = config_setting_length(setting);
		for(neighbor_id=0;neighbor_id<count;neighbor_id++){
			config_setting_t *neighbor = config_setting_get_elem(setting, neighbor_id);
			config_setting_lookup_string(neighbor,"ip_address",&string);
			cfg_ip = inet_addr(string); // network byte order
			if(cfg_ip == ip_address){
				printf("configGetNeighbor().cfg_ip == ip_address; elem = %d\n",neighbor_id);
				return neighbor_id;
			}

		}
		
	}
	else{ return 0; }
	
}

u_int16_t config_get_asn(){
	
	printf("config_get_asn()\n");
	u_int16_t my_asn;
	config_setting_t *setting = config_lookup_int(&cfg, "router-bgp.my_asn",&my_asn);

	if(setting!=NULL){
		return my_asn;
	}
}
	
u_int8_t config_get_version(){
	
	printf("config_get_version()\n");
	u_int8_t version;
	config_setting_t *setting = config_lookup_int(&cfg, "router-bgp.version",&version);

	if(setting!=NULL){
		return version;
	}
	
}
	

u_int32_t config_get_id(){
	
	printf("config_get_id()\n");
	/*u_int32_t my_asn;
	config_setting_t *setting = config_lookup_int(&cfg, "router-bgp.my_asn",&my_asn);

	if(setting!=NULL){
		return my_asn;
	}
	*/
}