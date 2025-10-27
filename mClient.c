#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>

int main(){
    int sock = 0,vr=0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    char msg[1024];

    sock = socket(AF_INET,SOCK_STREAM,0);
    if(sock < 0){
    	perror("creation failed \n");
	return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);

    if(inet_pton(AF_INET,"127.0.0.1",&serv_addr.sin_addr) <= 0){
    	perror("invalid address \n");
	return 1;
    }

    if(connect(sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr)) < 0){
    	perror("connection failed \n");
	return 1;
    }

    printf("connected to server at 127.0.0.1:2019 \n");

    while(1){
    	printf("enter msg : ");
	fgets(msg,sizeof(msg),stdin);
	msg[strcspn(msg,"\n")] = '\0';

	send(sock,msg,strlen(msg),0);
	if(strcmp(msg,"over") == 0){
	    break;
	}

	vr = read(sock,buffer,sizeof(buffer));
	buffer[vr] = '\0';

	printf("server : %s \n",buffer);
    }
    close(sock);
    return 0;
}
