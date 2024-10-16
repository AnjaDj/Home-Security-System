#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>


static pid_t pid = 0;

void takePic (char* filename)
{
        /*ako je povratna vrijednost fork()=0, nalazimo se u child procesu*/
        if((pid = fork()) == 0){
                /* Redirecting stdout and stderr into /dev/null so information of taking picture 
                   is not displayed in terminal*/ 
                int fd = open("/dev/null", O_WRONLY);
                if(fd != -1)
		{
		    dup2(fd, STDOUT_FILENO);
		    dup2(fd, STDERR_FILENO);
                    close(fd);
                    
                }
	    
                execl("/usr/bin/rpicam-still",
                        "/usr/bin/rpicam-still",
                        "-n",
                        "-o",
                        filename,
                        NULL);
                //Ako execl ne uspije, exit sa greskom
                _exit(1);
        }

        /*ako je povratna vrijednost fork() pozitivno, znaci da se nalazimo
        u parent procesu i povratna vrijednost je PID child procesa
        */ 
        else if (pid > 0){
                int status;
                waitpid(pid, &status, 0); // Cekaj da child proces zavrsi
        }
        else{
        // Ako fork ne uspije
        perror("Fork failed");
        }
}


