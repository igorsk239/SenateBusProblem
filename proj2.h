/*
 * File: proj2.h
 * Author: Igor Ignác xignac00@fit.vutbr.cz
 * Name: IOS Project 2 Senate bus problem
 * Created: 2017/2018
 * Faculty: Faculty of Information Technology, Brno University of Technology
*/

#ifndef H_PROJ2
#define H_PROJ2
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <semaphore.h>

#define RUN_OK 0
#define RUNTIME_ERR 1

/* Function prototypes */
void err_mesg();
void err_unlink();
void err_shmctl();
void err_close();
void free_sources();

#endif
