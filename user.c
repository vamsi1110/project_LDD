#include <stdio.h>
#include <fcntl.h>   //open
#include <unistd.h>  //read/write/lseek
#include <string.h>


int fd_sensor, oled_fd;  //file descriptors for kernel interfaces
 
void oled_display(char *msg)
{
    printf("[OLED]: %s\n", msg);
    write(oled_fd, msg, strlen(msg));
    usleep(500000);
}
int main()
{
    char buf[128];
    char oled_buf[32];

    fd_sensor = open("/sys/kernel/ultra/status", O_RDONLY);
    if (fd_sensor < 0) {
        perror("sensor open failed");
        return 1;
    }

    oled_fd = open("/dev/oled_file", O_WRONLY);
    if (oled_fd < 0) {
        perror("oled open failed");
        return 1;
    }

    while (1) {
        lseek(fd_sensor, 0, SEEK_SET);
        read(fd_sensor, buf, sizeof(buf));

        printf("Sensor Output: %s\n", buf);

        if (strstr(buf, "STOP")) {
            strcpy(oled_buf, "STOP");
        } else {
            strcpy(oled_buf, "CLEAR");
        }
        oled_display(buf);
       // write(fd_oled, oled_buf, strlen(oled_buf));
       char * msg="hi";
       //oled_display(msg);
        usleep(500000);
    }

    return 0;
}
