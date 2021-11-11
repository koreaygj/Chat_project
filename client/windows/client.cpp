#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <process.h>

#define BUF_SIZE 100
#define NAME_SIZE 20

unsigned WINAPI SendMsg(void *arg); //쓰레드 전송함수
unsigned WINAPI RecvMsg(void *arg); //쓰레드 수신함수
void error_handling(char *msg);

char name[NAME_SIZE] = "[DEFAULT]";
char msg[BUF_SIZE];

int main(int argc, char *argv[])
{
  WSADATA wsaData;
  SOCKET sock;
  SOCKADDR_IN server_addr;
  HANDLE sendThread, recvThread;
  if (argc != 4)
  {
    printf("Usage : %s <IP> <port> <name>\n", argv[0]);
    exit(1);
  }
  // 윈도우 소켓을 사용한다고 운영체제에 알림
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
  {
    char *error_msg = (char *)"WSAStartup() error!";
    error_handling(error_msg);
  }
  sprintf(name, "[%s]", argv[3]);
  //소켓을 하나 생성한다.
  sock = socket(PF_INET, SOCK_STREAM, 0);

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(argv[1]);
  server_addr.sin_port = htons(atoi(argv[2]));
  //서버에 접속한다.
  if (connect(sock, (SOCKADDR *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
  {
    char *error_msg = (char *)"connect() error";
    error_handling(error_msg);
  }

  //접속에 성공하면 이 줄 아래가 실행된다.

  sendThread = (HANDLE)_beginthreadex(NULL, 0, SendMsg, (void *)&sock, 0, NULL); //메시지 전송용 쓰레드가 실행된다.
  recvThread = (HANDLE)_beginthreadex(NULL, 0, RecvMsg, (void *)&sock, 0, NULL); //메시지 수신용 쓰레드가 실행된다.

  WaitForSingleObject(sendThread, INFINITE); //전송용 쓰레드가 중지될때까지 기다린다.
  WaitForSingleObject(recvThread, INFINITE); //수신용 쓰레드가 중지될때까지 기다린다.
  //클라이언트가 종료를 시도한다면 이줄 아래가 실행된다.
  closesocket(sock); //소켓을 종료한다.
  WSACleanup();      //윈도우 소켓 사용중지를 운영체제에 알린다.
  return 0;
}

unsigned WINAPI SendMsg(void *arg)
{                                 //전송용 쓰레드함수
  SOCKET sock = *((SOCKET *)arg); //서버용 소켓을 전달한다.
  char name_msg[NAME_SIZE + BUF_SIZE];
  while (1)
  {                              //반복
    fgets(msg, BUF_SIZE, stdin); //입력을 받는다.
    if (!strcmp(msg, "q\n") || !strcmp(msg, "Q\n"))
    { // q를 입력하면 종료한다.
      closesocket(sock);
      exit(1);
    }
    sprintf(name_msg, "%s %s", name, msg);     // name_msg에 메시지를 전달한다.
    send(sock, name_msg, strlen(name_msg), 0); // name_msg를 서버에게 전송한다.
  }
  return 0;
}

unsigned WINAPI RecvMsg(void *arg)
{
  SOCKET sock = *((SOCKET *)arg); //서버용 소켓을 전달한다.
  char name_msg[NAME_SIZE + BUF_SIZE];
  int strLen;
  while (1)
  {                                                             //반복
    strLen = recv(sock, name_msg, NAME_SIZE + BUF_SIZE - 1, 0); //서버로부터 메시지를 수신한다.
    if (strLen == -1)
      return -1;
    name_msg[strLen] = 0;    //문자열의 끝을 알리기 위해 설정
    fputs(name_msg, stdout); //자신의 콘솔에 받은 메시지를 출력한다.
  }
  return 0;
}

void error_handling(char *msg)
{
  fputs(msg, stderr);
  fputc('\n', stderr);
  exit(1);
}