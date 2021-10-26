#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>  //htonol, htons, INADDR_ANY, sockaddr_in 등등

int main(int argc, char* argv[]){
    int clnt_socket;
    int serv_socket;
    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size;

    serv_socket = socket(PF_INET, SOCK_STREAM, 0);
    if(serv_socket == -1)
    {
        printf("Socket error\n");
    }
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;                   //타입: ipv4
    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);    //ip주소
    serv_addr.sin_port=htons(atoi(argv[1]));        //port
}