#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define dev_path  "/dev/mydev"

int main(void){
    int i;
    int fd;
    char ch;
    char *menu = "1. Search\n2. Report\n3. Exit\n\nPlease enter your mode:";
    char mode = '0';
    char region = '0';
    char change_mode = '0';
    char change_group = '0';
    char exit = 'q';
    int child[3] = {0};
    int adult[3] = {0};
    int elder[3] = {0};
    int totalnum[3] = {0};
    int signal[5]={0};  //index 4 is number of people; index 5 is two different mode: 0 is light; 1 is flashing
    int change_num = 0;
    fd = open(dev_path, O_RDWR);
    if (fd == -1) {
        printf("Failed to open device file\n");
        return -1;
    }
    while(1){
        printf("%s",menu);
        scanf(" %c",&mode);
        while(mode == '1'){
            if(totalnum[0] != 0){
                signal[0] = 1;
            }
            else if(totalnum[0] == 0){
                signal[0] = 0;
            }
            if(totalnum[1] != 0){
                signal[1] = 1;
            }
            else if(totalnum[1] == 0){
                signal[1] = 0;
            }
            if(totalnum[2] != 0){
                signal[2] = 1;
            }
            else if(totalnum[2] == 0){
                signal[2] = 0;
            }
            signal[3] = totalnum[0]+totalnum[1]+totalnum[2];
            signal[4] = 0;
            if(write(fd,signal,sizeof(signal))<0){
                printf("Write allregion_signal is not completed!\n");
                return -1;
            }
            printf("1. Baseball Stadium : %d\n2. Big City : %d\n3. Zoo : %d\n\n",totalnum[0],totalnum[1],totalnum[2]);
            printf("Please enter region or enter any 'q' to back:");
            scanf(" %c",&region);
            printf("\n");
            while(region == '1'){
                signal[0] = 1;
                signal[1] = 0;
                signal[2] = 0; 
                signal[3] = child[0]+adult[0]+elder[0];
                signal[4] = 1;
                if(write(fd,signal,sizeof(int)*5)<0){
                    printf("Write basketball_signal is not completed!\n");
                    return -1;
                }
                printf("Child : %d\nAdult : %d\nElder : %d\n\n",child[0],adult[0],elder[0]);
                printf("Press any key to back\n");
                scanf(" %c",&ch);
                if (getchar()){
                    region = '0';
                    break;
                }
            }
            while(region == '2'){

                signal[0] = 0;
                signal[1] = 1;
                signal[2] = 0; 
                signal[3] = child[1]+adult[1]+elder[1];
                signal[4] = 1;
                if(write(fd,signal,sizeof(signal))<0){
                    printf("Write bigcity_signal is not completed!\n");
                    return -1;
                }
                printf("Child : %d\nAdult : %d\nElder : %d\n\n",child[1],adult[1],elder[1]);
                printf("Press any key to back\n");
                scanf(" %c",&ch);
                if (getchar()){
                    region = '0';
                    break;
                }
            }
            while(region == '3'){
                signal[0] = 0;
                signal[1] = 0;
                signal[2] = 1; 
                signal[3] = child[2]+adult[2]+elder[2];
                if(write(fd,signal,sizeof(signal))<0){
                    printf("Write zoo_signal is not completed!\n");
                    return -1;
                }
                printf("Child : %d\nAdult : %d\nElder : %d\n\n",child[2],adult[2],elder[2]);
                printf("Press any key to back\n");
                scanf(" %c",&ch);
                if (getchar()){
                    region = '0';
                    break;
                }
            }
            while(region == 'q'){
                mode = '0';
                break;
            }
        }
        while(mode == '2'){
            if(totalnum[0] != 0){
                signal[0] = 1;
            }
            else if(totalnum[0] == 0){
                signal[0] = 0;
            }
            if(totalnum[1] != 0){
                signal[1] = 1;
            }
            else if(totalnum[1] == 0){
                signal[1] = 0;
            }
            if(totalnum[2] != 0){
                signal[2] = 1;
            }
            else if(totalnum[2] == 0){
                signal[2] = 0;
            }
            signal[3] = totalnum[0]+totalnum[1]+totalnum[2];
            signal[4] = 0;
            if(write(fd,signal,sizeof(signal))<0){
                printf("Write allregion_signal is not completed in report!\n");
                return -1;
            }
            printf("Area(1~3) : ");
            scanf(" %c",&region);
            printf("Add or Reduce ('a' or 'r') : ");
            scanf(" %c",&change_mode);
            printf("Age group ('c', 'a', 'e'): ");
            scanf(" %c",&change_group);
            printf("The number of passenger : ");
            scanf(" %d",&change_num);
            while(region == '1'){
                if(change_mode == 'a'){
                    if(change_group == 'c'){
                        child[0] = child[0]+change_num;
                    }
                    else if(change_group == 'a'){
                        adult[0] = adult[0]+change_num;
                    }
                    else if(change_group == 'e'){
                        elder[0] = elder[0]+change_num;
                    }
                }
                else if(change_mode == 'r'){
                    if(change_group == 'c'){
                        child[0] = child[0]-change_num;
                    }
                    else if(change_group == 'a'){
                        adult[0] = adult[0]-change_num;
                    }
                    else if(change_group == 'e'){
                        elder[0] = elder[0]-change_num;
                    }
                }
                totalnum[0] = child[0]+adult[0]+elder[0];
                signal[0] = 1;
                signal[1] = 0;
                signal[2] = 0; 
                signal[3] = totalnum[0];
                signal[4] = 0;
                if(write(fd,signal,sizeof(signal))<0){
                    printf("Write basketball_signal is not completed in report!\n");
                    return -1;
                }
                printf("\n");
                printf("Press 'e' to exit or Press 'c' to continue report\n");
                scanf(" %c",&ch);
                if(ch == 'c'){
                    region = '0';
                }
                else if(ch == 'e'){
                    region = '0';
                    mode = '0';
                }
            }
            while(region == '2'){
                if(change_mode == 'a'){
                    if(change_group == 'c'){
                        child[1] = child[1]+change_num;
                    }
                    else if(change_group == 'a'){
                        adult[1] = adult[1]+change_num;
                    }
                    else if(change_group == 'e'){
                        elder[1] = elder[1]+change_num;
                    }
                }
                else if(change_mode == 'r'){
                    if(change_group == 'c'){
                        child[1] = child[1]-change_num;
                    }
                    else if(change_group == 'a'){
                        adult[1] = adult[1]-change_num;
                    }
                    else if(change_group == 'e'){
                        elder[1] = elder[1]-change_num;
                    }
                }
                totalnum[1] = child[1]+adult[1]+elder[1];
                signal[0] = 0;
                signal[1] = 1;
                signal[2] = 0; 
                signal[3] = totalnum[1];
                signal[4] = 0;
                if(write(fd,signal,sizeof(signal))<0){
                    printf("Write bigcity_signal is not completed in report!\n");
                    return -1;
                }
                printf("\n");
                printf("Press 'e' to exit or Press 'c' to continue report\n");
                scanf(" %c",&ch);
                if(ch == 'c'){
                    region = '0';
                }
                else if(ch == 'e'){
                    region = '0';
                    mode = '0';
                }
            }
            while(region == '3'){
                if(change_mode == 'a'){
                    if(change_group == 'c'){
                        child[2] = child[2]+change_num;
                    }
                    else if(change_group == 'a'){
                        adult[2] = adult[2]+change_num;
                    }
                    else if(change_group == 'e'){
                        elder[2] = elder[2]+change_num;
                    }
                }
                else if(change_mode == 'r'){
                    if(change_group == 'c'){
                        child[2] = child[2]-change_num;
                    }
                    else if(change_group == 'a'){
                        adult[2] = adult[2]-change_num;
                    }
                    else if(change_group == 'e'){
                        elder[2] = elder[2]-change_num;
                    }
                }
                totalnum[2] = child[2]+adult[2]+elder[2];
                signal[0] = 0;
                signal[1] = 0;
                signal[2] = 1; 
                signal[3] = totalnum[2];
                signal[4] = 0;
                if(write(fd,signal,sizeof(signal))<0){
                    printf("Write zoo_signal is not completed in report!\n");
                    return -1;
                }
                printf("\n");
                printf("Press 'e' to exit or Press 'c' to continue report");
                scanf(" %c",&ch);
                if(ch == 'c'){
                    region = '0';
                }
                else if(ch == 'e'){
                    region = '0';
                    mode = '0';
                }
            }
        }
    }    
    return 0;
}