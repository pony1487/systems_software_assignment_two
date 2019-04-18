#include <stdio.h>
#include <stdlib.h>
#include <string.h>    
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <unistd.h>  
#include <sys/types.h>

#define FILE_BUFFER_SIZE 512
#define UID_SIZE 10
#define GID_SIZE 10
#define FILENAME_SIZE 50


typedef struct message{
    char uid[UID_SIZE];
    char gid[GID_SIZE];
    char filename[FILENAME_SIZE]; 
}client_message;

void commandLoop();
char *getUserInput(void);
char * int_to_string(int id);
 
int main(int argc , char *argv[])
{
    int SID;
    struct sockaddr_in server;
    char clientMessage[500];
    char serverWelcomeMessage[500];
    char serverMessage[500];

    char file_read_char;

    if(argc != 3)
    {
        printf("Exiting!!! Usage: filename pathname\n");
        exit(EXIT_FAILURE);   
    }
    else
    {
        client_message msg;
        
        char filename[50], path[150];
        char file_buffer[512]; 
	    bzero(file_buffer, 512); 
        int block_size, counter =0;

        //get users info
        uid_t uid = geteuid();
        gid_t gid = getegid();

        //
        char* uid_as_str = int_to_string(uid);
        char* gid_as_str = int_to_string(gid);
        
        printf("This process is associated with UID: %d and GID: %d\n",uid,gid);

        //get the file name and path from cmd line args
        strcpy(filename,  argv[1]);
        strcpy(path, argv[2]);

        //fill message struct
        strcpy(msg.filename,argv[1]);
        strcpy(msg.uid,uid_as_str);
        strcpy(msg.gid,gid_as_str);

        printf("msg.filename: %s\n",msg.filename);
        printf("msg.uid: %s\n",msg.uid);
        printf("msg.gid: %s\n",msg.gid);

        //create the path to filename provided
        strcat(path, filename);
    
        printf("path: %s\n",path);

        FILE *fp = fopen(path, "r");

        if(fp == NULL)
        {
            perror("Error while opening the file.\n");
            exit(EXIT_FAILURE);
        }
        else
        {
            //Create socket
            SID = socket(AF_INET , SOCK_STREAM , 0);
            if (SID == -1)
            {
                printf("Error creating socket");
            } {
                printf("socket created");
            } 
            
            // set sockaddr_in variables
            server.sin_port = htons( 8082 ); // Port to connect on
            server.sin_addr.s_addr = inet_addr("127.0.0.1"); // Server IP
            server.sin_family = AF_INET; // IPV4 protocol
            
        
            //Connect to server
            if (connect(SID , (struct sockaddr *)&server , sizeof(server)) < 0)
            {
                printf("connect failed. Error");
                return 1;
            }
            
            printf("Connected to server ok!!\n");
            
            while(1)
            {    
                //init the transfer process. Send the clients information stored in the message structure     
                int nbytes = 0;
                if ((nbytes = write(SID, &msg, sizeof(msg)) != sizeof(msg)))
                {
                    perror("error writing msg struct");
                    exit(EXIT_FAILURE);
                }

                //Receive a reply from the server
                if( recv(SID , serverMessage , 500 , 0) < 0)
                {
                    printf("IO error");
                }

                int ret = strcmp(serverMessage, "success");

                if(ret == 0) {
                    printf("Server init: %s\n",serverMessage);
                    memset(serverMessage, 0, 500);

                    while((block_size = fread(file_buffer, sizeof(char), FILE_BUFFER_SIZE, fp)) > 0) 
                    {
                        printf("Data Sent %d = %d\n",counter, block_size);
                        if(send(SID, file_buffer, block_size, 0) < 0) 
                        {
                            exit(1);
                        }
                        bzero(file_buffer, FILE_BUFFER_SIZE);
                        counter++;
                    }

                    break;
                } 
            }

            
            // while((block_size = fread(file_buffer, sizeof(char), FILE_BUFFER_SIZE, fp)) > 0) 
            // {
            //     printf("Data Sent %d = %d\n",counter, block_size);
            //     if(send(SID, file_buffer, block_size, 0) < 0) 
            //     {
            //         exit(1);
            //     }
            //     bzero(file_buffer, FILE_BUFFER_SIZE);
            //     counter++;
            // }

            fclose(fp);
            close(SID);
            
            printf("\nProgram exiting...\n");
            return 0;
        }//end file open
    }//end argv check
}

void commandLoop()
{
    char *userInput;
    char **arguments;

    do{
        printf("\nShell>>");
        userInput = getUserInput();
        printf("%s\n",userInput);
    }while(1);
}
char *getUserInput(void)
{
    char *userInput = NULL;
    ssize_t b_size = 0;
    getline(&userInput,&b_size,stdin);
    return userInput;
}

char * int_to_string(int id)
{
    //get the number of chars in the id to set size for string
    int length = snprintf( NULL, 0, "%d", id);
    // + 1 for null
    char* int_as_str = malloc( length + 1 );
    snprintf(int_as_str, length + 1, "%d", id);
    
    return int_as_str;
}

