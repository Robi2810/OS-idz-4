#include <stdio.h>   
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void DieWithError(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}

int main(int argc, char *argv[])
{
    int client_socket;
    struct sockaddr_in server_addr;
    unsigned short server_port;
    unsigned int addr_len;
    char *server_ip;

    if (argc != 3)
    {
       fprintf(stderr, "Usage: %s <Server IP> <Server Port>\n", argv[0]);
       exit(1);
    }

    server_ip = argv[1];
    server_port = atoi(argv[2]);

    if ((client_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) DieWithError("socket() failed");
    addr_len = sizeof(server_addr);

    memset(&server_addr, 0, sizeof(server_addr));  
    server_addr.sin_family      = AF_INET;        
    server_addr.sin_addr.s_addr = inet_addr(server_ip); 
    server_addr.sin_port        = htons(server_port);

    double recvData[1];
    sendto(client_socket, recvData, sizeof(recvData), 0, (struct sockaddr *) &server_addr, sizeof(server_addr));
    for (;;) {
        recvfrom(client_socket, recvData, sizeof(recvData), 0, (struct sockaddr *) &server_addr, &addr_len);
        if (recvData[0] < 0) break;
        printf("Server calculated part: %f\n", recvData[0]);
    }
    printf("Server finished\n");
    close(client_socket);
    return 0;
}
