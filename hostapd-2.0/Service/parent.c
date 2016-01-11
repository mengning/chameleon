#include <unistd.h>
#include <stdio.h>
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
    char ss[2048];
    int i;
    for(i=0;i<len*2;i++){
        ss[i]=macaddr[i];
    }
    ss[len*2]='\0';
    printf("\nss:\n");
    for(i=0;i<len*2;i++){
        printf("%c",ss[i]);
    }
    printf("\n macaddr:\n");
    for(i=0;i<len*2;i++){
        printf("%c",macaddr[i]);
    }
    printf("\n");
    //return 0;
    if(bz==0){
        if(pipe(fd)<0){
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
            write(fd[1],ss,len*2);
        }else if(flagbz && id_h==0){//child
            printf("flagbz\n");
        }else if(flagbz==0 && id_h==0){//child
            printf("in child\n");
            close(fd[1]);
            char s[33];
            printf("fd:int: %d\n",fd[0]);
            inttostr(s,fd[0]);
            printf("fd:s:   %s\n",s);
            //if(execl("/home/luyuncheng/Workspace/Myhostapd/Hostapd/Service/","./haha",s,(char *)0)<0)
            if(execl("/home/luyuncheng/Workspace/Myhostapd/Hostapd/Service/./haha",s,(char *)0)<0)
                printf("execl error");
            else
                flagbz=1;
        }
    }
}
int main()
{
    bz=0;
    haha("hello world\n",6);
    while(1)
    {
        if(bz==1 && id_h>0){
            write(fd[1],"haha",2);
        }
        printf("helloworld\n\n\n\n");
    }
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
