#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <dirent.h>

#define BUF_SIZE 1024
void error_handling(char *message);

int main(int argc, char *argv[]){
    int serv_sock, clnt_sock;

    char message[BUF_SIZE];
    
    int str_len, i;

    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t clnt_adr_sz;

    if(argc!=2){
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if(serv_sock == -1) error_handling("socket() error");

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    if(bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1) error_handling("bind() error");

    if(listen(serv_sock, 5) == -1) error_handling("listen error");

    clnt_adr_sz = sizeof(clnt_adr);

    char end[] = "end";
    while(1){
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
        if(clnt_sock == -1) error_handling("accept() error");

        while(1){
            DIR* dir;
            struct dirent* entry;

            //file_server.c가 있는 파일의 경로
            char* folderPath = "/Volumes/Macintosh HD - Data/3-1 summer/MCNL/socket/study1";

            dir = opendir(folderPath);
            if(dir == NULL) {
                printf("Failed to open directory.\n");
            }

            while((entry = readdir(dir)) != NULL) {
                if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                    continue;

                // 파일 경로 생성
                char filePath[BUF_SIZE];
                snprintf(filePath, BUF_SIZE, "%s/%s", folderPath, entry->d_name);

                // 파일 정보 얻기
                struct stat fileStat;
                if (stat(filePath, &fileStat) == -1) {
                    printf("Failed to get file information.\n");
                    continue;
                }

                // 파일 크기와 이름 전송
                snprintf(message, BUF_SIZE, "%s: %lldByte", entry->d_name, fileStat.st_size);
                write(clnt_sock, message, BUF_SIZE);
            }
            
            write(clnt_sock, end, sizeof(end)+1);
            
            str_len = read(clnt_sock, message, BUF_SIZE);
            if (str_len == -1)
                error_handling("read() error");
            message[str_len] = '\0';

            if (strcmp(message, "2") == 0) {
                printf("클라이언트가 종료하였습니다.\n");
                break; // 클라이언트가 2를 보내면 서버 종료
            }

            printf("선택한 파일의 이름은 %s 입니다.\n", message);

            // client가 선택한 파일 열기
            FILE *file = fopen(message, "rb");
            if (file == NULL) {
                printf("Failed to open file.\n");
                continue;
            }
            // 파일 크기 구하기
            fseek(file, 0, SEEK_END);
            size_t file_size = ftell(file);
            fseek(file, 0, SEEK_SET);

            // 파일 크기 전송
            size_t file_size_n = htonl(file_size); // 호스트 바이트 순서를 네트워크 바이트 순서로 변환
            write(clnt_sock, &file_size_n, sizeof(size_t));

            // 파일 내용을 읽어 클라이언트로 전송
            while ((str_len = fread(message, 1, BUF_SIZE, file)) > 0) {
                write(clnt_sock, message, str_len);
            }
            fclose(file);
        }
    }
    close(clnt_sock);
    close(serv_sock);
    return 0;
}

void error_handling(char *message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
