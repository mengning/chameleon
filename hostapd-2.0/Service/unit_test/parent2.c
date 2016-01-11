#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#define u8 unsigned char
//54:E4:3A:3D:63:C7
u8 macaddr[]={0124,0344,072,075,0143,0307};
//54:E4:3A:3D:6D:0A
u8 macaddr2[]={0124,0344,072,075,0155,012};
int bz=0;
int flagbz=0;
const int MAXLINE=20;
int n;
int fd[2];
pid_t id_h;
void inttostr(char *s,int val){
    int i=0,len=0;
    while(val){
        s[len++]=val%10+'0';
        val=val/10;
    }
    for(i=0;i<len/2;i++){
        char tmp=s[i];
        s[i]=s[len-i-1];
        s[len-i-1]=tmp;
    }
    s[len]='\0';
}    
int haha(char *macaddr,int len){
    int i;
    if(bz==0){
        if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd)){    
            printf("pipe error");
            return -1;
        }
        if((id_h=fork())<0){
            printf("fork error\n");
            return -1;
        }else{
            bz=1;
        }
    }
    if(bz==1){
        if(id_h >0){//parent
            close(fd[0]);
//            printf("parent write %d byte\n",len);
//            for( i=0;i<len;i++)
//                printf("%02x ",macaddr[i]);
//            printf("\n");
         //   write(fd[1],macaddr,len);
        }else if(flagbz && id_h==0){//child
            printf("flagbz\n");
        }else if(flagbz==0 && id_h==0){//child
            printf("in child\n");
            
            close(fd[1]);
//          dup2(fd[1],STDOUT_FILENO);//复制管道的子进程端口到标准输出
//            dup2(fd[1],STDIN_FILENO);//复制管道的子进程端口到标准输入
//            close(fd2[0]);
            char pip_str[32]={'\0'};
            printf("fd:int: %d\n",fd[0]);
            //inttostr(pip_str,fd[0]);
            sprintf(pip_str,"%d",fd[0]);
            printf("fd:s:   %s\n",pip_str);
            //if(execl("/home/luyuncheng/Workspace/Myhostapd/Hostapd/Service/","./haha",s,(char *)0)<0)
            if(execl("/home/luyuncheng/Workspace/Myhostapd/Hostapd/Service/./haha",pip_str,(char *)0)<0)
                printf("execl error");
            else
                flagbz=1;
        }
    }
}

int main()
{
    bz=0;
    char tmp[128];
    haha(macaddr,6);
    if(bz==1 && id_h>0){
        //memset(tmp,0,sizeof(tmp));
        //read(fd2[0],tmp,100);
        write(fd[1],macaddr,6);
        
        memset(tmp,0,sizeof(tmp));
        read(fd[1],tmp,100);
        write(fd[1],macaddr2,6);
    }

    for(;;){
    }
   /* while(1)
    {
        if(bz==1 && id_h>0){
            write(fd[1],"haha",2);
        }
        printf("helloworld\n\n\n\n");
    }*/
    /*
    if(pipe(fd)<0)
	    printf("pipe error");
    if((pid=fork())<0){
	    printf("fork error");
    }else if(pid >0){//parent
	    close(fd[0]);
	    write(fd[1],"hello world\n",12);
    }else{//child
        printf("in child\n");
	    close(fd[1]);
        if(bz==0){
            bz=1;
            char s[33];
            printf("fd:int: %d\n",fd[0]);
            fun(s,fd[0]);
            printf("fd:s:   %s\n",s);
            //if(execl("/home/luyuncheng/Workspace/Myhostapd/Hostapd/Service/","./haha",s,(char *)0)<0)
            if(execl("/home/luyuncheng/Workspace/Myhostapd/Hostapd/Service/./haha",s,(char *)0)<0)
                printf("execl error");
        }
        if(bz){
            printf("bz=1\n");
           // bz=1;
	       // n=read(fd[0],line,MAXLINE);
	       // write(STDOUT_FILENO,line,n);
        }
    }
    while(1){
        write(fd[1],"hello world\n",12);
    }
    exit(0);*/
}
