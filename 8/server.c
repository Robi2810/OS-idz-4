#include <stdio.h>     
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>   
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define MAXPENDING 8

static int isSendData = 1;

double sumFromClients = 0;
double calculatedSum = 0;

typedef struct threadArgs {
    int socket;
} threadArgs;

void DieWithError(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}

int main(int argc, char *argv[])
{
    unsigned short port;
    int server_socket;
    int client_socket;
    int client_length;
    struct sockaddr_in client_addr;
    struct sockaddr_in server_addr;

    int multicast_sock;
    struct sockaddr_in multicastAddr;
    char *multicastIP;
    unsigned short multicastPort;
    
    
    if (argc != 5)
    {
        fprintf(stderr, "Incorrect input:  %s <Multicast Address> <Port for clients> <Port for Multicast> <Total sum>\n", argv[0]);
        exit(1);
    }

    multicastIP = argv[1];
    port = atoi(argv[2]);
    multicastPort = atoi(argv[3]);
    double totalSum = atof(argv[4]);

    if ((server_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) DieWithError("socket() failed");
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;              
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) DieWithError("bind() failed");
    printf("Open %s:%d\n", inet_ntoa(server_addr.sin_addr), port);

    

    if ((multicast_sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

   
    int multicastTTL = 1;
    if (setsockopt(multicast_sock, IPPROTO_IP, IP_MULTICAST_TTL, (void *) &multicastTTL, 
          sizeof(multicastTTL)) < 0)
        DieWithError("setsockopt() failed");

    memset(&multicastAddr, 0, sizeof(multicastAddr));   
    multicastAddr.sin_family = AF_INET;                 
    multicastAddr.sin_addr.s_addr = inet_addr(multicastIP);
    multicastAddr.sin_port = htons(multicastPort); 


    printf("Open %s:%d\n", inet_ntoa(multicastAddr.sin_addr), multicastPort);

    for (int i = 0; i < 8; i++) {
        client_length = sizeof(client_addr);
        double childPart;
        double recvData[1];

        recvfrom(server_socket, recvData, sizeof(recvData), 0, (struct sockaddr*) &client_addr, &client_length);
        childPart = recvData[0];
        sumFromClients += childPart;
        printf("Connect %s\n", inet_ntoa(client_addr.sin_addr));
        sendto(multicast_sock, recvData, sizeof(recvData), 0, (struct sockaddr *) &multicastAddr, sizeof(multicastAddr));
    }
    printf("Sum: %f\n", sumFromClients);
    printf("Total sum: %f\n", totalSum);
    if (totalSum != sumFromClients) {
        printf("INCORRECT Lawyer - SKAMer\n");
    } else {
        printf("CORRECT\n");
    }
    double sendData[1];
    sendData[0] = -1;
    sendto(multicast_sock, sendData, sizeof(sendData), 0, (struct sockaddr *) &multicastAddr, sizeof(multicastAddr));
    close(multicast_sock);
    close(server_socket);
    return 0;
}
