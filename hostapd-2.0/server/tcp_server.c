#include <netdb.h>
#include <errno.h>
#include <syslog.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <sys/types.h>      /* some systems still require this */
#include <sys/stat.h>
#include <sys/termios.h>    /* for winsize */
#include <stdio.h>        /* for convenience */
#include <stdlib.h>     /* for convenience */
#include <stddef.h>     /* for offsetof */
#include <string.h>     /* for convenience */
#include <unistd.h>     /* for convenience */
#include <signal.h>     /* for SIG_ERR */
#include <sys/resource.h>
#define BUFLEN 128
#define QLEN 10

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 256
#endif

#define	MAXLINE	4096
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
void
daemonize(const char *cmd)
{
	int i, fd0, fd1, fd2;
	pid_t pid;
	struct rlimit rl;
	struct sigaction sa;

	/*
	 * Clear file creation mask.
	 */
	umask(0);

	/*
	 * Get maximum number of file descriptors.
	 */
	if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
		err_quit("%s: can't get file limit", cmd);

	/*
	 * Become a session leader to lose controlling TTY.
	 */
	if ((pid = fork()) < 0)
		err_quit("%s: can't fork", cmd);
	else if (pid != 0) /* parent */
		exit(0);
	setsid();

	/*
	 * Ensure future opens won't allocate controlling TTYs.
	 */
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGHUP, &sa, NULL) < 0)
		err_quit("%s: can't ignore SIGHUP", cmd);
	if ((pid = fork()) < 0)
		err_quit("%s: can't fork", cmd);
	else if (pid != 0) /* parent */
		exit(0);

	/*
	 * Change the current working directory to the root so
	 * we won't prevent file systems from being unmounted.
	 */
	if (chdir("/") < 0)
		err_quit("%s: can't change directory to /", cmd);

	/*
	 * Close all open file descriptors.
	 */
	if (rl.rlim_max == RLIM_INFINITY)
		rl.rlim_max = 1024;
	for (i = 0; i < rl.rlim_max; i++)
		close(i);

	/*
	 * Attach file descriptors 0, 1, and 2 to /dev/null.
	 */
	fd0 = open("/dev/null", O_RDWR);
	fd1 = dup(0);
	fd2 = dup(0);

	/*
	 * Initialize the log file.
	 */
	openlog(cmd, LOG_CONS, LOG_DAEMON);
	if (fd0 != 0 || fd1 != 1 || fd2 != 2) {
		syslog(LOG_ERR, "unexpected file descriptors %d %d %d",
		  fd0, fd1, fd2);
		exit(1);
	}
}
// 下面这个用法是不完备的
int initserver2(int type,const struct sockaddr *addr,socklen_t alen,int qlen){
    int fd;
    int err=0;
    if((fd=socket(addr->sa_family,type,0))<0)return (-1);
    
    if(bind(fd,addr,alen)<0)
        goto errout;

    if(type==SOCK_STREAM || type==SOCK_SEQPACKET){
        if(listen(fd,qlen)<0)goto errout;
    }
    return (fd);

errout:
    err=errno;
    close(fd);
    errno=err;
    return (-1);
}

int initserver(int type, const struct sockaddr *addr, socklen_t alen,
  int qlen)
{
	int fd, err;
	int reuse = 1;

	if ((fd = socket(addr->sa_family, type, 0)) < 0)
		return(-1);
    //当服务器终止并且尝试立即重启时候，上面的函数无法正常工作
    //启用SO_REUSEADDR选项绕开不允许绑定同一个地址
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse,
	  sizeof(int)) < 0)
		goto errout;
	if (bind(fd, addr, alen) < 0)
		goto errout;
	if (type == SOCK_STREAM || type == SOCK_SEQPACKET)
		if (listen(fd, qlen) < 0)
			goto errout;
	return(fd);

errout:
	err = errno;
	close(fd);
	errno = err;
	return(-1);
}

int set_cloexec(int fd){
    int val;
    if((val=fcntl(fd,F_GETFD,0))<0)
        return (-1);
    val |=FD_CLOEXEC;//enable close on exec
    return (fcntl(fd,F_SETFD,val));
}
 int recvdata(int sockfd)
{
    int n;
    char buf[300];
    while((n=recv(sockfd,buf,300,0))>0)
        write(STDOUT_FILENO,buf,n);
    if(n<0)
        printf("recv error");
}


void server (int sockfd)
{
    int clfd;
    FILE *fp;
    char buf[BUFLEN];
    //对被执行的程序不需要的文件描述符设置执行时关闭标志，让别人不用sockfd
    set_cloexec(sockfd);
    for(;;){

        //一旦服务器listeln成功，就可以accept获得链接
        //第一个参数为原始描述符
        //第二个为sockaddr 地址，如果不关心客户端标识可以为NULL
        //第三个为len
        if((clfd=accept(sockfd,NULL,NULL))<0){
            syslog(LOG_ERR,"ruptimed:accept error: %s",strerror(errno));
            exit(1);
        }
        //recv data
        recvdata(sockfd);
        
        //打开文件发送data的数据
        set_cloexec(clfd);
        if((fp=popen("/home/luyuncheng/Workspace/Myhostapd/Hostapd/server/data","r"))==NULL){
            sprintf(buf,"error: %s\n",strerror(errno));
            send(clfd,buf,strlen(buf),0);
        }else{
            while(fgets(buf,BUFLEN,fp)!=NULL)
                send(clfd,buf,strlen(buf),0);
            pclose(fp);
        }
        close(clfd);
    }
}
int main(int argc,char *argv[])
{
    struct addrinfo *ailist,*aip;
    struct addrinfo hint;
    int sockfd,err,n;
    char host[]={"127.0.0.1"};

    if(argc!=1)
        err_quit("usage:ruptimed");
    if((n=sysconf(_SC_HOST_NAME_MAX))<0)
        n=HOST_NAME_MAX;
    //if((host=malloc(n))==NULL)
    //    err_sys("malloc error");

 //   if(gethostname(host,n)<0)
 //       err_sys("gethostname error");
    //daemonize("ruptimed");
    printf("host:%s\n",host);
    printf("start server pid=%d\n",getpid());
    memset(&hint,0,sizeof(hint));
    //用hint选择符合特定条件的地址，过滤地址
    //下面这个参数是需要一个规范的名字（与别名相对）
    hint.ai_flags=AI_CANONNAME;
    //hint.ai_flags=AI_ALL;
    
    hint.ai_socktype=SOCK_STREAM;
    hint.ai_canonname=NULL;
    hint.ai_addr=NULL;
    hint.ai_next=NULL;

    //将一个主机名和服务名映射到一个地址
    //第一个参数为主机名，第二个为服务名，如果仅仅提供一个，另一个需要为空
    //第三个为addrinfo
    //第四个为返回的addrinfo链表结构
    if((err=getaddrinfo(host,NULL,&hint,&ailist))!=0){
        syslog(LOG_ERR,"ruptimed:getaddrinfo error: %s",gai_strerror(err));
        printf("getaddrinfo error\n");
        exit(1);
    }
    for(aip=ailist;aip!=NULL;aip=aip->ai_next){
        //为每一个addrinfo初始化server
        if((sockfd = initserver(SOCK_STREAM,aip->ai_addr,aip->ai_addrlen,QLEN)) >=0){
            server(sockfd);
            exit(0);
        }
        
    }
    printf("exit\n");
    exit(1);
}

