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

void send_error(int cli_socket) {
    write(cli_socket, "ERROR", strlen("ERROR"));
}

void handle_request(int cli_socket, struct sockaddr_in client_addr) {
    char buff[255] = { 0 };
    int msg_len = read(cli_socket, &buff, 255);
    if (msg_len < 0) {
        error("ERROR on read");
    }

    printf("%s\n", buff);
    int ip[4] = {0};
    int j = 0;
    for (int i = 0; i < msg_len; i++) {
        char c = buff[i];
        if (c == '.') {
            if (ip[j] > 255) {
                send_error(cli_socket);
                return;
            }
            j++;
            continue;
        }

        if (c < '0' || c > '9') {
            send_error(cli_socket);
            return;
        }

        ip[j] *= 10;
        ip[j] += c - '0';
    }

    if (j != 3) {
        send_error(cli_socket);
        return;
    }

    if (ip[0] == 0 && ip[1] == 0) {
        send_error(cli_socket);
        return;
    }

    char ip_class;
    if (ip[0] < 128) ip_class = 'A';
    else if (ip[0] < 192) ip_class = 'B';
    else if (ip[0] < 224) ip_class = 'C';
    else if (ip[0] < 240) ip_class = 'D';
    else ip_class = 'E';

    memset(buff, 0, msg_len);
    buff[0] = ip_class;
    buff[1] = ' ';

    switch (ip_class) {
    case 'A':
        sprintf(buff + 2, "%d.0.0.0 %d.255.255.255", ip[0], ip[0]);
        break;
    case 'B':
        sprintf(buff + 2, "%d.%d.0.0 %d.%d.255.255", ip[0], ip[1], ip[0], ip[1]);
        break;
    case 'C':
        sprintf(buff + 2, "%d.%d.%d.0 %d.%d.%d.255", ip[0], ip[1], ip[2], ip[0], ip[1], ip[2]);
        break;
    default:
    }

    msg_len = strlen(buff);

    if (write(cli_socket, buff, msg_len) < 0) {
        error("ERROR on write");
    }
}

int main() {
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        error("ERROR opening socket");
    }
    
    struct sockaddr_in server_addr, client_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(1025);

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
        handle_request(cli_socket, client_addr);
    }
}