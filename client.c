//#include <WinSock.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
//#pragma comment(lib, "ws2_32.lib")
void error_handling(char *message);
#define bufsize 1024
int main()
{
    //WSADATA wsa;
    char buf[bufsize];
    int server_socket;
    server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if(SOCKET_ERROR == serve_socket)
    {}

}