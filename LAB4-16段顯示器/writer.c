#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#define LED_DEV_PATH "/dev/mydev"

void LED_WRITE(int fd,char *English_name){
    int len;
    for (int i = 0;i < strlen(English_name);i++){
        len=write(fd,&English_name[i],1);
        if (len < 0){
            printf("Write is not completed!\n");
        }
        sleep(1);
    }
}
int main(int argc, char *argv[]){
    int fd;
    if(argc != 2){
        fprintf(stderr,"Use ./writer <English name>");
    }
    fd = open(LED_DEV_PATH,O_RDWR);
    LED_WRITE(fd,argv[1]);    
}