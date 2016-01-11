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
  Http_t tree;

  if (argc<2)
    die ("server bug");

  signal(SIGCHLD, SIG_IGN);
  
  // get the pipe fd
  int pipefd = atoi (argv[1]);
  if (DEBUG)
    printf ("pipefd = %d\n", pipefd);

  // read the two fds from the pipe (sent from the
  // server)
  char temp[32];
  int file_fd;
  if (-1==recvfd(pipefd, temp, sizeof(temp), &file_fd))
    die ("recv fd");
  if (DEBUG)
    printf ("file_fd = %d\n", file_fd);
  int bank_fd;
  char temp2[32];
  if (-1==recvfd(pipefd, temp2, sizeof(temp2), &bank_fd))
    die ("recv fd");
  if (DEBUG)
    printf ("bank_fd = %d\n", bank_fd);

  while (1){
    char sockfd_str[32];
    int sockfd;
    if (-1==recvfd (pipefd, sockfd_str, sizeof(sockfd_str), &sockfd))
      die ("recv fd");
    
    printf ("str=%s\n", sockfd_str);

    if (DEBUG)
      printf ("httpd service recieves a sockfd =[%d]\n", sockfd);
    if(fork() == 0){
      // parse the http request
      ReqLine_t reqline = Parse_parse(sockfd, 1);
      if (DEBUG)
	printf ("child-httpd parse reqline:[%s]\n", reqline->uri);
      
      // dispatch to various services
      switch (reqline->kind){
      case REQ_KIND_GET:{
	sendfd(file_fd
	       , reqline->uri
	       , strlen(reqline->uri)+1
	       , sockfd);
	break;
      }
      case REQ_KIND_HEAD:{
	// unhandled
	fprintf (stderr, "to do\n");
	return;
      }
      case REQ_KIND_POST:{
	if (DEBUG)
	  printf("%s\n", "httpd dispatch POST.");
	sendfd(bank_fd
	       , reqline->uri
	       , strlen(reqline->uri)+1
	       , sockfd);
	break;
      }
      default:{
	fprintf (stderr, "error\n");
	return;	
      }
      }      
      exit(0);
    } 
    // will be closed in various services
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

