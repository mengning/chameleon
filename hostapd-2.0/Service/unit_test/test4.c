#include<stdio.h>
#include<fcntl.h>

int main(){
    FILE *x;
    x=fopen("txt","a+");
    fprintf(x,"1234\n");
}
