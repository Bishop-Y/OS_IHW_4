#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define DEFAULT_PORT 8080
#define DEFAULT_IP "127.0.0.1"

int main(int argc, char const *argv[]) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    int inheritance_part = 0;
    int port = DEFAULT_PORT;
    char ip[15] = DEFAULT_IP;

    // Parsing input parameters
    if (argc == 2) {
        inheritance_part = atoi(argv[1]);
    }

    if (argc == 4) {
        inheritance_part = atoi(argv[1]);
        strcpy(ip, argv[2]);
        port = atoi(argv[3]);
    }

    // Creating socket
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert IP address from text to binary form
    if(inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    inheritance_part = htonl(inheritance_part);
    if (sendto(sock, &inheritance_part, sizeof(inheritance_part), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("sendto failed");
        exit(EXIT_FAILURE);
    }

    return 0;
}
