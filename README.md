## Server
The server is written with the library **<sys/socket.h>** therefore is not compatible with Windows OS.
It uses file descriptors to distinguish each client with the **<sys/time.h>** library.

#### Current features
- [x] Managing multiple clients
- [x] Echoing sent messages from clients
- [x] Awareness of client disconnects
- [x] Sending commands to specific clients
- [x] Master account authentication
- [x] Retrieving a list of clients connected

Features for the server are not final.


## Client
The client tailored for the server specifically has not yet been developed however you can use a **raw** socket or **telnet** to communicate with the server.
#### Current features
- [x] Retrieving basic information about client
- [x] Execution of commands specified by server
- [ ] Compatability for Windows OS with C#

Features for the client not final.

## Configuration

Enter your server ip into the client.c file.
line:35  `serv_addr.sin_addr.s_addr = inet_addr("<server_ip_here>");`

Enter your desired login password into cnc.c.
line:23  `char password[] = "<password_here>\r\n";`

Connect to your server via PuTTY and login with the before mentioned password.

### Commands: 
 - list                          
[lists connected clients]
 - send <id_of_socket>:<command> 
 [send a command to a client]
 - help                          
 [shows available commands]

## Screenshots
![Alt text](/demo.png?raw=true)
