#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

# include <stdio.h>
# include <netinet/in.h>
# define HEADER_BUF 8192
# define PORT "6969"
typedef struct {
	FILE *fd;
	long content_length;
	char * content_type;
} FileData ;

typedef struct {
	char method[8];
	char path[256];
	char protocol[20];
} HeaderData;

typedef struct {
	int sock_fd;
	char addr[INET6_ADDRSTRLEN];
} ThreadArgs;

void *get_in_addr(struct sockaddr *sa);
int setup_socket();
char *get_request(int fd);
int render_html(int socket_fd, FILE* file_fd);
char * status_lookup(int status_code);
void send_response(int socket_fd, int status_code, char *content_type, long content_length);
void send_error(int socket_fd, int status_code);
FileData * parse_file(char *file_name);
HeaderData * parse_request(char *request);
int handle_client(void *arg);
#endif // !SERVER_UTILS_H

