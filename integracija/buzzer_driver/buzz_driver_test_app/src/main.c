#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define BUF_LEN 80

int main()
{
    int file_desc;
    int ret_val;
    char *msg = "10";
    char tmp[BUF_LEN];

    /* Open /dev/buzzer_driver device. */
    file_desc = open("/dev/buzzer_driver", O_RDWR);

    if(file_desc < 0)
    {
        printf("'/dev/buzzer_driver' device isn't open\n");
        printf("Try:\t1) Check does '/dev/buzzer_driver' node exist\n\t2)'chmod 666 /dev/ \
               buzzer_driver'\n\t3) ""insmod"" memory module\n");

        return -1;
    }
    printf("'/dev/buzzer_driver' device is successfully opened!\n");

    /* Write to /dev/buzzer_driver device. */
    ret_val = write(file_desc, msg, strlen(msg));
    printf("Written message: %s", msg);
    printf("ret_val: %d\n", ret_val);

    /*  Close /dev/buzzer_driver device. */
    close(file_desc);
	return 0;
}
