#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h> //htonol, htons, INADDR_ANY, sockaddr_in 등등

int main(int argc, char *argv[])
{
    int clnt_socket;
    int serv_socket;
    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size;

    serv_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_socket == -1)
    {
        printf("Socket error\n");
    }
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;                                                  //타입: ipv4
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);                                   // ip주소
    serv_addr.sin_port = htons(atoi(argv[1]));                                       // port
    if (bind(serv_socket, (struct socketaddr *)&serv_addr, sizeof(serv_addr)) == -1) // socket과 서버주소를 바인딩
        error_handling("bind error");
    //연결 대기열 5개 생성
    if (listen(serv_soket, 5) == -1)
        error_handling("listen error");
    //클라이언트로부터 요청이 오면 연결 수락
    clnt_addr_size = sizeof(clnt_addr);
    clnt_sock = accept(serv_socket, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
    if (clnt_sock == -1)
        error_handling("accept error");
    //데이터 전송
    char msg[] = "Hello\n";
    write(clnt_sock, msg, sizeof(msg));
    //소켓들 닫기
    close(clnt_socket);
    close(serv_socket);
    return 0;
}
void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}