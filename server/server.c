#include <stdio.h>      // for IO
#include <string.h>     //for strlen
#include <sys/socket.h> // for socket
#include <arpa/inet.h>  //for inet_addr
#include <unistd.h>     //for write
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>

#define MAX_NUM_CLIENTS 25

void *client_handler(void *socket);
void sighandler(int);
 
int main(int argc , char *argv[])
{
    signal(SIGINT, sighandler);

    // Thread stuff
    pthread_t threads[MAX_NUM_CLIENTS];
    int client_count = 0;

    int s; // socket descriptor
    int cs; // Client Socket
    int connSize; // Size of struct 

    struct sockaddr_in server , client;
     
    printf("Server started....\n");

    //Create socket
    s = socket(AF_INET , SOCK_STREAM , 0);
    if (s == -1)
    {
        printf("Could not create socket\n");
    } else {
    	printf("Socket Successfully Created!!\n");
    } 

    // set sockaddr_in variables
    server.sin_port = htons( 8082 ); // Set the prot for communication
    server.sin_family = AF_INET; // Use IPV4 protocol
    server.sin_addr.s_addr = INADDR_ANY; 
         
    //Bind
    if( bind(s,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("Bind issue!!\n");
        return 1;
    } else {
    	printf("Bind Complete!!\n");
    }
     
    //Listen for a conection
    listen(s,3); 
    //Accept and incoming connection
    printf("Waiting for incoming connection from Client>>");
    connSize = sizeof(struct sockaddr_in);
     
    while(cs = accept(s, (struct sockaddr *)&client, (socklen_t*)&connSize))
    {
        char *welcome_message = "Welcome.\n";
        write(cs , welcome_message , strlen(welcome_message));

        if( pthread_create( &threads[client_count], NULL ,  client_handler , (void*) &cs) < 0)
        {
            perror("could not create thread");
            return 1;
        }
        else
        {
            client_count++;
            printf("\nNumber of clients: %d\n",client_count);
        }   
    }

    if (cs < 0)
    {
        perror("Can't establish connection");
        return 1;
    } 
              
    return 0;
}

void *client_handler(void *socket)
{
    //cast the socket back to int
    int sock = *(int*)socket;

    char message[500];
    int READSIZE;  
    printf("Connection from client accepted: Socket: %d\n",sock);
    printf("client_handler called\n");

    while(1) {
        memset(message, 0, 500);
        READSIZE = recv(sock , message , 500 , 0);
        write(sock , message , strlen(message));
    }
 
    if(READSIZE == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(READSIZE == -1)
    {
        perror("read error");
    }
}

void sighandler(int signum) 
{
   printf("Caught signal %d, closing...\n", signum);
   exit(1);
}
