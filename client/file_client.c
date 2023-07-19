#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024
void error_handling(char *message);

int main(int argc, char *argv[]){
    int sock;
    char message[BUF_SIZE];
    char filelist[BUF_SIZE];
    size_t filesize = 0;
    int str_len, bb;
    struct sockaddr_in serv_adr;

    if(argc!=3){
        printf("Usage: %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock==-1) error_handling("socket() error");

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port = htons(atoi(argv[2]));

    if(connect(sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))== -1) error_handling("connect() error");
    else puts("Connected..........");

    int cho;
    char filename;
    while(1){
        printf("CHOOSE MENU-----------------------\n");
        printf("1. Download\n");
        printf("2. Quit\n");

        printf("Select number: ");
        scanf("%d", &cho);

        if(cho == 2) {
            break;
        }
        printf("\n");

        if (cho == 1) {
            printf("FILE LIST-------------------------\n");
            while((str_len = read(sock, filelist, BUF_SIZE)) > 0){
                if(strcmp(filelist, "end") == 0) break;
                if(str_len == -1) error_handling("read() error");

                printf("%s\n", filelist);
            }
            printf("\n\n");

            printf("Select a file name to download: ");
            scanf("%s", &filename);
            write(sock, &filename, strlen(&filename)+1);

            // 파일 크기 읽기
            read(sock, &filesize, sizeof(size_t));
            filesize = ntohl(filesize); // 네트워크 바이트 순서를 호스트 바이트 순서로 변환

            char* folderPath = "/Volumes/Macintosh HD - Data/3-1 summer/MCNL/socket/client";

            // 파일 경로 생성
            char filePath[BUF_SIZE];
            snprintf(filePath, BUF_SIZE, folderPath, filename);

            FILE *file = fopen(&filename, "wb");
            if (file == NULL) {
                printf("Failed to create file.\n");
                close(sock);
                exit(1);
            }

            while(filesize > 0)
            {
                bb = BUF_SIZE;
                if(filesize < BUF_SIZE)
                    bb = read(sock, message, filesize);
                else
                    bb = read(sock, message, BUF_SIZE);

                if(bb == -1)
                {
                    error_handling("read() error");
                    break;
                }

                fwrite(message, sizeof(char), bb, file);
                filesize -= bb;
            }
            printf("download 완료\n\n");
            fclose(file);
        }
    }
    close(sock);
    return 0;
}

void error_handling(char *message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}