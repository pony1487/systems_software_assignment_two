#include <stdio.h>
#include <stdlib.h>
#include <string.h>    
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <unistd.h>    
 
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
        char filename[50], path[150];
        char file_buffer[512]; 
	    bzero(file_buffer, 512); 


        strcpy(filename,  argv[1]);
        strcpy(path, argv[2]);

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
            //testing opening a file
            // while((file_read_char = getc(fp)) != EOF) 
            // {
            //     putchar(file_read_char);
            // }
        
            // fclose(fp);
            
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

            memset(serverWelcomeMessage, 0, 500);
            //Receive a welcome message from the server
            if( recv(SID , serverWelcomeMessage , 500 , 0) < 0)
            {
                printf("IO error");
            }
            
            // display welcome message
            printf("\nServer sent: %s\n",serverWelcomeMessage);
            
            //keep communicating with server
            while(1)
            {                 
                printf("\nEnter message : ");
                scanf("%s" , clientMessage);
                
                //Send some data
                if( send(SID , clientMessage , strlen(clientMessage) , 0) < 0)
                {
                    printf("Send failed");
                    return 1;
                }

                //Receive a reply from the server
                if( recv(SID , serverMessage , 500 , 0) < 0)
                {
                    printf("IO error");
                    //break;
                }
                
                puts("\nServer sent: ");
                puts(serverMessage);

                //clear previous message
                memset(serverMessage, 0, 500);

            }
            
            close(SID);
            return 0;
        }//end file open
    }//end argv check
}
