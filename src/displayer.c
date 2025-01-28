#include <asm-generic/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <ncurses.h>
#include <locale.h>
#include "sys/epoll.h"

#define PORT 7700
#define MAX_BUFFER 8192
#define MAX_EVENTS 64

void writeCenter(int,char *);
void IterateLyric(int,char*,int,int);
int CountRow(char* buffer);

int main(int agrc, char* argv[]){

    setlocale(LC_ALL, "");

    int started = 0;
    int scrollRow = 10;
    int availableRow ;
    int xMax,yMax;

    MEVENT event;

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);

    //printf("attempting\n");
    int sockfd, newsockfd;
    char buffer[MAX_BUFFER];
    struct sockaddr_in serv_addr, cli_addr;
    struct epoll_event events[MAX_EVENTS];

    if((sockfd = socket(AF_INET,SOCK_STREAM,0))<0){
        perror("Socket");
        exit(1);
    }
    //printf("socket");
    int opt = 1;
    if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt))<0){
        close(sockfd);
        perror("SOCK OPT");
        exit(1);
    }
    //printf("setopt\n");

    memset(&serv_addr, 0, sizeof(serv_addr));
    memset(&cli_addr, 0, sizeof(cli_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    int epfd = epoll_create(1);
    struct epoll_event sock_event;

    if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof( serv_addr) )<0){
        perror("BIND");
        close(sockfd);
        exit(1);
    }
    //printf("Bind success\n");

    listen(sockfd, 3);
    //printf("Listening on port: %d\n", PORT);
    socklen_t clilen = sizeof((cli_addr));

    if((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen))<0){
        perror("ACCEPT");
        close(sockfd);
        exit(1);
    }
    //printf("Accept\n");

    sock_event.events = EPOLLIN | EPOLLET;
    sock_event.data.fd = newsockfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD,newsockfd,&sock_event);

    while (1){
        int ready = epoll_wait(epfd,events,MAX_EVENTS,100);

        for (int i = 0; i<ready;i++){

            memset(buffer, 0, sizeof(buffer));
            int bytes_recvd;
            if ((bytes_recvd = recv(newsockfd,buffer,MAX_BUFFER - 1,0))<0){
                perror("RECV");
                exit(1);
            }
            buffer[bytes_recvd] = '\0';

            scrollRow = 0;
            clear();
            availableRow = CountRow(buffer);
            //mvprintw(2, 1, "%s\n",buffer);
            IterateLyric(2, buffer,scrollRow,availableRow);
            refresh();
            
        }

        int input = getch();

        if (input != ERR){
            if (input == KEY_DOWN){
                int maxRow = getmaxy(stdscr);
                if((availableRow - scrollRow) <= maxRow)continue;
                scrollRow++;
                clear();
                IterateLyric(2, buffer,scrollRow,availableRow);
                refresh();
            }
            else if(input == KEY_UP){
                if(scrollRow <= 0)continue;
                scrollRow--;
                clear();
                IterateLyric(2, buffer,scrollRow,availableRow);
                refresh();
            }
        }
        

    }
}

void writeCenter(int row,char* buffer){
    int textLength = strlen(buffer);
    //int startX = (COLS - textLength)/2;
    mvprintw(row, 3, "%s", buffer);
}

void IterateLyric(int startRow,char*Lyric,int scrollRow,int availableRow){
    char *lyricDup = strdup(Lyric);
    char * lyricLine = strtok(lyricDup, "\n");
    int row = startRow;

    int rowNow = scrollRow;
    while(rowNow > 0) {
        lyricLine = strtok(NULL, "\n");
        rowNow -- ;
    }

    while (lyricLine != NULL) {
        writeCenter(row++, lyricLine);
        lyricLine = strtok(NULL, "\n");
    }
    free(lyricDup);
}

int CountRow(char* buffer){
    int i = 0;
    while(*buffer !='\0'){
        if(*buffer == '\n') i++;
        buffer++;
    }
    return i;
}