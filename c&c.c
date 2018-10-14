#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>       // FD_SET, FD_ISSET, FD_ZERO - FD = FILE DESCRIPTOR
#include <netinet/in.h>
#include <arpa/inet.h>      // CONVERSION METHODS htons, htonl, ntohl, ntohs..
#include <errno.h>          // DECLARATIONS FOR ERROR CONSTANTS
#include <pthread.h>
#include "fatal.h"
#include "hexdump.h"
#include "exec_shell.h"

#define PORT 9090

// &X ADDR OF X
// *X POINTER OF X

int main() {
    int server_socket, client_socket[30], new_socket;
    int max_clients = 30, max_sd, addrlen, sd, activity, i, valread;
    int enabled = 1, recv_length=1, con_clients=0;
    socklen_t sin_size;
    char buffer[1024]; // 1KB DATA BUFFER
    int command; // COMMAND FROM SERVER INPUT
    int debug = 1;
    fd_set readfds;    // SET FILE DESCRIPTORS FOR FUTURE CONNECTIONS

    printf(R"EOF(
        8888888888P         d8888  88888888888  .d88888b.   .d8888b.
              d88P         d88888      888     d88P' 'Y88b  d88P  Y88b
             d88P         d88P888      888     888     888  Y88b.
            d88P         d88P 888      888     888     888   'Y888b.
           d88P         d88P  888      888     888     888      'Y88b.
          d88P         d88P   888      888     888     888        '888
         d88P         d8888888888      888     Y88b. .d88P  Y88b  d88P
        d8888888888  d88P     888      888      'Y88888P'    'Y8888P'

)EOF");

    char *message= "WELCOME TO THE HIVENET\n\r";

    for(i = 0; i < max_clients; i++){
        client_socket[i] = 0;               // INITIALIZE ALL CLIENT SOCKETS
    }

    if((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        fatal("SERVER SOCKET COULD NOT BE STARTED\n");


    if(setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(int)) == -1)
        fatal("SETTING OPTION SO_REUSEADDR FAILED\n");
        // ALLOW PROGRAM TO TAKE CONTROL OF PORT EVEN IF IT SEEMS IN USE

    struct sockaddr_in host_addr, client_addr;
    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(PORT);
    host_addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(host_addr.sin_zero), '\0', 8); // SET REST OF HOST_ADDR TO 0

    if(bind(server_socket, (struct sockaddr*) &host_addr, sizeof(host_addr)) == -1) {
        fatal("SERVER COULD NOT BIND TO HOST ADDRESS\n");
    }

    if(listen(server_socket, 5) == -1)
        fatal("SERVER COULD NOT COMMENCE LISTENING\n");

    addrlen = sizeof(host_addr);
    printf("STANDBY FOR CONNECTIONS..\n");

    // LISTEN LOOP //
    while(1) {

        FD_ZERO(&readfds);                          // CLEAR FILE DESCRIPTOR FOR CLIENT
        FD_SET(server_socket, &readfds);            // ADD MASTER SOCKET TO FD
        max_sd = server_socket;

        // CHECK FOR SERVER INPUT
        while ((command = fgetc(stdin)) != EOF) {
            printf("[%c]\n", command);
        }

        // ADD SLAVE SOCKETS TO FD
        for(i = 0; i < max_clients; i++)
        {
            sd = client_socket[i];

            if(sd > 0)                      // IF SOCKET DESCRIPTOR IS VALID, ADD TO FD
                FD_SET( sd, &readfds);

            if(sd > max_sd)                 // SET HIGHEST FD NUMBER TO MAX SD NUMBER
                max_sd = sd;
        }

        // int select(int nfds, fd_set *readfds, fd_set *writefds,fd_set *exceptfds, struct timeval *timeout);
        // WAIT FOR ACTIVITY, TIMEOUT IS NULL
        activity = select( max_sd + 1, &readfds, NULL, NULL, NULL);

        // EINTR = INTERRUPTED FUNCTION  [ <errno.h> decleration ]
        if((activity < 0) && (errno!=EINTR)) {
            fatal("SELECT FUNCTION ERROR DURING MULTIPLEXING\n");
        }

        // IF IT'S ACTIVITY ON THE SERVER SOCKET - INCOMING CONNECTION
        if(FD_ISSET(server_socket, &readfds)){
            if((new_socket = accept(server_socket, (struct sockaddr *)&client_addr, (socklen_t*)&addrlen)) == -1)
                fatal("SERVER COULD NOT ACCEPT NEW SOCKET FROM SERVER SOCKET\n");

            printf("[->] CONNECTION FROM %s:%d | FILE DESCRIPTOR IS %d\n",
                inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), new_socket);

                con_clients++;

                char *welcome;
                asprintf(&welcome, "%c]0;%s%d%c", '\033', "Clients connected: ", con_clients , '\007');

                send(new_socket, welcome, strlen(welcome), 0);

            if(send(new_socket, message, strlen(message), 0) != strlen(message))
                fatal("SERVER COULD NOT SEND GREETING MESSAGE TO NEW CLIENT\n");


            for(i = 0; i < max_clients; i++) {
                if(client_socket[i] == 0)       // IF CLIENT SOCKET I IS CLEAR
                {
                    client_socket[i] = new_socket;
                    printf("[--] ADDING TO LIST OF SOCKETS AS %d\n", i);
                    break;
                }
            }
        }

        // IF IT'S ACTIVITY ON A CLIENT SOCKET - IO OPERATION
        for (i = 0; i < max_clients; i++) {
            sd = client_socket[i];

            if(FD_ISSET(sd, &readfds))
            {
                if((valread = read(sd, buffer, 1024)) == 0) // IF SOCKET REPLIES ZERO
                {
                    getpeername(sd, (struct sockaddr*)&client_addr, (socklen_t*)&addrlen);
                    printf("[!!] CLIENT DISCONNECTED %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

                    con_clients--;

                    // TERMINATE THE SOCKET AND FD
                    close(sd);
                    client_socket[i] = 0;
                }
                else // ECHO BACK MESSAGE
                {
                    buffer[valread] = '\0';
                    printf("[--] Message from %s:%d > %s", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), buffer);
                    send(sd, buffer, strlen(buffer), 0);
                }
            }
        }
    }
    return 0;
}
