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

/* Char buffer holding messages for LED drvier */
#define BUF_LEN 20
const char* message_for_YELLOW_LED = "YELLOW";
const char* message_for_GREEN_LED   = "GREEN";
const char* message_for_RED_LED        = "RED";

 /* Mutex controlling servo movement critical section */
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

/* Flag to indicate when sensor has detected an object, used to restart the semaphore */
volatile uint8_t door_opened = 0x00;
volatile uint8_t cancel_timer = 0x00;
volatile uint8_t time_is_up    = 0x00;

/* Paths to char driver files */
const char* LED_DRIVER     = "/dev/led_driver";
const char* BUZZ_DRIVER  = "/dev/buzz_driver";
const char* ADC_DRIVER    = "/dev/adc_driver";

/* File descriptors for all driver files after opening */
int led_fd, buzz_fd, adc_fd;

/* Function that opens all device files and checks for errors */
int open_drivers(void);

/* SIGINT handler function, closes driver files */
void kill_handler(int signo, siginfo_t *info, void *context);

/*  Thread function reading data from ADC (sensor), comparing it to threshold value */
void* sensor_run(void* arg)
{
    char data[2];
	
	/* Odgovara udaljenosti od priblizno 75 cm */
	const uint16_t THRESHOLD = 0x0070;
	
    while(1)
	{
        read(adc_fd, data, 2);
		
		uint16_t result_data = (data[1] << 8) | data[0];  // Kombinovanje dva bajta u 16-bitni rezultat

        if(result_data < THRESHOLD)		// Detektovali smo otvaranje ulaznih vrata
		{
            door_opened = 0xff;
			pthread_exit(NULL);
        }
    }
}

/*  Thread function for timer*/
void* timer_run(void* arg)
{
	struct timespec req = {1, 0};  // 1 sekunda i 0 nanosekundi
	
    for (int i = 0; i < 30; i++) 
	{
		if (cancel_timer == 0xff)
			pthread_exit(NULL);
		
        nanosleep(&req, NULL);
    }
	
	time_is_up = 0xff;		// vrijeme je isteklo
	pthread_exit(NULL);
}

/* Main thread */
int main(void)
{
	/* Nit za ocitavanje sa senzora */
    pthread_t sensor_thread;
	/* Nit za tajmer*/
	pthread_t timer_thread;
	
	char sifra[10];
	
    struct sigaction act;
    memset(&act,0,sizeof(act));
    act.sa_sigaction = kill_handler;		/* SIGINT handler function, closes driver files */
    act.sa_flags        = SA_SIGINFO;
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
				else{	// Unesena neispravna sifra
					write(led_fd, message_for_RED_LED, BUF_LEN);
				}
			}
			
			if (cancel_timer == 0xff || time_is_up == 0xff) 
				break;
		}
		
    }
	pthread_join(sensor_thread, NULL);
	pthread_join(timer_thread, NULL);
    return 0;
}


int open_drivers(void)
{
    led_fd    = open(LED_DRIVER, O_RDWR);
    buzz_fd = open(BUZZ_DRIVER, O_RDWR);
    adc_fd   = open(ADC_DRIVER, O_RDWR);
	
    if ( led_fd < 0 || buzz_fd < 0 || adc_fd < 0)
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
        exit(1);
    }
}