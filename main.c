#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    // Putanja do sysfs fajla
    const char *sysfs_path = "/sys/kernel/communicate/stop_timer";
    
    // Otvaramo fajl
    int fd = open(sysfs_path, O_WRONLY);
    if (fd == -1) {
        perror("Greska prilikom otvaranja fajla");
        return 1;
    }
    
    // Pisemo u fajl da zaustavimo tajmer
    const char *stop_command = "stop";
    if (write(fd, stop_command, sizeof(stop_command)) == -1) {
        perror("Greska prilikom pisanja u fajl");
        close(fd);
        return 1;
    }
    
    printf("Tajmer je zaustavljen.\n");
    
    // Zatvaramo fajl
    close(fd);
    
    return 0;
}
