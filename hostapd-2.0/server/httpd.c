#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "parse.h"
#include "http.h"
#define DEBUG 1

static void error (int fd, int errcode, char *msg);
static void parse(int fd);

// utilities
void die (const char *msg)
{
  fprintf (stderr, "Error: %s\n", msg);
  exit (0);
}

void kprint (const char *msg)
{
  write (1, msg, sizeof (msg));
  return;
}

void write_file (int sockfd, const char *s)
{
  int size = strlen (s);

  write (sockfd, s, size);
  return;
}


int main (int argc, char **argv)
{
    printf("0x%x\n",system);

    printf("0x%x\n",exit);
  int pipefd;
  Http_t tree;

  if (argc<2)
    die ("server bug");

  pipefd = atoi (argv[1]);
  if (DEBUG)
    printf ("pipefd = %d\n", pipefd);//4

  signal(SIGCHLD, SIG_IGN);

  while (1){
    char sockfd_str[32];
    int sockfd;
    recvfd (pipefd, sockfd_str, sizeof(sockfd_str),&sockfd);
    printf("str = %s\n",sockfd_str);
    //int sockfd = atoi (sockfd_str);
    if (DEBUG)
      printf ("client recieves a sockfd = %d\n", sockfd);

    int pid;
    if ((pid=fork())==0){
      // parse the http requst
      tree = Parse_parse(sockfd);
      
      if (DEBUG){
	printf ("parse success"
		"the http tree is:\n");
      }
      
      // response
      Handle_main (sockfd, tree);
      
      close(sockfd);
      
      if (DEBUG)
	printf("socket closed..");
      exit (0);
    }
     close(sockfd);
  }
  return 0;
}

////////////////////////////////////////////
// parser
void error (int fd, int errCode, char *msg)
{
  int c;

  while(read(fd, &c, 1)!=-1)
    ;
  close (fd);
  
  fprintf (stderr, "%d\n", errCode);
  fprintf (stderr, "%s\n", msg);
  exit (0);
  return;
}

