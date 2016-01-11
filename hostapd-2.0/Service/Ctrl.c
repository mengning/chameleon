#include "Ctrl.h"

//lyc function begin
static struct wpa_ctrl *ctrl_conn;
static int hostapd_cli_attached=0;
static const char *ctrl_iface_dir = "/var/run/hostapd";
static char *ctrl_ifname = NULL;
static int ping_interval = 5;

static void hostapd_cli_msg_cb(char *msg,size_t len){
	printf("lyc,hostapd_cli_msg_cb msg%s\n",msg);
}
static int _wpa_ctrl_command(struct wpa_ctrl *ctrl, char *cmd, int print)
{
    char buf[4096];
    size_t len;
    int ret;
    
    if (ctrl_conn == NULL) {
        printf("Not connected to hostapd - command dropped.\n");
        return -1;
    }
    len = sizeof(buf) - 1;
    ret = wpa_ctrl_request(ctrl, cmd, strlen(cmd), buf, &len,
                           hostapd_cli_msg_cb);
    if (ret == -2) {
        printf("'%s' command timed out.\n", cmd);
        return -2;
    } else if (ret < 0) {
        printf("'%s' command failed.\n", cmd);
        return -1;
    }
    
    buf[len] = '\0';
    printf("command buf:%s\n", buf);
    return 0;
}
static struct wpa_ctrl * open_connection(const char *ifname)
{
    char *cfile;
    int flen;
 
    if (ifname == NULL) return NULL;
    
    flen = strlen(ctrl_iface_dir) + strlen(ifname) + 2;
    cfile = malloc(flen);
    if (cfile == NULL)  return NULL;
    //open connection
    snprintf(cfile, flen, "%s/%s", ctrl_iface_dir, ifname);
    printf("lyc:cfile:%s\n",cfile);
    ctrl_conn = wpa_ctrl_open(cfile);
    free(cfile);
    return ctrl_conn;
}
static void hostapd_cli_eloop_terminate(int sig,void *signal_ctx)
{
    eloop_terminate();
}
static void hostapd_cli_interactive(void)
{
	const int max_args = 10;
	char cmd[256],*res,*argv[max_args],*pos;
	int argc;
	printf("\nInteractive mod\n\n");
    eloop_register_signal_terminate(hostapd_cli_eloop_terminate,NULL);
        
}

static int hostapd_wps_config(struct wpa_ctrl *ctrl,const char *ssidname,char *auths,char *encr,char *psw){
	int ret;
	int i;
    char buf[4096];
	size_t len;
	char ssid_hex[2*32+1];//at most 64 length
	char key_hex[2*64+1];
	//char *cmd="wps_config changessid WPA2PSK CCMP 12345678";
/*	char auths2[]="WPA2PSK\0";
	char encr2[]="CCMP\0";
	char psw2[]="12345678\0";
    if(auths==NULL){
        auths=(char *)malloc(sizeof(char)*7+1);
        strncpy(auths,auths2,8);
    }
    if(encr==NULL){
        encr=(char *)malloc(sizeof(char)*4+1);
        strncpy(encr,encr2,5);
    }*/
    if(psw==NULL){
        psw=(char *)malloc(sizeof(char)*8+1);
    //    strncpy(psw,psw2,9);
    }
	if(strlen(ssidname)>32){
		printf("lyc config ssidname:'%s' to long\n",ssidname);
	//    free(auths);
    //    free(encr);
        free(psw);
        return -1;
	}
    printf("ssidname:%s\n",ssidname);
    //printf("auth:%s\n",auths);
    //printf("encr:%s\n",encr);
    printf("psw:%s\n",psw);
	ssid_hex[0]='\0';
	for(i=0;i<32;i++){
		if(ssidname[i]=='\0')
			break;
		os_snprintf(&ssid_hex[i*2],3,"%02x",ssidname[i]);
	}

//改密码 
	key_hex[0]='\0';
	for(i=0;i<64;i++){
		if(psw[i]=='\0')
			break;
		os_snprintf(&key_hex[i*2],3,"%02x",psw[i]);
	}


//	snprintf(buf,sizeof(buf),"WPS_CONFIG %s %s %s %s",ssid_hex,auths,encr,key_hex);
	snprintf(buf,sizeof(buf),"WPS_CONFIG %s %s",ssidname,psw);
// 	  free(auths);
//    free(encr);
//    free(psw);

    return _wpa_ctrl_command(ctrl,buf,1);
}
static void close_connection(void){
	    //close_cnnection
    if(ctrl_conn == NULL)
        return;
    if(hostapd_cli_attached){
    	wpa_ctrl_detach(ctrl_conn);
    	hostapd_cli_attached = 0;
    }
    wpa_ctrl_close(ctrl_conn);
    ctrl_conn = NULL;
}
static void haha_cleanup(void){
	close_connection();
    //deinit    
    os_program_deinit();
}
static void hostapd_cli_action_process(char *msg, size_t len)
{
    const char *pos;

    pos = msg;
    if (*pos == '<') {
        pos = os_strchr(pos, '>');
        if (pos)
            pos++;
        else
            pos = msg;
    }

    //hostapd_cli_exec(action_file, ctrl_ifname, pos);
}
static void hostapd_cli_recv_pending(struct wpa_ctrl *ctrl, int in_read,
                                     int action_monitor)
{
    int first = 1;
    if (ctrl_conn == NULL)
        return;
    while (wpa_ctrl_pending(ctrl)) {
        char buf[256];
        size_t len = sizeof(buf) - 1;
        if (wpa_ctrl_recv(ctrl, buf, &len) == 0) {
            buf[len] = '\0';
            if (action_monitor)//默认monitor为0
                hostapd_cli_action_process(buf, len);
            else {
                if (in_read && first)
                    printf("\n");
                first = 0;
                printf("%s\n", buf);
            }
        } else {
            printf("Could not read pending message.\n");
            break;
        }
    }
}

static void hostapd_cli_terminate(int sig){
	haha_cleanup();
	exit(0);
}
static void hostapd_cli_alarm(int sig)
{
    
    if (ctrl_conn && _wpa_ctrl_command(ctrl_conn, "PING", 0)) {
        printf("Connection to hostapd lost - trying to reconnect\n");
        close_connection();
    }
    if (!ctrl_conn) {
        ctrl_conn = open_connection(ctrl_ifname);
        if (ctrl_conn) {
            printf("Connection to hostapd re-established\n");
            if (wpa_ctrl_attach(ctrl_conn) == 0) {
                hostapd_cli_attached = 1;
            } else {
                printf("Warning: Failed to attach to "
                       "hostapd.\n");
            }
        }
    }
    if (ctrl_conn)
        hostapd_cli_recv_pending(ctrl_conn, 1, 0);
    alarm(ping_interval);
}
static u8 my[]={0x54,0xe4,0x3a,0x3d,0x63,0xc7};
int checkmac(const u8 *macaddr,const size_t len){
    int i=0,j=0;
    int flag=0;
    /*printf("mac check raw frame\n");
    for(i=0;i<len;i++){
        printf("%02x ",macaddr[i]);
    }
    printf("\n");*/
    for(i=0,j=0;i<len;i++){
        printf("i:%x j:%x ",macaddr[i],my[j]);
        if((unsigned short)macaddr[i]==(unsigned short)my[j]){
            j++;
            if(j==6){
                flag=1;
                break;
            }
        }else{
            j=0;
                
        }
    }
    printf("\nflag:%d\n",flag);
    return flag;
}
int Ctrl(const char *ssidname,const size_t len,char *auths,char *encr,char *psw){
    
	printf("\nlyc:macaddr len:%lu\n",len);
    int i,j;
    
    for(i=0;i<len;i++){
        printf("%02x ",ssidname[i]);
    }
//    printf("\nmac:addr without 02x\n");
//    for(i=0,j=0;i<len;i++){
//        printf("%x ",macaddr[i]);
//    }
//    printf("\n");
    int interactive=0;
    if(os_program_init())	return -1;
    
    if(eloop_init())return -1;
    //Connect
    for(;;)
    {   
        if(ctrl_ifname==NULL){
            struct dirent *dent;
            DIR *dir = opendir(ctrl_iface_dir);
            if(dir){
                while((dent=readdir(dir))){
                    if(os_strcmp(dent->d_name,".")==0
                       ||
                       os_strcmp(dent->d_name,"..")==0)
                       continue;
                printf("Select interface '%s'\n",dent->d_name);
                ctrl_ifname=os_strdup(dent->d_name);
                break;
                }
                closedir(dir);
            }
        }
        ctrl_conn=open_connection(ctrl_ifname);
        if(ctrl_conn){
            printf("lyc Ctrl Connection established.\n");
            break;
        }
        //else 
        printf("lyc Ctrl Could not connect to hostapd and re-tryiing\n");
        os_sleep(4,0);
        continue;
    }
    //通过給ctrl_conn传输数据，attach用来收消息
    //注册事件监听

    //signal
    //signal(SIGINT, hostapd_cli_terminate);
    //signal(SIGTERM, hostapd_cli_terminate);
    //signal(SIGALRM, hostapd_cli_alarm);

    //interactive 为1  交互信息
    
    //hostapd_cli_interactive();
    
    //attach 接收消息
    if(wpa_ctrl_attach(ctrl_conn)==0){
    	hostapd_cli_attached=1;
        printf("lyc Ctrl attach succeed\n");
    }else{
        printf("lyc Failed to attach to hostapd.\n");
    }

    //if（interactive）
    //	hostapd_cli_interactive();
    //else
//        hostapd_wps_config(ctrl_conn,"chagessid","WPA2PSK","CCMP","12345678");
		hostapd_wps_config(ctrl_conn, ssidname, auths, encr, psw);
//wps_config haha WPA2PSK CCMP 12345678
    os_free(ctrl_ifname);
    haha_cleanup();
    //exit(0);
}
