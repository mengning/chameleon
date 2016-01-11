#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "sql-lite3/dbutil.h"
#include "parse.h"
#include "handle.h"
#ifndef DEBUG
#define DEBUG 0
#endif
static char *fileOk = 
"HTTP/1.1 200 OK\r\n\r\n";
static char *fileNotExist =
"<html><b>404</b>: File does not exist!</html>";
static char *noPermission = 
"<html><b>401</b>: Access denied: no permission!</html>";
// lookup the HTTP body to find key
static char *lookup (char *key);
static char *kv_lookup(char *key);
void handleHead (char *uri, int fd)
{
    // to do 
    return;
}

static char *getTime()
{
    time_t now;
    char *t;

    time(&now);
    t = ctime(&now);
    return t;
}

void handleGet (char *uri, int fd)
{
    char *path;
    char *info;

    assert (uri);
    if (DEBUG)
        printf("uri=[%s]\n", uri);
    if (strcmp (uri, "/")==0)
        uri = "/index.html";
    path = malloc (strlen (uri)+3);
    strcpy (path, "./");
    strcpy (path+2, uri);
    if (DEBUG)
        printf ("path=[%s]\n", path);

    if (access(path, F_OK)!=0){
        fprintf (stderr, "file: %s not found\n"
                , path);
        info = "HTTP/1.1 404 File Not Found\r\n\r\n";
        write (fd, info, strlen(info));
        write (fd, fileNotExist, strlen (fileNotExist));
        return;
    }
    if (access(path, R_OK)!=0){
        fprintf (stderr, "file: %s no perm\n"
                , path);
        info = "HTTP/1.1 401 No Permission\r\n\r\n";
        write (fd, info, strlen(info));
        write (fd, noPermission, strlen (noPermission));
        return;
    }
    fprintf (stderr, "file: %s OK\n"
            , path);
    info = "HTTP/1.1 200 OK\r\n\r\n";
    write (fd, info, strlen(info));

    char c;
    int file = open (path, O_RDONLY);
    while (read (file, &c, 1)){
        write (fd, &c, 1);
    }
    close (file);
    return;
}

////////////////////////////////////////////
// handle various POST-related requests.
// post login
void handlePostLogin (int fd
        , char *name
        , char *passwd
        , int checkPasswd)
{
    int t = 0;

    if(checkPasswd)
        t = Db_checkUserPasswd(name, passwd);
    else
        t =1;
    printf("name:%s passwd:%s t:%d\n",name,passwd,t);
    char c[1024];
/*    if (checkPasswd)
    {
         
        char * cookieheader = "HTTP/1.1 200 OK\r\nSet-Cookie: ";
        char temp[20];
        int i;
        for(i=0;;i++)
        {
            if(passwd[i]==0)
            {
                temp[i]=passwd[i];
                break;
            } 
            temp[i]=passwd[i]*2+1;
        }
        sprintf(c,"%s%s=%s\r\n\r\n",cookieheader,name,temp);//cookie值为密码
        printf("cookie::::::%s\n",c);
        write(fd,c,strlen(c));
    }
    else t = 1;
    // not a valid user.
    t=1;*/
    if (!t){
        sprintf(c,
                "<html>"
                "<head>"
                "<title>login failed</title>"
                "</head>"
                "<body>Sorry, %s. Login fail.."
                "<p>Please login again."
                "<p><form name='logoutform' method='POST' ><table><tr><td> <input type='submit' name='submit_exit' value='Exit'></td></tr></table></form>"
                "</body></html>"
                , name);
        char *info = fileOk;
        write(fd, info, strlen(info));
        write(fd, c, strlen(c));
        close(fd);
        return;
    }
    /*wanglei*/
    if(!checkPasswd)
    {
        char * ok = "HTTP/1.1 200 OK\r\n\r\n";
        write(fd,ok,strlen(ok));
    }
    // login success!
    char *logintime = getTime ();
    char * balance = Db_readBalance (name);
    //wanglei
    sprintf(c,
            "<html>"
            "<head>"
            "<title>Welcome</title>"
            "</head>"
            "<body>Welcome you: <b>%s</b> to the SSE Chameleon."
            "<p>"
            "Current time is: %s."
            "<p>"
            "Your current mac is: <b>%s</b>."
            "<p>"
            "<form name='transferform' method='POST'>"
            "<table>"
            "<tr>"
            "<td>&nbsp;</td>"
            "<td> <input type='hidden' name='current_user' value='%s'>"
            "</td>"
            "</tr>"
            "<tr>"
            "</td>"
            "</tr>"
            "<tr>"
            "<td>SET Mac:</td>"
            "<td> <input type='text' name='macaddress' size='20' autocomplete='no'>"
            "</td>"
            "</tr>"
            "<tr>"
            "<td>&nbsp;</td>"
            "<td> <input type='submit' name='setmac' value='change'>"
            "</td>"
            "</tr>"
            "</table>"
            "</form>"
            "<p>"
            "<form name='logoutform' method='POST'>"
            "<table>"
            "<tr>"
            "<td> <input type='submit' name='submit_logout' value='Logout'>"
            "</td>"
            "</tr>"
            "</table>"
            "</form>"
            "</body></html>"
            , name
            , logintime
            , balance
            , name);
    char *info =fileOk;
    write(fd,info,strlen(info));
    write(fd, c, strlen(c));
    close (fd);
    return;  
}

// POST: register
void handlePostRegister (int fd
        , char *name
        , char *passwd)
{
    int t = Db_checkUser(name);
    char c[1024];

    // user exists
    if(t){
        sprintf(c,
                "<html>"
                "<head>"
                "<title>register failed</title>"
                "</head>"
                "<body>Sorry, %s has registered!"
                "<p>Please register again."
                "<p><form name='exitform' method='POST'><table><tr><td> <input type='submit' name='submit_exit' value='Exit'>"
                "</td>"
                "</tr>"
                "</table>"
                "</form></body></html>"
                , name);
        char *info = fileOk;
        write(fd, info, strlen(info));
        write(fd, c, strlen(c));
        close (fd);
        return;
    }
    //
    char *logintime = getTime();
    Db_registerUser(name, passwd);
    sprintf(c
            , "<html>"
            "<head>"
            "<title>register succeed</title>"
            "</head>"
            "<body>Welcome, <b>%s</b>, you register succeed."
            "<p>The current time is: %s."
            "<p><form name='logoutform' method='POST'>"
            "<table><tr><td> <input type='submit' name='submit_logout'"
            "value='Logout'></td></tr></table></form></body></html>"
            , name
            , logintime);
    char *info = fileOk;
    write(fd, info, strlen(info));
    write(fd, c, strlen(c));
    close(fd);
    return;
}

void handlePostLogout (int fd)
{
    char *info = "HTTP/1.1 303 SeeOther\r\n"
        "Location: ./index.html\r\n\r\n";

    write (fd, info, strlen (info));
    close (fd);
    return;
}

// POST
// data structure and values holding body info
struct kv{
    char *key;
    char *value;
};
static struct kv temp[16];
static struct kv cook[16];
static char *cook_lookup(char *key)
{
    int i = 0;
    while (i<16 && cook[i].key){
        if (strcmp(cook[i].key, key)==0)
            return cook[i].value;
        // a dirty hack
        if (cook[i].key[0]=='s'
                && key[0]=='s')
            return cook[i].value;
        i++;
    }
    return 0;
}
static void getcookv (char *s)
{
    if(!s)return;
    int len = strlen(s);
    char *start = s;
    int num = 0;
    while(start<s+len){
        char *end = strchr(start, '=');
        *end = '\0';
        cook[num].key = start;
        start = end+1;
        end = strchr(start, '&');
        if(end == 0){
            cook[num].value = start;
            break;
        }
        *end = '\0';
        cook[num].value = start;
        start = end+1;
        num++;}
    return;
}

static char *kv_lookup(char *key)
{
    int i = 0;
    while (i<16 && temp[i].key){
        if (strcmp(temp[i].key, key)==0)
            return temp[i].value;
        // a dirty hack
        if (temp[i].key[0]=='s'
                && key[0]=='s')
            //这里是解决submit没有没有匹配相的情况，恰好setmac第一个字母为s
            return temp[i].value;
        i++;
    }
    return 0;
}

static void kv_print ()
{
    int i=0;
    while (i<16 && temp[i].key){
        fprintf (stderr, "kv[%d]=%s, %s\n"
                , i
                , temp[i].key
                , temp[i].value);
        i++;
    }
    return;
}

// read values from the body.
static void getValues (Http_t tree,int fd)
{
    assert (tree);
    char *body = tree->body;
    assert (body);
    //wanglei
    char *head = tree->headers;
    assert(head);
    int len = strlen(body);

    char *myCookie=Header_search(head,"Cookie:");
    getcookv(myCookie);

    char *start = body;
    int num = 0;
    while(start<body+len){
        char *end = strchr(start, '=');
        *end = '\0';
        temp[num].key = start;
        start = end+1;
        end = strchr(start, '&');
        if(end == 0){
            temp[num].value = start;
            break;
        }
        *end = '\0';
        temp[num].value = start;
        start = end+1;
        num++;
    }
    return;
}

void handlePost(Http_t tree, int fd)
{
    getValues (tree,fd);
    kv_print();
    char *name = 0; 
    char *passwd = 0;
    char *type = kv_lookup ("submit");
    Db_init(0);

    if (strcmp (type, "Exit")==0
            || strcmp (type, "Logout")==0){
        handlePostLogout(fd);
    }
    else if (strcmp (type, "Login")==0){
        name = kv_lookup ("login_username");
        passwd = kv_lookup ("login_password");
        assert(name);
        assert(passwd);
        handlePostLogin (fd, name, passwd, 1);
    }
    else if (strcmp (type, "Register")==0){
        name = kv_lookup ("login_username");
        passwd = kv_lookup ("login_password");
        assert(name);
        assert(passwd);
        handlePostRegister (fd, name, passwd);
    }
    else if (strcmp (type, "change")==0)
    {
        char *cuser = kv_lookup ("current_user");
        char *macad = kv_lookup ("macaddress");
        assert(cuser);
        assert(macad);
        Db_writeBalance(cuser,macad);
        handlePostLogin(fd,cuser,0,0);
        close(fd);
    }
    /*wanglei*/
    else if(strcmp (type,"checkmac")==0)
    {
        char c[1000];
        char* value = kv_lookup("macvalue");
        char* ssid  = db_getSSID(value);
        sprintf(c,"#%s",ssid);
        char* pass  = db_getPASSWD(value);
        sprintf(c,"%s:%s#",c,pass);
        write(fd,c,strlen(c));
        close(fd);
    }/**/
    else {
        fprintf (stderr, "%s\n", "server bug");
        close(fd);
    }
    return;
}

void Handle_main (int fd, Http_t tree)
{
    assert (tree);
    if (tree->kind != HTTP_KIND_REQUEST){
        fprintf (stderr, "%s\n", "server bug");
        close (fd);
        exit (0);
    }

    ReqLine_t reqline = tree->reqLine;

    assert (reqline);
    switch (reqline->kind){
        case REQ_KIND_GET:
            handleGet(reqline->uri, fd);
            break;
        case REQ_KIND_HEAD:
            handleHead(reqline->uri, fd);
            break;
        case REQ_KIND_POST:
            handlePost(tree, fd);
            break;
        default:
            fprintf (stderr, "%s\n", "server bug");
            close (fd);
            exit (0);
            break;
    }  
    return;
}
