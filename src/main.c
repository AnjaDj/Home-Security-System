/****************************************************************************
 * Copyright (C) 2024 by Anja Djakovic                                      *
 *                                                                          *
 * This file is part of Home-Security-System                                *
 *                                                                          *
 *   Home-Security-System is free solution: you can redistribute it and/or  *
 *   modify it.                                                             *
 *   Home-Security-System is distributed in the hope that it will be useful *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                   *
 *                                                                          *
 *   See the README.md and WikiPage for more details.                       *
 *                                                                          *
 ****************************************************************************/
/**
 * @file main.c
 * @author Anja Djakovic
 * @date 11 Oct 2024
 * @brief File containing main user-space program for Home-Security-System
 *
 * This .c file is user-space program  which represents the bridge between
 * user and hardware devices such as ADC, BUZZER, LEDS, TIMER. It consists
 * of main thread, thread for reading ADC data, thread for tracking timer. 
 * @see https://github.com/AnjaDj/Home-Security-System/wiki
 */
 
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

volatile uint8_t door_opened = 0x00;   /** Flag to indicate when sensor has detected an object  - door is opened  */
volatile uint8_t cancel_timer= 0x00;   /** Flag to indicate user has stopped the timer - correct password entered */
volatile uint8_t time_is_up  = 0x00;   /** Flag to indicate timer has counted 30secs and correct password wasnt entered */

/** Paths to char driver files */
const char* LED_DRIVER   = "/dev/LED_driver";
const char* BUZZ_DRIVER  = "/dev/BUZZ_driver";
const char* ADC_DRIVER   = "/dev/ADC_driver";
const char* TIMER_DRIVER = "/dev/TIMER_driver";

/** File descriptors for all driver files after opening */
int led_fd, buzz_fd, adc_fd, timer_fd;

/** Function that opens all device files and checks for errors */
int open_drivers(void);

/** SIGINT handler function, closes driver files */
void kill_handler(int signo, siginfo_t *info, void *context);

/**  Thread function reading data from ADC (sensor), comparing it to threshold value */
void* sensor_run(void* arg)
{
    char data[4];
    const uint32_t THRESHOLD = 0x00000401;
	
    while(1)
    {
        read(adc_fd, data, 4);
	    usleep(50000);
	    //printf("Reading data = %x %x %x %x\n", data[3],data[2],data[1],data[0]);
		
	    uint32_t result_data = data[3]<<24 | data[2]<<16 | data[1]<<8 | data[0] ;
	
	    printf("Data from distance sensor = %x\n",result_data);

        /** door is opened */
        if(result_data > THRESHOLD)
	    {
            door_opened = 0xff;
	        pthread_exit(NULL);
        }
    }
}

static char status[1];
ssize_t ret_val;

/**  Thread function for timer*/
void* timer_run(void* arg)
{
    write(timer_fd, message_for_TIMER_start, strlen("start"));
    usleep(50000);
    
    while(time_is_up != 0xff && cancel_timer != 0xff)
    {
	    ret_val = read(timer_fd, status, 1);
	
	    if(status[0]==0x00000000) time_is_up   = 0xff;    /** 30secs have expired. Timer has counted down to 0 */
	    if(status[0]==0x00000010) cancel_timer = 0xff;    /** Timer is stopped by user. Timer hasnt exceeded 0 */
	
	    usleep(50000);
    }
    pthread_exit(NULL);
}

/** Main thread */
int main(void)
{
    pthread_t sensor_thread;
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

    pthread_create(&sensor_thread, NULL, sensor_run, NULL);     /** Creates and runs the sensor thread */

    while(1){
    
        /* Someone has entered the room, activate security procedure */
	    if (door_opened == 0xff){
			
	    write(led_fd, message_for_YELLOW_LED, BUF_LEN);
	    pthread_create(&timer_thread, NULL, timer_run, NULL);  /** Creates and runs the timer thread */
	
	    while(time_is_up != 0xff)
	    {
				/* Enter the password */
				printf("Unesite sifru: ");
				scanf("%s",sifra);
				
				if( (strcmp(sifra,PASSWORD) == 0) && (time_is_up != 0xff) ){        /** correct password on time */
					write(led_fd, message_for_GREEN_LED, BUF_LEN);
					cancel_timer = 0xff;
					break;
				}
				else if ((strcmp(sifra,PASSWORD) != 0) && (time_is_up != 0xff)){    /** wrong password on time */
					write(buzz_fd, message_for_BUZZER, BUF_LEN);
					write(led_fd, message_for_RED_LED, BUF_LEN);
				}
	    }
	    printf("time_is_up   = %x\n",time_is_up);
	    printf("cancel_timer = %x\n",cancel_timer);
	    
	    if (cancel_timer == 0xff){ 
		    write(timer_fd, message_for_TIMER_stop, strlen("stop"));
		    break;
	    }
	    
	    if (time_is_up == 0xff)
	    {
	        write(buzz_fd, message_for_BUZZER, strlen(message_for_BUZZER));
		    write(led_fd, message_for_RED_LED, BUF_LEN);
		
		    /** snapshoot - run camera */
		    int ret = system("./camera_run.sh");
		
		    if (ret < 0){
		        printf("Greska prilikom pozivanja skripte camera_run.sh\n");
		    }else{
		        printf("Skripta camera_run.sh pozvana\n");
		    }
		
		    break;
	    }
	}
		
    }
	pthread_join(sensor_thread, NULL);
	pthread_join(timer_thread, NULL);
	printf("main exit\n");
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
