#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <string.h>

#define BUF_LEN 80

int main() {
    // Putanja do sysfs fajla
    const char *sysfs_path = "/sys/kernel/communicate/stop_timer";
    // Putanja do drajver fajla
    const char *device_path = "/dev/countdown_timer_driver";
    
    int filed;
    ssize_t ret;
    char buffer[BUF_LEN];
    filed = open(device_path, O_WRONLY);
    if(filed < 0)
    {
        perror("Failed to open device ... \n");
        return errno;
    }

        // Otvaramo fajl
        int fd = open(sysfs_path, O_WRONLY);
        if (fd < 0) 
        {
            perror("Greska prilikom otvaranja fajla. \n");
            return errno;
        }

        // Pisemo u fajl da zaustavimo tajmer
        const char *stop_command = "stop";
        const char *start_command = "start";
        strcpy(buffer, "start");
        
        ret = write(filed, buffer, strlen(buffer));
        printf("POkrenut tajmer. \n");
        /*if (write(fd, stop_command, sizeof(stop_command)) == -1) {
            perror("Greska prilikom pisanja u fajl\n");
            close(fd);
            return 1;
        }

        printf("Tajmer je zaustavljen.\n");
        * */
        
        // Zatvaramo fajl
        int val = read(filed, buffer, BUF_LEN);
        printf("value %d\n", val);
        /*while(read(filed, buffer, sizeof(buffer)) == 0)
        {
            printf("Nije zavrsio. \n");
        }
        printf("Tajmer je zaustavljen.\n");
*/
        close(fd);


    return 0;
}
