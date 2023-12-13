#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#define LED_DEV_PATH "/dev/etx_device"

void LED_WRITE(int fd,char *char_number){
    int len;
    len=write(fd,char_number,strlen(char_number));
    if (len < 0){
        printf("Write is not completed!\n");
    }
}
int main(void){
    int i;
    int fd;
    char char_number[10];
    fd = open(LED_DEV_PATH,O_RDWR);
    printf("Please enter your student number: ");
    scanf("%s",char_number);
    LED_WRITE(fd,char_number);    
}