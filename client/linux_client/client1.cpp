#include <stdio.h>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
using namespace std;
#define BUF_SIZE 100
#define NAME_SIZE 20

void *send_msg(void *arg);
void *recv_msg(void *arg);
void error_handling(char *msg);

char name[NAME_SIZE] = "[DEFAULT]";
char msg[BUF_SIZE];

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in serv_addr;

    //쓰레드 송신 , 쓰레드 수신
    pthread_t snd_thread, rcv_thread;
    void *thread_return;

    if (argc != 4)
    {
        printf("usage : %s <ip> <port> <name> \n", argv[0]);
        exit(1);
    }
    cout << sprintf(name, "[%s]", argv[3]);
    //IPv4, TCP 소켓 생성
    sock = socket(PF_INET, SOCK_STREAM, 0);

    //서버 주소정보 초기화
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    //서버 주소 정보를 기반으로 연결요청, 이때 비로소 클라이언트 소켓이됨.
    connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    //쓰레드 생성 및 실행
    pthread_create(&snd_thread, NULL, send_msg, (void *)&sock);
    pthread_create(&rcv_thread, NULL, recv_msg, (void *)&sock);

    //쓰레드 종료까지 대기
    pthread_join(snd_thread, &thread_return);
    pthread_join(rcv_thread, &thread_return);

    //클라이언트 소켓 연결 종료
    close(sock);
    return 0;
}

//송신
void *send_msg(void *arg)
{

    //클라이언트의 파일 디스크립터
    int sock = *((int *)arg);
    char name_msg[NAME_SIZE + BUF_SIZE];

    while (1)
    {

        //콘솔에서 문자열 입력 받고
        fgets(msg, BUF_SIZE, stdin);

        if (!strcmp(msg, "q\n") || !strcmp(msg, "Q\n"))
        {
            //클라이언트 소캣 종료
            close(sock);
            //프로그램 종료
            exit(1);
        }
        //클라이언트 이름과 msg 합침
        sprintf(name_msg, "%s %s", name, msg);

        //null 문자 제외하고 서버로 문자열 보냄
        write(sock, name_msg, strlen(name_msg));
    }

    return NULL;
}

//수신
void *recv_msg(void *arg)
{

    //클라이언트의 파일 디스크립터
    int sock = *((int *)arg);
    char name_msg[NAME_SIZE + BUF_SIZE];
    int str_len;

    while (1)
    {

        str_len = read(sock, name_msg, NAME_SIZE + BUF_SIZE - 1);
        //read 실패시
        if (str_len == -1)
        {
            return (void *)-1;
        }

        name_msg[str_len] = '\0';

        //콘솔에 출력
        fputs(name_msg, stdout);
    }

    return NULL;
}

//에러 처리
void error_handling(char *msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}