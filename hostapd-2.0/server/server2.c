#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define HOST_PORT 8080
#define DEBUG 1
#define BUF_SIZE 1024

#define DIE(msg)						\
    do{								\
        fprintf (stderr, "Error: %s:%d: %s\n",			\
                __FILE__, __LINE__, msg);				\
        exit (0);							\
    }while(0)							\


#define u8 unsigned char
//54:E4:3A:3D:63:C7
u8 macaddr[]={0124,0344,072,075,0143,0307};

int main (int argc, char **argv)
{
    int sockfd, client_sockfd;
    int host_port;
    int yes;
    char buffer[BUF_SIZE];
    struct sockaddr_in host_addr, client_addr;

    host_port = HOST_PORT;
    if (argc>1)
        host_port = atoi(argv[1]);

    if ((sockfd = socket (AF_INET, SOCK_STREAM, 0))==-1){
        DIE("creating a socket");
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
        DIE("setting socket option SO_REUSEADDR");

    memset(&(host_addr), '\0', sizeof(host_addr)); // zero off the structure
    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(host_port);
    host_addr.sin_addr.s_addr = 0;

    if (bind(sockfd, (struct sockaddr *)&host_addr
                , sizeof(struct sockaddr)) == -1)
        DIE("binding to socket");

    if (listen(sockfd, 1000) == -1)
        DIE("listening on socket");




    unsigned char recvmsg[BUF_SIZE];
    char sendmsg[BUF_SIZE]; 
    while (1){
        printf("begin \n");
        int size,i;
        if ((client_sockfd = accept 
                    (sockfd, (struct sockaddr *)&client_addr, &size))==-1)
            DIE("accepting client connection");

        printf("Parent process client_sockfd = %d\n",client_sockfd);	

        //if (DEBUG)
        //    printf ("server: accepting a client from %s port %d\n"
        //            , inet_ntoa (client_addr.sin_addr)
        //            , ntohs (client_addr.sin_port));

        while(1){
            printf("begin2\n");
            memset(recvmsg, 0, sizeof(recvmsg));
            memset(sendmsg, 0, sizeof(sendmsg));
            if (read(client_sockfd, recvmsg, sizeof(recvmsg)) < 0) {
                perror("recv err");
                break;
            }
            //printf("client: %s\n", recvmsg);
            int flag=1;
            for(i=0;i<6;i++){
                if(recvmsg[i]!=macaddr[i])flag=0;
                printf("0x%02x ",recvmsg[i]);
            }
            if(flag){
                strcpy(sendmsg, "Yes");
                write(client_sockfd, sendmsg, strlen(sendmsg) + 1);
            }else{
                strcpy(sendmsg, "No");
                write(client_sockfd, sendmsg, strlen(sendmsg) + 1);
            }
        }
        close(client_sockfd);
    }
    return 0;
}

