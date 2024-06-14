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

void send_request(int s) {
    
}

int main(int argc, char** argv) {
    char server_host[] = "";
    int port = 0;

    int s = socket(AF_INET, SOCK_STREAM, 0);    //tcp
    //int s = socket(AF_INET, SOCK_DGRAM, 0);    //udp
    if (s < 0) {
        error("ERROR opening socket");
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    struct hostent *serv = gethostbyname(server_host);
    bcopy((char*)serv->h_addr_list[0], (char*)&server_addr.sin_addr.s_addr, serv->h_length);
    server_addr.sin_port = htons(port);

    if (connect(s, (const struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        error("ERROR on connect");
    }

    send_request(s);
    close(s);

    return 0;
}