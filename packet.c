#include <stdio.h>      // printf
#include <string.h>     // memset
#include <stdlib.h>     // exit(0);
#include <arpa/inet.h>  // inet_ntoa, ntohs
#include <sys/socket.h> // bind, recvfrom
#include <sys/queue.h>  // for Linkedlist
#include <unistd.h>     // close();
#include <stdbool.h> // for bool
#include <errno.h>
#include <poll.h>
#include <fcntl.h>
//#include "encode.h"
//#include "config.h"
#include "packet.h"

extern int errno;

int sendAll(int s, char *buf, int *len){
	
	printf("sendAll()\n");
	int total = 0;
	int bytesLeft = *len; 
	int n;
	
	while(total < *len){
		
	n = send(s,buf+total, bytesLeft,0);
	
	if(n == -1){ break;}
	total += n;
	bytesLeft -= n;
	}
	
	*len = total;
	
	if(n == -1){
		return -1;
	}
	
	else{
		return 0;
	}
	
}

int get_listener_socket(){
	
	
	struct sockaddr_in si_me;
    int s;

	
    if ((s=socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
    }
	
	 setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (int *)1, sizeof(int));
	/*
	if(fcntl(s,F_SETFL,O_NONBLOCK) == -1){
			perror("FCTNL\n");
	}
	*/
    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT); 
    si_me.sin_addr.s_addr = htonl(INADDR_ANY); 

    if(bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1){
        perror("bind");
		exit(1);
    }
	if(listen(s, BACKLOG) == -1){
		perror("listen");
		exit(1);
	}
	

	return s;
	
	
	
}

void add_to_pfds(struct pollfd *pfds[], int newfd, int *fd_count, int *fd_size)
{
    // If we don't have room, add more space in the pfds array
    if (*fd_count == *fd_size) {
        *fd_size *= 2; // Double it

        *pfds = realloc(*pfds, sizeof(**pfds) * (*fd_size));
    }

    (*pfds)[*fd_count].fd = newfd;
    (*pfds)[*fd_count].events = POLLIN | POLLOUT; // Check ready-to-read

    (*fd_count)++;
}


// Remove an index from the set
void del_from_pfds(struct pollfd pfds[], int i, int *fd_count)
{
    // Copy the one from the end over this one
    pfds[i] = pfds[*fd_count-1];

    (*fd_count)--;
}

void printPayload(unsigned char *data,unsigned int size){

        int i;
        for (i=0; i< size; i++){ printf(" %02X" ,(unsigned int) data[i]);
         if( i!=0 &&  i%16==0)  { 	 printf("\n"); }
        }

}

void connectionPool(){
	
	int s_listener;
	int s_other;
	int fd_count=0;
	int fd_size=5;
	int addrlen;
	int i;
	int recv_len;
	struct sockaddr_in si_me, si_other;
	
	unsigned char buf[BUFLEN] = {0};
	struct packet *p;
	struct packet *BGP_reply;
	
	p = calloc(0,sizeof(struct packet));
	
	struct pollfd *pool_fds = malloc(sizeof *pool_fds * fd_size);
	
	s_listener = get_listener_socket();
	
	if(s_listener == -1){
		
		perror("s_listener\n");
		exit(1);
	}
	pool_fds[0].fd = s_listener;
	pool_fds[0].events = POLLIN;
	fd_count++;
	
	while(1){
		
		int poll_count = poll(pool_fds,fd_count,-1);
		printf("poll()\n");
		
		if(poll_count == -1){
			perror("poll\n");
			exit(1);
		}
		
		for(i=0;i<fd_count;i++){
			
			if (pool_fds[i].revents & POLLIN) { 

                if (pool_fds[i].fd == s_listener) {
                    
                    addrlen = sizeof(si_other);
                    s_other = accept(s_listener,(struct sockaddr *)&si_other,&addrlen);

                    if (s_other == -1) {
                        perror("accept");
					}
					else{
						add_to_pfds(&pool_fds, s_other, &fd_count, &fd_size);
						printf("BGP_new_connection() from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
						BGP_new_connection(si_other.sin_addr);
						
					}

				}
				//In case not listener fd
				else{
					printf("recv()\n");
					if ((recv_len = recv(s_other, buf, BUFLEN, 0)) <= 0) {
						
						if(recv_len == 0){
							
							printf("connection closed\n");
						}
						else{
							perror("recv\n");
							
						}
						
						close(pool_fds[i].fd);
						del_from_pfds(pool_fds, i, &fd_count);
					}
					p->data = buf;
					p->index = 0;
					p->size = recv_len;
					BGP_reply = BGP_event_receive(p,si_other.sin_addr);
					
					if(BGP_reply!=NULL){
						sendAll(s_other,BGP_reply->data,&(BGP_reply->size));
						free(BGP_reply);
					}
				}
			}
			if (pool_fds[i].revents & POLLOUT){
				BGP_event_send();
				printf("round robin=%d\n",i);
				sleep(2);
				
				
			}

		
		
		
	}
	
	}
}


void startBgp(bool initiator){
	int i=0;
	return;

}
