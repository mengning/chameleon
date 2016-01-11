#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse.h"
#include "token.h"

#ifndef DEBUG
#define DEBUG 0
#endif

Body_t Body_new (char *key, char *value)
{
  Body_t p = malloc (sizeof(*p));
  if (0==p)
    outOfMemory();

  strcpy(p->key, key);
  strcpy(p->value,value);
  return p;
}
enum {A_NONE, A_SPACE, A_CRLF} ahead = A_NONE;

struct Token_t token = {-1, 0};

Http_t http400 = 0;
Http_t generate400();

//////////////////////////////////////
// prototypes
char *eatToken (enum TokenKind_t, int);

char getChar (int fd)
{
  int n;
  char c;
  char *info;

  n = read (fd, &c, 1);
  switch (n){
  case -1:
    info = "fail to read the socket\n";
    write (1, info, strlen (info));
    Http_print (fd, http400);
    close (fd);
    exit (0);
    break;
  case 0: // socket is blocked
    info = "read 0 char\n";
    write (1, info, strlen (info));
    Http_print (fd, http400);
    close (fd);
    exit (0);
    break;
  case 1:
    break;
  default:
    info = "server bug\n";
    write (1, info, strlen (info));
    Http_print (fd, http400);
    close (fd);
    exit (0);
    break;
  }
  return c;
}

int i;
int gfd;
void getToken (int fd, int sepBySpace)
{  
  i = 0;
  char c;
  char s[1024];
  gfd=fd;
  switch (ahead){
  case A_NONE:
    c = getChar (gfd);
    break;
  case A_SPACE:
    ahead = A_NONE;
    Token_new(token, TOKEN_SPACE, 0);
    return;
  case A_CRLF:
    ahead = A_NONE;
    Token_new(token, TOKEN_CRLF, 0);
    return;
  default:{
    char *info = "server bug";
    write (1, info, strlen (info));
    Http_print (gfd, http400);
    close (gfd);
    exit (0);
    return;
  }
  }

  while (1){
    switch (c){
    case ' ':
      if (sepBySpace){
	if (i){
	  char *p;
	  int kind;
	  
	  // remember the ' '
	  ahead = A_SPACE;
	  s[i] = '\0';
	  p = malloc (strlen(s)+1);
	  strcpy (p, s);
	  kind = Token_getKeyWord (p);
	  if (kind>=0){
	    
	    Token_new (token, kind, 0);
	    return;
	  }
	  Token_new (token, TOKEN_STR, p);
	  return;
	}
	Token_new(token, TOKEN_SPACE, 0);
	return;
      }
      s[i++] = c;
      break;
    case '\r':{
      char c2;
      
      c2 = getChar (gfd);
      if (c2=='\n'){
	if (i){
	  char *p;
	  int kind;
	  // remember the ' '
	  ahead = A_CRLF;
	  s[i] = '\0';
	  p = malloc (strlen(s)+1);
	  strcpy (p, s);
	  kind = Token_getKeyWord (p);
	  if (kind>=0){
	    Token_new (token, kind, 0);
	    return;
	  }
	  Token_new (token, TOKEN_STR, p);
	  return;
	}
	Token_new(token, TOKEN_CRLF, 0);
	return;
      }
      s[i++] = c;
      s[i++] = c2;
      break;
    }
    default:
      s[i++] = c;
      break;
    }
    c = getChar (gfd);
  }
  return;
}

void parseError(int fd)
{
  //eatAllChars(fd);
  Http_print (fd, http400);
  fprintf (stderr, "%s\n", "parse error");
  close (fd);
  exit (0);
}

char *eatToken(enum TokenKind_t kind, int fd)
{
  if (token.kind == kind)
    return token.lexeme;

  fprintf (stderr, "Error: parse error\n");
  fprintf (stderr, "expects: %s\n"
	   , TokenKind_toString (kind));
  fprintf (stderr, "but got: %s\n"
	   , TokenKind_toString(token.kind));
  parseError (fd);
  return 0;
}

ReqLine_t Parse_reqLine (int fd)
{
  enum ReqKind_t kind;
  ReqLine_t reqline;
  
  getToken(fd, 1);
  //printf("parse(F) uri =  %s; kind = %d; n = %d\n"
  //, token.lexeme, token.kind, TOKEN_POST);
  switch(token.kind){
  case TOKEN_HEAD:
    kind = REQ_KIND_HEAD;
    break;
  case TOKEN_GET:
    kind = REQ_KIND_GET;
    break;
  case TOKEN_POST:    
    kind = REQ_KIND_POST;
    break;
  default:
    parseError(fd);
    break;
  }
  
  getToken(fd, 1);      
  eatToken(TOKEN_SPACE, fd);  
  getToken (fd, 1);  
  char *uri = eatToken(TOKEN_STR, fd);
  getToken (fd, 1);
  eatToken(TOKEN_SPACE, fd);
  getToken(fd, 1);
  eatToken(TOKEN_HTTP_ONE_ONE, fd);
  getToken(fd, 1);
  eatToken(TOKEN_CRLF, fd);

  if (DEBUG){
    fprintf (stderr, "uri=%s\n", uri);
  }

  reqline = ReqLine_new (kind
			 , uri
			 , HTTP_ONE_ONE);
  ReqLine_print (1, reqline);
  return reqline;
}

Header_t parseHeaders (int fd, int *bodyLength)
{
  int i = 0;
  Header_t p = 0;

  getToken(fd, 1);

  fprintf (stderr, "header starting\n");
  while (token.kind != TOKEN_CRLF){
    // this should not be string, a dirty hack
    char *key = eatToken(TOKEN_STR, fd);
    if (DEBUG)
      fprintf (stderr, "%s", key);
    
    getToken(fd, 1);
    eatToken(TOKEN_SPACE, fd);
    
    getToken (fd, 0);
    char *value = eatToken(TOKEN_STR, fd);
    if (DEBUG)
      fprintf (stderr, "%s\n", value);
    
    getToken (fd, 1);
    eatToken(TOKEN_CRLF, fd);

    // the order is reversed...
    p = Header_new (key, value, p);
    // the content length is used to parse the body
    if (strcmp(key, "Content-Length:")==0)
      *bodyLength = atoi(value);
    
    getToken (fd, 1);
  }
  return p;
}

char *parseBody (int fd, int contentLength)
{
  if(contentLength==0) 
    return 0;

  if (DEBUG)
    printf("Parse body starting...\n");
  
  char *body = (char*)malloc(contentLength+1);
  memset(body, 0, contentLength+1);
  
  int i = 0;
  while(i < contentLength)
    body[i++] = getChar(fd);
  if (DEBUG)
    printf("HTTP body=[%s]\n", body);
  return body;
}

void *Parse_parse (int fd, int reqOnly)
{
  http400 = generate400();  
  ReqLine_t reqline = 0;
  Http_t http = 0;

  if(reqOnly){
    reqline = Parse_reqLine (fd);
    return reqline;
  }
  
  int bodyLength = 0;
  Header_t headers = parseHeaders(fd, &bodyLength);
  if(DEBUG)
    fprintf (stderr, "content-length=%d\n", bodyLength);

  if (token.kind!=TOKEN_CRLF)
    parseError(fd);
  
  char *body = parseBody(fd, bodyLength);  
  http = Http_new (HTTP_KIND_REQUEST
		   , reqline
		   , 0 // respond
		   , headers
		   , body);
  
  if (DEBUG){
    fprintf (stderr, "%s\n", "Print HTTP starting...");
    Http_print (1, http);
    fprintf (stderr, "%s\n", "Print HTTP finished.");
  }
  return http;
}

////////////////////////////////////
// 400
Http_t generate400()
{
  enum HttpKind_t kind = HTTP_KIND_RESPONSE;
  RespLine_t respLine;
  Header_t header = 0;
  char *body;
  Http_t http = 0;

  respLine = 
    RespLine_new (HTTP_ONE_ONE
		  , RESP_400
		  , "bad request");
  body = "";
  http = Http_new (kind
		   , 0
		   , respLine
		   , header
		   , body);
  return http;
}

