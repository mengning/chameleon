#include <netdb.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>        /* for convenience */
#include <stdlib.h>     /* for convenience */
#include <stddef.h>     /* for offsetof */
#include <string.h>     /* for convenience */
#include <unistd.h>     /* for convenience */
#include <signal.h>     /* for SIG_ERR */
#include <sys/socket.h>
#include <sys/types.h>      /* some systems still require this */
#include <sys/stat.h>
#include <sys/termios.h>    /* for winsize */

#define	MAXLINE	4096
#define MAXSLEEP 128
#define BUFLEN 128
static void
err_doit(int errnoflag, int error, const char *fmt, va_list ap)
{
    char    buf[MAXLINE];

    vsnprintf(buf, MAXLINE-1, fmt, ap);
    if (errnoflag)
        snprintf(buf+strlen(buf), MAXLINE-strlen(buf)-1, ": %s",
                strerror(error));
    strcat(buf, "\n");
    fflush(stdout);     /* in case stdout and stderr are the same */
    fputs(buf, stderr);
    fflush(NULL);       /* flushes all stdio output streams */
}

void
err_quit(const char *fmt, ...)
{
    va_list     ap;

    va_start(ap, fmt);
    err_doit(0, 0, fmt, ap);
    va_end(ap);
    exit(1);
}

void
err_exit(int error, const char *fmt, ...)
{
    va_list		ap;

    va_start(ap, fmt);
    err_doit(1, error, fmt, ap);
    va_end(ap);
    exit(1);
}
void
err_sys(const char *fmt, ...)
{
    va_list		ap;

    va_start(ap, fmt);
    err_doit(1, errno, fmt, ap);
    va_end(ap);
    exit(1);
}
//可迁移的支持重试的连接
int connect_retry(int domain,int type,int protocol,
        const struct sockaddr *addr,socklen_t alen)
{
    int numsec,fd,cont=0;
    for(numsec=1;numsec<=MAXSLEEP;numsec<<=1){
        printf("the %d times connect numsec%d\n ",cont++,numsec);
        if((fd=socket(domain,type,protocol))<0) return (-1);
        if(connect(fd,addr,alen)==0){
            return (fd);
        }
        close(fd);
        //delay before trying again
        if(numsec<=MAXSLEEP/2)
            sleep(numsec);
    }
    return (-1);
}
void senddata(int sockfd)
{
    char *req ="this is connection from client\n";
    write(sockfd,req,strlen(req));
    printf("sneddata over\n");
}
void print_uptime(int sockfd)
{
    int n;
    char buf[BUFLEN];
    while((n=recv(sockfd,buf,BUFLEN,0))>0)
        write(STDOUT_FILENO,buf,n);
    if(n<0)
        err_sys("recv error");
}
int start(int argc,char *argv[]){
    struct addrinfo *ailist,*aip;
    struct addrinfo hint;
    int sockfd,err;
    if(argc!=2)
        err_quit("usage: ruptime hostname");
    memset(&hint,0,sizeof(hint));
    hint.ai_socktype=SOCK_STREAM;
    hint.ai_canonname=NULL;
    hint.ai_addr=NULL;
    hint.ai_next=NULL;
    //输入的第一个参数为host的主机名字，第二个是服务名
    //第三个是传入的hint，用来过滤socket，
    //第四个是返回的addrinfo的链表
    if((err=getaddrinfo(argv[1],"",&hint,&ailist))!=0)
        err_quit("getaddrinfo error: %s",gai_strerror(err));
    printf("getaddrinfo....over\n");
    for(aip=ailist;aip!=NULL;aip=aip->ai_next){
        if((sockfd=connect_retry(AF_INET,SOCK_STREAM,0,
                        aip->ai_addr,aip->ai_addrlen))<0)
        {
            err=errno;
        }
        else
        {
            senddata(sockfd);
            print_uptime(sockfd);
            exit(0);
        }
    }
    err_exit(err,"can not connect to %s",argv[1]);

}
int main(int argc,char *argv[])
{
    start(argc,argv);
}

