#include <unistd.h>
#include <stdio.h>
void  fun(char *s,int a){
    int i=0,j=0,len=0;    
    while(a){
        s[len++]=a%10+'0';
        a=a/10;
    }
    printf("len:%d   s1:%s\n",len,s);
    for(i=0;i<len/2;i++){
        char tmp=s[i];
        s[i]=s[len-i-1];
        s[len-i-1]=tmp;
    }
    s[len]='\0';
}
int main()
{
    char s[100];
    fun(s,12345);
    printf("%s\n",s);
    fun(s,123456);
    printf("%s\n",s);
    /*
    bz=0;
    int n;
    int fd[2];
    pid_t pid;
    char line[MAXLINE];
    	
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
            if(execl("./haha",s,(char *)0)<0)
                printf("execl error");
        }
        if(bz){
            printf("bz=1\n");
           // bz=1;
	       // n=read(fd[0],line,MAXLINE);
	       // write(STDOUT_FILENO,line,n);
        }
    }
    exit(0);
    */
}
