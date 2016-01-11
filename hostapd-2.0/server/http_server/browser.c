#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>

#define PORT  8989


int main(int argc, char *argv[])
{ 
int  port = PORT;
  if (argc>1)
    port = atoi(argv[1]);

  int sock_client = socket(AF_INET,SOCK_STREAM, 0);//sock fd
  
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);  //server port
  
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");  ///server ip address

  
  if (connect(sock_client, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
      perror("connect");
      exit(1);
    }
 
  printf("sock_client = %d\n",sock_client);
  

  /*
    below show that client send a normal request to the web server
    you should fix the code to realize your attack
  */
  
  char *req ="POST / HTTP/1.1\r\nContent-Length: 84\r\nCookie: wanglei=123456\r\n\r\nsubmit_transfer=Transfer&transfer_from=wanglei&transfer_to=xiaohua&transfer_money=50";

  char *req1 ="POST / HTTP/1.1\r\nContent-Length: 29\r\n\r\nsubmit=checkmac&macvalue=1234";
  write(sock_client,req1,strlen(req1));
  
 /**************************receive the response from web server******************************************/
  char resp[1024];
  int num = 0;
  while(read (sock_client, &resp[num], 1))
	num++;
  resp[num] = 0;
  printf("Response = %s\n",resp);
  
  close(sock_client);
  return 0;
}
