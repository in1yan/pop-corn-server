#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <threads.h>
#include "server_utils.h"
#include <arpa/inet.h>
#define HEADER_BUF 8192

char PORT[5];
int BACKLOG;

int main(){
	Configs *config =parse_config();
	strcpy(PORT, config->port);
	BACKLOG = config->backlog;
	struct sockaddr_storage their_addr;
	int new_fd;
	socklen_t sin_size;
	int numbytes;
	// socket creation
	int sockfd = setup_socket(PORT);
	if(sockfd==-1){
		fprintf(stderr, "server: failed to bind");
		exit(1);
	}
	if(listen(sockfd, BACKLOG) == -1){
		perror("listen");
		exit(1);
	}
	printf("Waiting for connections...\n");
	fflush(stdout);
	// start listening to connections
	while(1){
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if(new_fd == -1){
			perror("accept");
			continue;
		}
		ThreadArgs *args = malloc(sizeof(ThreadArgs));
		args->sock_fd = new_fd;
		args->root = config->root;
		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), args->addr, sizeof(  args->addr )); // convert network address to presentable
		thrd_t t;
		if ( thrd_create(&t, handle_client, args) != thrd_success ){
			perror("Failed to create a thread");
			send_error(new_fd, 500);
			free(args);
			continue;
		}
		thrd_detach(t);
	}
}

