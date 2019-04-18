#include <stdio.h>      // for IO
#include <string.h>     //for strlen
#include <sys/socket.h> // for socket
#include <arpa/inet.h>  //for inet_addr
#include <unistd.h>     //for write
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>

#define MAX_NUM_CLIENTS 25
#define RECV_FILE_BUFF_SIZE 512
#define UID_SIZE 10
#define GID_SIZE 10
#define FILENAME_SIZE 50



typedef struct message{
    char uid[UID_SIZE];
    char gid[GID_SIZE];
    char filename[FILENAME_SIZE]; 
}client_message;


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
    int str_cmp_ret;

    printf("Connection from client accepted: Socket: %d\n",sock);
    printf("client_handler called\n");

    while(1) {
        //recieve client struct
        client_message received_message;
        int size;

        if( (size = recv ( sock, (void*)&received_message, sizeof(client_message), 0)) >= 0)
        {
            printf("msg.filename: %s\n",received_message.filename);
            printf("msg.uid: %s\n",received_message.uid);
            printf("msg.gid: %s\n",received_message.gid);

            write(sock , "success" , strlen("success"));

            char file_buffer[RECV_FILE_BUFF_SIZE];
            char *file_name = "/home/ronan/Documents/College/SystemsSoftware/AssignmentTwo/server/intranet/server_test.txt";
            FILE *file_open = fopen(file_name, "w");

            if(file_open == NULL)
            {
                printf("File %s Cannot be opened file on server.\n", file_name);
            }
            else 
            {
                bzero(file_buffer, RECV_FILE_BUFF_SIZE); 
                int block_size = 0;
                int counter = 0;
                while((block_size = recv(sock, file_buffer, RECV_FILE_BUFF_SIZE, 0)) > 0) {
                    printf("Data Received %d = %d\n",counter,block_size);
                    int write_sz = fwrite(file_buffer, sizeof(char), block_size, file_open);
                    bzero(file_buffer, RECV_FILE_BUFF_SIZE);
                    counter++;
                }

                fclose(file_open);
            }

            break;
        }
        else if(size == -1)
        {
            perror("read error");
        }


    }
 
}


// void *client_handler(void *socket)
// {
//     //cast the socket back to int
//     int sock = *(int*)socket;

//     char file_buffer[RECV_FILE_BUFF_SIZE];
//     char *file_name = "/home/ronan/Documents/College/SystemsSoftware/AssignmentTwo/server/intranet/server_test.txt";
//     FILE *file_open = fopen(file_name, "w");

//     if(file_open == NULL)
//     {
//         printf("File %s Cannot be opened file on server.\n", file_name);
//     }
//     else 
//     {
//         bzero(file_buffer, RECV_FILE_BUFF_SIZE); 
//         int block_size = 0;
//         int counter = 0;
//         while((block_size = recv(sock, file_buffer, RECV_FILE_BUFF_SIZE, 0)) > 0) {
//             printf("Data Received %d = %d\n",counter,block_size);
//             int write_sz = fwrite(file_buffer, sizeof(char), block_size, file_open);
//             bzero(file_buffer, RECV_FILE_BUFF_SIZE);
//             counter++;
//         }

//         fclose(file_open);
//     }
// }

void sighandler(int signum) 
{
   printf("Caught signal %d, closing...\n", signum);
   exit(1);
}
