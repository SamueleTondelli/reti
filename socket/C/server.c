#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void error(char* str) {
    perror(str);
    exit(1);
}

void handle_request(int cli_socket, struct sockaddr_in client_addr) {

}

int main() {
    int port = 0;

    int s = socket(AF_INET, SOCK_STREAM, 0);    //tcp
    //int s = socket(AF_INET, SOCK_DGRAM, 0);    //udp
    if (s < 0) {
        error("ERROR while opening socket");
    }
    
    struct sockaddr_in server_addr, client_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(s, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        error("ERROR on binding");
    }

    listen(s, 5);

    while (1) {
        int cli_len = sizeof(client_addr);
        int cli_socket = accept(s, (struct sockaddr*)&client_addr, &cli_len);
        if (cli_socket < 0) {
            error("ERROR on accept");
        }
        handle_request(cli_socket, client_addr);    //single process

        /*
        //multi process
        int pid = fork();
        if (pid < 0) {
            error("ERROR on fork");
        }

        if (pid == 0) {
            handle_request(cli_socket, client_addr);
            exit(0);
        }
        */
    }
}