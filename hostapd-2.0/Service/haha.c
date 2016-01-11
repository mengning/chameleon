
#include "haha.h"
int bz;
#define MAXLINE 2048
#define PORT  8888
#define u8 unsigned char
u8 line[MAXLINE];
int sendingmsg(int sockfd,u8 *mac)
{
    char c[200];
    char *req ="POST / HTTP/1.1\r\nContent-Length: 42\r\n\r\nsubmit=checkmac&macvalue=";
    sprintf(c,"%s%02x-%02x-%02x-%02x-%02x-%02x",req,mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    printf("req:%s\n",c);
    if(write(sockfd,c,strlen(c))<0){
        printf("sendingmsg in haha error\n");
        return -1;
    }
    return 0;
}
int recevemsg(int sockfd,char *msg)
{
    char tmp[1024];
    int nbyte=0;
    if(nbyte=read(sockfd,&tmp,1001)<0)
    {
        printf("recvmesage in haha error\n");
        return -1;
    }
    printf("recevemsg in haha with function:%s\n",tmp);
    memset(msg,0,sizeof(msg));
    strncpy(msg,tmp,1001);
    return nbyte;
}
int closesocket(int sockfd)
{
    close(sockfd);
    return 0;
}
int client(int Port,char *address)
{
    int sock_client = socket(AF_INET,SOCK_STREAM, 0);//sock fd

    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(Port);  //server port

    addr.sin_addr.s_addr = inet_addr(address);  ///server ip address

    if (connect(sock_client, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("connect");
        exit(1);
    }

    printf("sock_client = %d\n",sock_client);

    //   char *req ="message from client\0";
    //    sendingmsg(sock_client,req);
    //write(sock_client,req,strlen(req)+1);

    //receive the response from web server
    //    char resp[1024];
    //    recevemsg(sock_client,resp);
    //    close(sock_client);

    return sock_client;
}


int fd[2];
int fd2[2];
static void init(){
    //memset(acceptque,0,sizeof(acceptque));
    //acceptsz=-1;
}
static int command(char *resp){
    int idl=1;
    char *ssidname;
    char *psw;
    int idr=0;
    while(resp[idr]!='\0'){
        if(resp[idr]==':')break;
        idr++;
    }
    ssidname=(char *)malloc(sizeof(char)*(idr-idl+2));
    strncpy(ssidname,resp+idl,idr-idl);

    //    printf("idl:%d   idr:%d\n  ssid:%s\n",idl,idr,ssidname);

    idl=++idr;
    while(resp[idr]!='\0'){
        if(resp[idr]=='#')break;
        idr++;
    }
    psw=(char *)malloc(sizeof(char)*(idr-idl+2));
    strncpy(psw,resp+idl,(idr-idl));


    printf("command ====>ssidname:%s\n",ssidname);
    printf("command ====>psw:%s\n",psw);
    
    Ctrl(ssidname,strlen(ssidname),NULL,NULL, psw);
    free(ssidname);
    free(psw);
    return 0;
}
int readmac(u8 *line, u8 *mac, int *pos, int len)
{
    int t_len=0;
    while(line[(*pos)]!='#' && (*pos)<len){
        mac[t_len++]=line[(*pos)++];
    }
    if(t_len!=6)return -1; 
    printf("readmac \n");
    int i;
    for(i=0;i<6;i++)
        printf("0x%02x ",mac[i]);
    return (*pos);
}
int sendtoserver(int sockfd,u8 *line)
{
    //        write(fd[0],"YES",3);
    //if(strlen(resp)>2){
    //    memset(resp,0,sizeof(resp));
    //}
    int i,j;
    printf("mac:%s\n",line);
    char resp[1024];
    memset(resp,0,sizeof(resp));
    sendingmsg(sockfd,line);
    int nbyte;
    nbyte=recevemsg(sockfd,resp);
    printf("recevemsg in haha with while:%s\n\n",resp);
    if(nbyte<=3) {
        printf("YES\n");
        if(command(resp)<0)
            printf("Command Error\n");
    }else{
        printf("resp no:%s\n",resp);
    }

}
int main(int argc,char *argv[])
{
    init();
    int n,i,j;
    printf("in haha %d\n",argc);
    //int sockfd=client(PORT,"104.236.139.146");

    //from pipe connect hostapd
    if(argc==1){
        fd[0]=atoi(argv[0]);
    }
    while(1){
        memset(line,0,sizeof(line));
        n=read(fd[0],line,512);
        // write(STDOUT_FILENO,line,n);

        printf("\nchild read %d byte\n",n);
        int pos=0;

        while(1) {
            u8 mac[7];
            ++pos;
            printf("line:%s\n",line);
            if(readmac(line,mac,(&pos),n)==-1){
                printf("readmac%s\n",mac);
                break;
            }
            mac[6]='\0';
            
            int sockfd=client(PORT,"127.0.0.1");
            sendtoserver(sockfd,mac);
            closesocket(sockfd);
        }


    }
    printf("end\n");
}
