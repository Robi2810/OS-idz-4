#include <stdio.h>     
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>   
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define MAXPENDING 8

double sumFromClients = 0;
int external_socket = -1;
struct sockaddr_in external_addr;

typedef struct threadArgs {
    int socket;
} threadArgs;

void DieWithError(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}


void *externalThread(void* args) {
    int server_socket;
    unsigned int client_length;
    pthread_detach(pthread_self());
    server_socket = ((threadArgs*)args)->socket;
    free(args);
    client_length = sizeof(external_addr);
    int recvData[1];
    recvfrom(server_socket, recvData, sizeof(recvData), 0, (struct sockaddr *) &external_addr, &client_length);
    printf("New connection from %s\n", inet_ntoa(external_addr.sin_addr));
    external_socket = server_socket;
}

int main(int argc, char *argv[])
{
    unsigned short port;
    unsigned short external_port;
    int server_socket;
    int client_socket;
    unsigned int client_length;
    int external_server_socket;
    struct sockaddr_in client_addr;
    struct sockaddr_in ext_addr;
    struct sockaddr_in server_addr;
    pthread_t threadId;
    
    if (argc != 4)
    {
        fprintf(stderr, "Usage:  %s <Port for clients> <Port for external programm> <Total sum>\n", argv[0]);
        exit(1);
    }

    port = atoi(argv[1]);
    external_port = atoi(argv[2]);
    double totalSum = atof(argv[3]);

    if ((server_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) DieWithError("socket() failed");
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;              
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) DieWithError("bind() failed");
    printf("Open socket on %s:%d\n", inet_ntoa(server_addr.sin_addr), port);

    if ((external_server_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) DieWithError("socket() failed");
    memset(&ext_addr, 0, sizeof(ext_addr));
    ext_addr.sin_family = AF_INET;              
    ext_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    ext_addr.sin_port = htons(external_port);

    if (bind(external_server_socket, (struct sockaddr *) &ext_addr, sizeof(ext_addr)) < 0) DieWithError("bind() failed");
    printf("Open socket on %s:%d\n", inet_ntoa(ext_addr.sin_addr), external_port);

    threadArgs *args = (threadArgs*) malloc(sizeof(threadArgs));
    args->socket = external_server_socket;

    if (pthread_create(&threadId, NULL, externalThread, (void*) args) != 0) DieWithError("pthread_create() failed");
    
    for (int i = 0; i < 8; i++) {
        client_length = sizeof(client_addr);
        double childPart;
        double recvData[1];

        recvfrom(server_socket, recvData, sizeof(recvData), 0, (struct sockaddr*) &client_addr, &client_length);
        childPart = recvData[0];
        sumFromClients += childPart;
        printf("New connection from %s\n", inet_ntoa(client_addr.sin_addr));
        if (external_socket > 0) sendto(external_socket, recvData, sizeof(recvData), 0, (struct sockaddr *) &external_addr, sizeof(external_addr));
    }
    printf("Sum from children: %f\n", sumFromClients);
    printf("Total sum: %f\n", totalSum);
    if (totalSum != sumFromClients) {
        printf("Attorney is liar\n");
    } else {
        printf("Everything is OK\n");
    }
    double sendData[1];
    sendData[0] = -1;
    if (external_socket > 0) {
        sendto(external_socket, sendData, sizeof(sendData), 0, (struct sockaddr *) &external_addr, sizeof(external_addr));
        close(external_socket);
    }
    return 0;
}
