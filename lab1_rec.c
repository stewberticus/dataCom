#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char** argv){
	int sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd < 0){
		printf("error");
		return 1; 
	}
	struct sockaddr_in serveraddr,clientaddr;
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_port=htons(9876);
	serveraddr.sin_addr.s_addr = INADDR_ANY;
	 
	bind(sockfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr));
	listen(sockfd,10);
	while(1){
	int len = sizeof(clientaddr);
	int clientsocket = accept(sockfd,(struct sockaddr*)&clientaddr,&len);
	char line[5000];
	recv(clientsocket,line,5000,0);
	printf("Got from client %s\n",line);
	char *p = line;
	p[strlen(p)-1] = 0;
	strcat(line,"!!!");
	send(clientsocket,line,strlen(line),0);
	close(clientsocket);
	}
	return 0; 
	

	
}
