#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUF_SIZE 100
#define MAX_CLNT 100

void *handle_clnt(void *arg);
void send_msg(char *msg, int len);
void error_handling(char *message);
//서버에 접속한 클라이언트 수
int clnt_cnt = 0;
//클라이언트와의 송수신을 위해 생성한 소켓의 파일 디스크립터를 저장한 배열
int clnt_socks[MAX_CLNT];
//뮤텍스를 통한 쓰레드 동기화를 위한 변수
pthread_mutex_t mutx;

int main(int argc, char *argv[])
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_sz;
    pthread_t t_id;
    //실행파일 경로 /port 번호 입력
    if (argc != 2)
    {
        printf("usage : %s <port> \n", argv[0]);
        exit(1);
    }
    //뮤텍스 생성
    pthread_mutex_init(&mutx, NULL);
    //IPv4, TCP 소캣 생성
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    //서버주소 정보 초기화
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));
    //서버주소 정보를 기반으로 주소 할당
    bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    //서버소켓 (리스닝 소켓)이 됨
    //연결요청 대기 큐를 생성하고, 클라이언트의 요청을 기다림
    listen(serv_sock, 5);
    while (1)
    {
        clnt_addr_sz = sizeof(clnt_addr);
        /*
         클라이언트의 연결요청 수락하고
         클라이언트와의 송수신을 위한 새로운 소캣 생성
         */
        clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_sz);
        //클라이언트의 ip정보를 문자열로 변환 후 출력
        printf("connected client ip : %s \n", inet_ntoa(clnt_addr.sin_addr));
        pthread_mutex_lock(&mutx); //뮤텍스 lock
        //클라이언트 수와 파일 디스크립터 등록
        clnt_socks[clnt_cnt++] = clnt_sock;
        pthread_mutex_unlock(&mutx); //뮤텍스 unlock
        printf("member count : %lu \n", sizeof(clnt_socks));
        printf("clnt_cnt : %d \n", clnt_cnt);
        //handle_clnt 등록 - clnt_sock 을 인자값으로 넘김
        //쓰레드 생성 및 실행
        pthread_create(&t_id, NULL, handle_clnt, (void *)&clnt_sock);
        //쓰레드가 종료되면 소멸시킴
        pthread_detach(t_id);
    }
    return 0;
}
//클라이언트 처리
void *handle_clnt(void *arg)
{
    int clnt_sock = *((int *)arg);
    int str_len = 0, i;
    char msg[BUF_SIZE];
    printf("handle_clnt 스레드 진입. 클라이언트 소켓 번호:  %d \n", clnt_sock);
    //클라이언트로부터 eof 를 수신할때까지 읽어서
    while ((str_len = read(clnt_sock, msg, sizeof(msg))) != 0)
    {
        //send_msg 함수 호출
        send_msg(msg, str_len);
    }
    printf("클라이언트 소켓 삭제 시작\n");
    pthread_mutex_lock(&mutx); //뮤텍스 lock
    //disconnected 된 클라이언트 삭제
    for (i = 0; i < clnt_cnt; i++)
    {
        printf("clnt_sock : %d \n", clnt_sock);
        printf("clnt_socks[%d] : %d \n", i, clnt_socks[i]);
        //현재 해당하는 파일 디스크립터를 찾으면
        if (clnt_sock == clnt_socks[i])
        {
            //클라이언트가 연결요청을 했으므로 해당 정보를 덮어씌워 삭제
            while (i < clnt_cnt - 1)
            {
                puts("클라이언트가 연결요청을 했으므로 해당 정보를 덮어씌워 삭제");
                printf("i : %d \n", i);
                printf("clnt_socks[i+1] : %d \n", clnt_socks[i + 1]);
                clnt_socks[i] = clnt_socks[i + 1];
                i++;
            }
            break;
        }
    }
    clnt_cnt--;                  //클라이언트 수 감소
    pthread_mutex_unlock(&mutx); //뮤텍스 unlock
    close(clnt_sock);            //클라이언트와의 송수신을 위한 생성했던 소켓종료
    printf("클라이언트 소켓 삭제 및 스레드 종료 \n");
    printf("현재 clnt_cnt : %d \n", clnt_cnt);
    return NULL;
}
//send to all
void send_msg(char *msg, int len)
{
    int i;
    pthread_mutex_lock(&mutx); //뮤텍스 lock
    for (i = 0; i < clnt_cnt; i++)
    {
        //현재 연결된 모든 클라이언트에게 메시지 전송
        write(clnt_socks[i], msg, len);
    }
    pthread_mutex_unlock(&mutx); //뮤텍스 unlock
}
void error_handling(char *msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}