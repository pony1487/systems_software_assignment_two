#include <stdio.h>      // for IO
#include <string.h>     //for strlen
#include <sys/socket.h> // for socket
#include <arpa/inet.h>  //for inet_addr
#include <unistd.h>     //for write
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

#define MAX_NUM_CLIENTS 25
#define RECV_FILE_BUFF_SIZE 512
#define UID_SIZE 10
#define GID_SIZE 10
#define UEID_SIZE 10
#define GEID_SIZE 10
#define FILENAME_SIZE 50
#define GROUPNAME_SIZE 50
#define DEST_FOLDER_NAME_SIZE 50


pthread_mutex_t lock_x;

typedef struct message{
    char uid[UID_SIZE];
    char gid[GID_SIZE];
    char ueid[UEID_SIZE];
    char geid[GEID_SIZE];
    char groupname[GROUPNAME_SIZE];
    char filename[FILENAME_SIZE]; 
    char dest_folder_name[DEST_FOLDER_NAME_SIZE];
}client_message;


void *client_handler(void *socket);
void sighandler(int);
void check_client_permissions(char*,char*);
char *get_username_from_uid(gid_t);
void add_slash_to_string(char*, char);
char *create_file_path(char*,char*,char*);
 
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

    for(int i = 0; i < client_count;i++)
    {
        pthread_join(threads[i],NULL);
    }
              
    return 0;
}

void *client_handler(void *socket)
{
    //clients ids
    uid_t client_uid;
    gid_t client_gid;
    uid_t client_ueid;
    uid_t client_geid;

    gid_t supp_groups[] = {};

    int ngroups = 10;
    gid_t *groups;
    groups = malloc(ngroups * sizeof(gid_t));

    //get servers id's
    uid_t server_uid = getuid();
    gid_t server_gid = getgid();
    uid_t server_ueid = geteuid();
    uid_t server_geid = getegid();

    printf("-----------------------------------\n");
    printf("This server thread is associated with UID: %d and GID: %d\n",server_uid,server_gid);
    printf("This server thread is associated with UEID: %d and GEID: %d\n",server_ueid,server_geid);
    printf("-----------------------------------\n");


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
        char client_group[GROUPNAME_SIZE];
        char dest_folder[DEST_FOLDER_NAME_SIZE];

        if( (size = recv ( sock, (void*)&received_message, sizeof(client_message), 0)) >= 0)
        {
            printf("msg.filename: %s\n",received_message.filename);
            printf("clients uid: %s\n",received_message.uid);
            printf("clients gid: %s\n",received_message.gid);
            printf("clients ueid: %s\n",received_message.ueid);
            printf("clients geid: %s\n",received_message.geid);

            //store clients ids and groupname
            client_uid = atoi(received_message.uid);
            client_gid = atoi(received_message.gid);
            client_ueid = atoi(received_message.ueid);
            client_geid = atoi(received_message.ueid);
            strcpy(client_group,received_message.groupname);
            strcpy(dest_folder,received_message.dest_folder_name);

            const char *username = get_username_from_uid(client_uid);

            printf("-----------------------------------\n");            
            printf("Client Username: %s\n",username);
            printf("Client Group: %s\n",client_group);
            printf("Client sending to: %s\n",dest_folder);

            if(getgrouplist(username,client_uid,groups,&ngroups) == -1)
            {
                perror("Could not get groups");
            }

            printf("Group List:\n");
            for(int i = 0; i < ngroups; i++)
            {
                supp_groups[i] = groups[i];
                printf("%d\n",supp_groups[i]);
            }

            printf("-----------------------------------\n");


            //change to the clients id and try to execute the transfer
            setgroups(ngroups,supp_groups);

            if(setreuid(client_uid,server_uid) < 0 )
            {
                perror("could not change ids: setreuid()");
                exit(EXIT_FAILURE);  
            }

            if(setregid(client_uid,server_uid) < 0)
            {
                perror("could not change ids: setregid()");
                exit(EXIT_FAILURE);  
            }

            if(seteuid(client_uid) < 0)
            {  
                perror("could not change ids: seteuid()");
                exit(EXIT_FAILURE);  
            }
            if(setegid(client_uid) < 0)
            {
                perror("could not change ids: setegid()");
                exit(EXIT_FAILURE);  
            }


            printf("User id: %d\n",getuid());
            printf("Group id: %d\n",getgid());
            printf("Effective User id: %d\n",geteuid());
            printf("Effective Group id: %d\n",getegid());

            char file_buffer[RECV_FILE_BUFF_SIZE];
            //create file path
            char intranet_path[] = "/home/ronan/Documents/College/SystemsSoftware/AssignmentTwo/server/intranet/";
            char *file_path = create_file_path(intranet_path,dest_folder,received_message.filename);
            printf("%s\n",file_path);

            //lock file while thread is using it
            pthread_mutex_lock(&lock_x);

            FILE *file_open = fopen(file_path, "w");

            if(file_open == NULL)
            {
                perror("File Cannot be opened file on server.");
                write(sock , "failure" , strlen("failure"));
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
                free(file_path);
                printf("-----------------------------------\n");

                pthread_mutex_unlock(&lock_x);

            }

            //switch back to root
            if(seteuid(server_uid) < 0)
            {  
                perror("could not change back to root: seteuid()");
                exit(EXIT_FAILURE);  
            }
            if(setegid(server_uid) < 0)
            {
                perror("could not change back to root: setegid()");
                exit(EXIT_FAILURE);  
            }
            if(setregid(server_uid,server_uid) < 0)
            {
                perror("could not change back to root: setregid()");
                exit(EXIT_FAILURE);  
            }
            if(setreuid(server_uid,server_uid) < 0 )
            {
                perror("could not change back to root: setreuid()");
                exit(EXIT_FAILURE);  
            }

            printf("Back to root\n");
            printf("User id: %d\n",getuid());
            printf("Group id: %d\n",getgid());
            printf("Effective User id: %d\n",geteuid());
            printf("Effective Group id: %d\n",getegid());
            
            //write(sock , "success" , strlen("success"));
            break;

        }
        else if(size == -1)
        {
            perror("read error");
        }


    }//end while
    pthread_exit(NULL);
}

char *get_username_from_uid(gid_t uid)
{
    struct passwd *pw;
    pw = getpwuid(uid);
    if(pw != NULL)
    {
        return pw->pw_name;
    }
    else
    {
        return "no user name";
    } 
}

void check_client_permissions(char *uid,char *gid)
{

}

void sighandler(int signum) 
{
   printf("Caught signal %d, closing...\n", signum);
   exit(1);
}

void add_slash_to_string(char* str, char c)
{
        int len = strlen(str);
        str[len] = c;
        str[len+1] = '\0';
}

char *create_file_path(char* dir_path,char* group_name,char* file_name)
{
        add_slash_to_string(group_name,'/');

        char *full_file_path = malloc((strlen(dir_path) +strlen(group_name) + strlen(file_name) + 1)*sizeof(char));

        strcat(dir_path,group_name);
        strcat(dir_path,file_name);
        strcpy(full_file_path,dir_path);

        return full_file_path;
}