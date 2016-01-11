#include <stdio.h>
#include <string.h>
static int command(char *resp){
    int idl=4;
    char *ssidname;
    char *auths;
    char *encr;
    char *psw;
    int idr=4;
    while(resp[idr]!='\0'){
        if(resp[idr]=='#')break;
        idr++;
    }
    ssidname=(char *)malloc(sizeof(char)*(idr-idl+2));
    strncpy(ssidname,resp+idl,10);
    
//    printf("idl:%d   idr:%d\n  ssid:%s\n",idl,idr,ssidname);
    
    idl=++idr;
    while(resp[idr]!='\0'){
        if(resp[idr]=='#')break;
        idr++;
    }
    auths=(char *)malloc(sizeof(char)*(idr-idl+2));
    strncpy(auths,resp+idl,(idr-idl));
    
    
    idl=++idr;
    while(resp[idr]!='\0'){
        if(resp[idr]=='#')break;
        idr++;
    }
    encr=(char *)malloc(sizeof(char)*(idr-idl+2));
    strncpy(encr,resp+idl,(idr-idl));
    
    
    idl=++idr;
    while(resp[idr]!='\0'){
        if(resp[idr]=='#')break;
        idr++;
    }
    psw=(char *)malloc(sizeof(char)*(idr-idl+2));
    strncpy(psw,resp+idl,(idr-idl));

    printf("ssidname:%s\n",ssidname);
    printf("auths:%s\n",auths);
    printf("encr:%s\n",encr);
    printf("psw:%s\n",psw);

//    Ctrl(ssidname,strlen(ssidname), auths, encr, psw);
    free(ssidname);
    free(auths);
    free(encr);
    free(psw);
    return 0;
}
int main(){
    command("Yes#changessid#WPA2PSK#CCMP#12345678#");
}

