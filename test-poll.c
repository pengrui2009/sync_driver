#include <stdio.h>  
#include <sys/types.h>  
#include <sys/stat.h>  
#include <fcntl.h>  
#include <unistd.h>   //sleep  
#include <poll.h>  
#include <signal.h>  
#include <fcntl.h>  
  
int fd;  
  
void mysignal_fun(int signum)  
{  
    unsigned char key_val;  
    read(fd,&key_val,1);  
    printf("key_val = 0x%x\n",key_val);  
}  
  
  
/* fifth_test 
 */   
int main(int argc ,char *argv[])  
{  
    int flag; 
    int result = 0; 
    fd_set fds;
    //signal(SIGIO,mysignal_fun);  
  
    fd = open("/dev/buttons",O_RDWR);  
    if (fd < 0)  
    {  
        printf("open error\n");  
    }  
  
    while(1)  
    {  
        struct timeval timeout = {3, 0};
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        
        /* 为了测试，主函数里，什么也不做 */  
        result = select(fd +1, &fds, &fds, NULL, &timeout);
	if(result)
        {
            read(fd, &flag, 1);
            printf("flag:%d\n", flag);
        }
        printf("result:%d\n", result);
    }  
    return 0;  
}  

