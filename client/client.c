#include <stdio.h>
#include <stdlib.h>
#include <string.h>    
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <unistd.h>  
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

#define FILE_BUFFER_SIZE 512
#define UID_SIZE 10
#define GID_SIZE 10
#define UEID_SIZE 10
#define GEID_SIZE 10
#define FILENAME_SIZE 50
#define GROUPNAME_SIZE 50



typedef struct message{
    char uid[UID_SIZE];
    char gid[GID_SIZE];
    char ueid[UEID_SIZE];
    char geid[GEID_SIZE];
    char groupname[GROUPNAME_SIZE];
    char filename[FILENAME_SIZE];

}client_message;

void commandLoop();
char *getUserInput(void);
char *int_to_string(int id);
char *get_group_name_from_gid(int); 
 
int main(int argc , char *argv[])
{
    if(argc != 3)
    {
        printf("Exiting!!! Usage: filename pathname\n");
        exit(EXIT_FAILURE);   
    }
    else
    {
        int SID;
        struct sockaddr_in server;
        char clientMessage[500];
        char serverMessage[500];

        client_message msg;
        
        char filename[50], path[150];
        char file_buffer[512]; 
	    bzero(file_buffer, 512); 
        int block_size, counter = 0;

        //get users info
        uid_t uid = getuid();
        gid_t gid = getgid();
        uid_t ueid = geteuid();
        uid_t geid = getegid();

        //convert int to string so it can be sent to server
        char* uid_as_str = int_to_string(uid);
        char* gid_as_str = int_to_string(gid);
        char* ueid_as_str = int_to_string(ueid);
        char* geid_as_str = int_to_string(geid);
        
        printf("This process is associated with UID: %d and GID: %d\n",uid,gid);
        printf("This process is associated with UEID: %d and GEID: %d\n",ueid,geid);

        //get users group name
        struct group *grp;
        struct passwd *pwd;

        gid_t supp_groups[] = {};

        int ngroups = 10;
        gid_t *groups;
        groups = malloc(ngroups * sizeof(gid_t));

        grp = getgrgid(gid);
        pwd = getpwuid(uid);

        printf("-----------------------------------\n");            

        if(getgrouplist(pwd->pw_name,uid,groups,&ngroups) == -1)
        {
            perror("Could not get groups");
        }

        printf("Group List:\n");
        for(int i = 0; i < ngroups; i++)
        {
            supp_groups[i] = groups[i];
            printf("%d\n",supp_groups[i]);
            printf("%s\n",get_group_name_from_gid(supp_groups[i]));
        }

        printf("-----------------------------------\n");

        //get the file name and path from cmd line args
        strcpy(filename,  argv[1]);
        strcpy(path, argv[2]);

        //fill message struct
        strcpy(msg.filename,argv[1]);
        strcpy(msg.uid,uid_as_str);
        strcpy(msg.gid,gid_as_str);
        strcpy(msg.ueid,ueid_as_str);
        strcpy(msg.geid,geid_as_str);
        //ignore the 1st group which is the users name
        strcpy(msg.groupname,get_group_name_from_gid(supp_groups[1]));

        printf("msg.groupname: %s\n",msg.groupname);

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

                if(ret == 0) 
                {
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
                else
                {
                    printf("Server init not successful\n");
                } 
            }

            //Clean up
            fclose(fp);
            close(SID);
            
            printf("\nProgram exiting...\n");
            return 0;
        }//end file open
    }//end argv check
}

//not used
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

char *get_group_name_from_gid(int gid)
{
    struct group *grp;
    grp = getgrgid(gid);
    return grp->gr_name;
}


