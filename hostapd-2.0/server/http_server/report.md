# Part A: Identity Forgery

Exercise 1

* 转账金额可以为负数，也可能整数溢出
* 转账用户可以为自己，或者不存在用户
* 个人余额没有限制，在余额不足时依然可以转账

Exercise 2
  
* 对转账的金额做限制，检查可转账用户，余额校验

 
 Exercise 3
 
 * 在登录时，用户信息存在transfer_from中，即
  char *from = kv_lookup ("transfer_from");//记录登录者
  
Exercise 4

* 构建如下post请求 char *req ="POST / HTTP/1.1\r\nContent-Length: 84\r\n\r\nsubmit_transfer=Transfer&transfer_from=wanglei&transfer_to=xiaohua&transfer_money=50";
即可在不登陆的情况下转账

Exercise 5

```c
//在login时创建cookie
   char * cookieheader = "HTTP/1.1 200 OK\r\nSet-Cookie: ";
   sprintf(c,"%s%s=%s\r\n\r\n",cookieheader,name,temp);//cookie值为密码
   write(fd,c,strlen(c));
  }
//从数据结构header中查找cookie
  char *head = tree->headers;
  assert(head);
  char *myCookie=Header_search(head,"Cookie:");
  getcookv(myCookie);
//创建函数从headers中读取cookie信息
   static void getcookv (char *s)
   {
     if(!s)return;
     int len = strlen(s);
     char *start = s;
     int num = 0;
     while(start<s+len)
      {
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
//在transfer函数中检验cookie
char* cookievalue=cook_lookup(from);
if(!cookievalue)
    {
     write(fd,fileOk,strlen(fileOk));
     write(fd,noPermission,strlen(noPermission));
     return;
     }
int t=Db_checkUserPasswd(from, cookievalue);
//因为cookie就是密码值，可以通过以上函数来校验cookie的值
  if(!t)
   {
    write(fd,noPermission,strlen(noPermission));
    return;
   }
```

#Part B: Packet Sniffing

Exercise 6

* char *req ="POST / HTTP/1.1\r\nContent-Length: 84\r\nCookie: wanglei=123456\r\n\r\nsubmit_transfer=Transfer&transfer_from=wanglei&transfer_to=xiaohua&transfer_money=50";

#Part C: Encryption

Exercise 7

```c
//在构建cookie时
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
      //简单加密
    }
    sprintf(c,"%s%s=%s\r\n\r\n",cookieheader,name,temp);
    write(fd,c,strlen(c));
 //在解析cookie时
  char* cookievalue=cook_lookup(from);
  char* decryp=cookievalue;
  int i;
  for(i=0;;i++)
{
     //解密
     if(decryp[i]==0) break;
     decryp[i]=decryp[i]-1;
     decryp[i]=decryp[i]/2;
}
```