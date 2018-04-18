#include <stdio.h>  
#include <sys/types.h>  
#include <sys/stat.h>  
#include <fcntl.h>  
#include <unistd.h>   //sleep  
#include <poll.h>  
#include <signal.h>  
#include <fcntl.h>  
  
int fd;  
  
/* fifth_test 
 */   
int main(int argc ,char *argv[])  
{  
    int flag;  
    unsigned char val = 2;
    fd = open("/dev/buttons",O_RDWR);  
    if (fd < 0)  
    {  
        printf("open error\n");  
    }  
  
    write(fd, &val, 1);
 
    close(fd);  
    return 0;  
}  

