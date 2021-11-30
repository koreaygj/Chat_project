#include <iostream>
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
    // IPv4, TCP 소캣 생성
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
        // handle_clnt 등록 - clnt_sock 을 인자값으로 넘김
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
        // send_msg 함수 호출
        send_msg(msg, str_len);
    }
    printf("클라이언트 소켓 삭제 시작\n");
    pthread_mutex_lock(&mutx); //뮤텍스 lock
    // disconnected 된 클라이언트 삭제
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
// send to all
void send_msg(char *msg, int len)
{
    pthread_mutex_lock(&mutx); //뮤텍스 lock
    for (int i = 0; i < clnt_cnt; i++)
    {
        //현재 연결된 모든 클라이언트에게 메시지 전송
        write(clnt_socks[i], msg, len);
    }
    pthread_mutex_unlock(&mutx); //뮤텍스 unlock
}
// void send_file(char *msg, int len)
// {
//     pthread_mutex_lock(&mutx);
//     for (int i = 0; < clnt_cnt; i++)
//     {
//         // jpg
//         size_t fsize, nsize = 0;
//         size_t fsize2;
//         /* 전송할 파일 이름을 작성합니다 */
//         file = fopen("aurora.jpg" /* 파일이름 */, "rb");
//         /* 파일 크기 계산 */
//         // move file pointer to end
//         fseek(file, 0, SEEK_END);
//         // calculate file size
//         fsize = ftell(file);
//         // move file pointer to first
//         fseek(file, 0, SEEK_SET);
//         // send file contents
//         while (nsize != fsize)
//         {
//             // read from file to buf
//             // 1byte * 256 count = 256byte => buf[256];
//             int fpsize = fread(buf, 1, 256, file);
//             nsize += fpsize;
//             send(serv_sock, buf, fpsize, 0);
//         }
//         fclose(file);
//         close(serv_sock);
//         return;
//     }
//     pthread_mutex_unlock(&mutx);
// }
// int save_file(char *file_buf, int filelen)
// {
//     char *split_pos = NULL;
//     char fullpath[MAX_PATH] = "", filename[MAX_PATH] = "";
//     int filename_len = 0, filecontent_len = 0;
//     split_pos = strchr(file_buf, FILE_DELIMITER);
//     if (save_folder[strlen(save_folder) - 1] == '\\')
//         strcpy(fullpath, save_folder);
//     else
//         sprintf(fullpath, "%s\\", save_folder);
//     strncpy(filename, file_buf, split_pos - file_buf);
//     strcat(fullpath, filename);
//     filename_len = strlen(filename);
//     filecontent_len = filelen - (filename_len + sizeof(FILE_DELIMITER));
//     if (write_file(fullpath, file_buf + strlen(filename) + sizeof(FILE_DELIMITER), filecontent_len) != 0)
//     {
//         puts("write_file 에러\n");
//         return -1;
//     }
//     printf(" >> 수신한 파일명 : %s, 파일크기 : %d바이트\n", fullpath, filecontent_len);
//     return 0;
// }
// int recv_file(int socket)
// {
//     PACKET_HEADER trans_header;
//     char *header = (char *)&trans_header;
//     char *file_buf = NULL, *data_buf = NULL;
//     int header_len = 0, filelen = 0;
//     int read_len = 0, data_len = 0;

//     header_len = 5;
//     memset(&trans_header, 0, sizeof(PACKET_HEADER));
//     while (read_len = recv_socket(socket, (char *)header, header_len))
//     {
//         if (read_len == -1)
//             return -1;
//         else if (read_len == header_len)
//             break;

//         header_len -= read_len;
//         header += read_len;
//     }
//     memcpy(&trans_header.len, &header[1], 4);
//     filelen = ntohl(trans_header.len);
//     if ((filelen <= 0) || (trans_header.type != PACKET_TYPE))
//         return -1;
//     if ((file_buf = (char *)calloc(1, filelen + 1)) == NULL)
//         return -1;
//     read_len = 0;
//     data_buf = file_buf;
//     data_len = filelen;
//     while (1)
//     {
//         read_len = recv_socket(socket, (char *)data_buf, data_len);
//         if (read_len == -1)
//         {
//             puts("recv_socket 에러");
//             return -1;
//         }
//         else if (read_len == data_len)
//             break;
//         data_len -= read_len;
//         data_buf += read_len;
//     }
//     save_file(file_buf, filelen);
//     free(file_buf);
//     return 0;
// }