#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <stdint.h>

#define PASSWORD "1234"
#define BUF_LEN 20

const char* message_for_YELLOW_LED  = "YELLOW";
const char* message_for_GREEN_LED   = "GREEN";
const char* message_for_RED_LED     = "RED";
const char* message_for_BUZZER      = "10";
const char* message_for_TIMER_start = "start";
const char* message_for_TIMER_stop  = "stop";

 /* Mutex controlling servo movement critical section */
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

/* Flag to indicate when sensor has detected an object, used to restart the semaphore */
volatile uint8_t door_opened = 0x00;
volatile uint8_t cancel_timer= 0x00;
volatile uint8_t time_is_up  = 0x00;

/* Paths to char driver files */
const char* LED_DRIVER   = "/dev/LED_driver";
const char* BUZZ_DRIVER  = "/dev/BUZZ_driver";
const char* ADC_DRIVER   = "/dev/ADC_driver";
const char* TIMER_DRIVER = "/dev/TIMER_driver";

/* File descriptors for all driver files after opening */
int led_fd, buzz_fd, adc_fd, timer_fd;

/* Function that opens all device files and checks for errors */
int open_drivers(void);

/* SIGINT handler function, closes driver files */
void kill_handler(int signo, siginfo_t *info, void *context);

/*  Thread function reading data from ADC (sensor), comparing it to threshold value */
void* sensor_run(void* arg)
{
    char data[4];
	
    const uint32_t THRESHOLD =0x00000401;
	
    while(1)
    {
        read(adc_fd, data, 4);
	
	usleep(50000);
	
	//printf("Reading data = %x %x %x %x\n", data[3],data[2],data[1],data[0]);
		
	uint32_t result_data = data[3]<<24 | data[2]<<16 | data[1]<<8 | data[0] ; // Kombinovanje 4 bajta u 32-bitni rezultat
	
	printf("Data from distance sensor = %x\n",result_data);

        if(result_data > THRESHOLD)		// Detektovali smo otvaranje ulaznih vrata
	{
            door_opened = 0xff;
	    pthread_exit(NULL);
        }
    }
}

static char status[1];
ssize_t ret_val;

/*  Thread function for timer*/
void* timer_run(void* arg)
{
    write(timer_fd, message_for_TIMER_start, strlen("start"));
    usleep(50000);
    
    while(time_is_up != 0xff && cancel_timer != 0xff) //sve dok vrijeme nije isteklo ili sve dok timer nije cancelovan
    {
	ret_val = read(timer_fd, status, 1);
	
	if(status[0]==0x00000000) time_is_up = 0xff;    /* 30secs have expired. Timer has counted down to 0 */
	if(status[0]==0x00000010) cancel_timer = 0xff;  /* Timer is stopped by user. Timer hasnt exceeded 0 */
	
	usleep(50000);
    }
    pthread_exit(NULL);
}

/* Main thread */
int main(void)
{
    /* Nit za ocitavanje sa senzora */
    pthread_t sensor_thread;
    /* Nit za tajmer*/
    pthread_t timer_thread;
	
    char sifra[BUF_LEN]="";
	
    /* SIGINT handler function, closes driver files */
    struct sigaction act;
    memset(&act,0,sizeof(act));
    act.sa_sigaction = kill_handler;		
    act.sa_flags     = SA_SIGINFO;
    sigaction(SIGINT,&act,NULL);

    /* Opens all device files and checks for errors */
    if(open_drivers() < 0){
        perror("FATAL ERROR: Failed opening device files !!\n");
        return -1;
    }

    /* Kreiramo i pokrecemo nit koja je zaduzena za ocitavanje vrijednosti sa senzora */
    pthread_create(&sensor_thread, NULL, sensor_run, NULL);

    while(1){
        /* Neko je usao u prostoriju, pokrece se sigurnosna procedura */
	if (door_opened == 0xff){
			
	    write(led_fd, message_for_YELLOW_LED, BUF_LEN);
			
	    /* Kreiramo i pokrecemo tajmersku nit */
	    pthread_create(&timer_thread, NULL, timer_run, NULL);
	
	    /* dok vrijeme nije isteklo */
	    while(time_is_up != 0xff)
	    {
				/* Unos sifre */
				printf("Unesite sifru: ");
				scanf("%s",sifra);
				
				if( (strcmp(sifra,PASSWORD) == 0) && (time_is_up != 0xff) ){ // Ako je sifra ispravna i vrijeme nije isteklo
					write(led_fd, message_for_GREEN_LED, BUF_LEN);
					cancel_timer = 0xff;
					break;
				}
				else if ((strcmp(sifra,PASSWORD) != 0) && (time_is_up != 0xff)){	// Unesena neispravna sifra i vrijeme nije isteklo
					write(buzz_fd, message_for_BUZZER, BUF_LEN);
					write(led_fd, message_for_RED_LED, BUF_LEN);
				}
	    }
	    if (cancel_timer == 0xff){ 
		write(timer_fd, message_for_TIMER_stop, strlen("stop"));
		break;
	    }
	    
	    if (time_is_up == 0xff)
	    {
	        write(buzz_fd, message_for_BUZZER, strlen(message_for_BUZZER));
		write(led_fd, message_for_RED_LED, BUF_LEN);
		break;
	    }
	}
		
    }
	pthread_join(sensor_thread, NULL);
	pthread_join(timer_thread, NULL);
    return 0;
}


int open_drivers(void)
{
    led_fd   = open(LED_DRIVER, O_RDWR);
    buzz_fd  = open(BUZZ_DRIVER, O_RDWR);
    adc_fd   = open(ADC_DRIVER, O_RDWR);
    timer_fd = open(TIMER_DRIVER, O_RDWR);
	
    if ( led_fd < 0 || buzz_fd < 0 || adc_fd < 0 || timer_fd < 0)
    {
	printf("Error - driver not opened\n");
        return -1;
    }
    else 
	return 0;
}

void kill_handler(int signo, siginfo_t *info, void *context)
{
    if(signo == SIGINT)
	{
        close(led_fd);
        close(buzz_fd);
        close(adc_fd);
	close(timer_fd);
        exit(1);
    }
}
