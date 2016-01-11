#include <stdio.h>
#include <unistd.h>
#include <string.h>
#define u8 unsigned char
//54:E4:3A:3D:63:C7
u8 macaddr[]={0124,0344,072,075,0143,0307};
//int  haha[]={84,224,58,75,43,7};
int main(){
    int i=0;
    for(i=0;i<6;i++){
        printf("O%o ",macaddr[i]);
    }
    printf("\n=======\n");
    for(i=0;i<6;i++){
        printf("0x%02x ",macaddr[i]);
    }
    printf("\n=======\n");
    printf("%d",'1');
}
