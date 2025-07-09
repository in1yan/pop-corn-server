#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <threads.h>

#define PORT "6969"
#define BACKLOG 10
#define HEADER_BUF 8192

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



void *get_in_addr(struct sockaddr *sa){
	if(sa->sa_family == AF_INET)
		return &(((struct sockaddr_in *)sa)->sin_addr);

	return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}
int setup_socket(){

	int sockfd=-1;
	struct addrinfo hints, *servinfo, *p;
	int yes = 1;
	int rv;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;

	if((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0){
		fprintf(stderr, "getaddrinfo : %s\n", gai_strerror(rv));
		return -1;
	}

	for(p=servinfo; p!=NULL; p=p->ai_next){
		if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol))==-1){
			perror("server: socket");
			continue;
		}
		if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
			perror("setsockopt");
			continue;
		}
		if((bind(sockfd, p->ai_addr, p->ai_addrlen)) == -1){
			close(sockfd);
			perror("server: bind");
			continue;
		}
		break;
	}
	freeaddrinfo(servinfo);
	return sockfd;
}
char *get_request(int fd){
	char *request = malloc(HEADER_BUF);
	int total = 0, n;
	while((n= recv(fd, request+total, HEADER_BUF-total-1, 0)) > 0){
		total += n;
		request[total] = '\0';
		if(strstr(request, "\r\n\r\n")) return request;
		if(total >= HEADER_BUF){
			perror("server: request");
			free(request);
			return NULL;
		}
	}
	return request;	
}
int render_html(int socket_fd, FILE* file_fd){
	char buffer[1024];
	size_t n;
	if(!file_fd){
		return -1;	
	}
	while((n=fread(buffer, 1, sizeof(buffer), file_fd)) >0)
		send(socket_fd, buffer,n, 0);
	return 1;	
}
char * status_lookup(int status_code){
	switch (status_code) {
		case 200: return "OK";
		case 400: return "Bad Request";
		case 403: return "Forbidden";
		case 404: return "Not Found";
		case 500: return "Internal Server Error";
		default:return "Unknown";
	}
}
void send_response(int socket_fd, int status_code, char *content_type, long content_length){
	char header[512];
	snprintf(header, sizeof(header),
		  "HTTP/1.1 %d %s\r\n"
		  "Content-Type: %s\r\n"
		  "Content-Length: %ld\r\n"
		  "Connection: close\r\n"
		  "\r\n", status_code, status_lookup(status_code), content_type, content_length);
	send(socket_fd, header, strlen(header), 0);

}
void send_error(int socket_fd, int status_code){
	char body[512];
	snprintf(body, sizeof(body), "<h1> %d %s</h1>", status_code, status_lookup(status_code));
	send_response(socket_fd, status_code, "text/html", strlen(body));
	send(socket_fd, body, strlen(body), 0);
}
FileData * parse_file(char *file_name){
	FILE *fd = fopen(file_name, "r");
	if(!fd) return NULL;
	FileData *result = (FileData *)malloc(sizeof(FileData));
	fseek(fd, 0, SEEK_END);
	long int content_length = ftell(fd);
	fseek(fd, 0, SEEK_SET);
	char *ext = strrchr(file_name, '.');
	char *mime;
	if(!ext) mime= "application/octet-stream";
	else if(strcmp(ext, ".html") == 0)  mime = "text/html";
	else if(strcmp(ext, ".css") == 0) mime= "text/css";
	else if(strcmp(ext, ".js") == 0) mime= "application/javascript";
	else if(strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) mime= "image/jpeg";
	else if(strcmp(ext, ".png") == 0) mime= "image/png";
	else if(strcmp(ext, ".webp") == 0) mime= "image/webp";
	else if(strcmp(ext, ".gif") == 0) mime= "image/gif";
	else mime = "application/octet-stream";
	result->content_length = content_length;
	result->fd = fd;
	result->content_type = mime;
	return result;
}

HeaderData * parse_request(char *request){
	HeaderData *result = (HeaderData *)malloc(sizeof(HeaderData));

	char *first_line = strtok(request, "\r\n");
	sscanf(first_line, "%s %s %s", result->method, result->path, result->protocol);
	char header[256];
	if(strcmp(result->path, "/") == 0)
		strcpy(result->path, "index.html");
	else{
		snprintf(result->path, sizeof(result->path), "%s",result->path+1);
	}
	return result;
}

int handle_client(void *arg){
	ThreadArgs *args = (ThreadArgs *)arg;
	char *request = get_request(args->sock_fd);
	if(!request){
		fprintf(stderr, "[%s] Failed to get request\n", args->addr);
		send_error(args->sock_fd, 400);
		close(args->sock_fd);
		free(args);
		return -1;
	}
	// parse the request to determine the html file
	HeaderData *parsed_request = parse_request(request);
	if(!parsed_request){
		fprintf(stderr, "[%s] Failed to parse request\n", args->addr);
		send_error(args->sock_fd, 400);
		close(args->sock_fd);
		free(args);
		return -1;
	}
	char header[256];
	printf("[ %s ] --> %s %s %s\n", args->addr, parsed_request->method,  parsed_request->path, parsed_request->protocol);
	// setup the response header
	FileData *response_header_data = parse_file(parsed_request->path);
	if(!response_header_data){
		send_error(args->sock_fd, 404);
		close(args->sock_fd);
		free(args);
		free(parsed_request);
		return -1;
	}
	FILE *template = response_header_data->fd;
	send_response(args->sock_fd, 200, response_header_data->content_type, response_header_data->content_length);
	render_html(args->sock_fd, template);
	shutdown(args->sock_fd, SHUT_WR);
	fclose(template);
	close(args->sock_fd);
	free(parsed_request);
	free(response_header_data);
	free(args);
	return 0;
}
int main(){
	struct sockaddr_storage their_addr;
	int new_fd;
	socklen_t sin_size;
	int numbytes;
	// socket creation
	int sockfd = setup_socket();
	if(sockfd==-1){
		fprintf(stderr, "server: failed to bind");
		exit(1);
	}
	if(listen(sockfd, BACKLOG) == -1){
		perror("listen");
		exit(1);
	}
	printf("Waiting for connections...\n");
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

