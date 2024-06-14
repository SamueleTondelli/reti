#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void error(char* str) {
    perror(str);
    exit(1);
}

void send_request(int s, char* ip) {
    if (write(s, ip, strlen(ip)) < 0) {
        perror("ERROR on write");
        return;
    }

    char buff[255] = {0};
    if (read(s, buff, 255) < 0) {
        perror("ERROR on read");
        return;
    }

    printf("%s\n", buff);
}

int main(int argc, char** argv) {
    if (argc != 3) {
        printf("ERROR\n");
        return 1;
    }

    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        error("ERROR opening socket");
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    struct hostent *serv = gethostbyname(argv[1]);
    bcopy((char*)serv->h_addr_list[0], (char*)&server_addr.sin_addr.s_addr, serv->h_length);
    server_addr.sin_port = htons(1025);

    if (connect(s, (const struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        error("ERROR on connect");
    }

    send_request(s, argv[2]);
    close(s);
    return 0;
}