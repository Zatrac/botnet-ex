## Server
The server is written with the library **<sys/socket.h>** therefore is not compatible with Windows OS.
It uses file descriptors to distinguish each client with the **<sys/time.h>** library.

#### Current features
- [x] Managing multiple clients
- [x] Echoing sent messages from clients
- [x] Awareness of client disconnects
- [ ] Sending commands to specific clients
- [x] Master account authentication
- [x] Retrieving a list of clients connected

Features for the server are not final.


## Client
The client tailored for the server specifically has not yet been developed however you can use a **raw** socket or **telnet** to communicate with the server.
#### Current features
- [ ] Retrieving basic information about client
- [ ] Execution of commands specified by server
- [ ] Compatability for Windows OS with C#

Features for the client are simply in the idea phase currently and are absolutely not final.
