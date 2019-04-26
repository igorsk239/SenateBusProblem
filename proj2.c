/*
 * File: proj2.c
 * Author: Igor Ignác xignac00@fit.vutbr.cz
 * Name: IOS Project 2 Senate bus problem
 * Created: 2017/2018
 * Faculty: Faculty of Information Technology, Brno University of Technology
*/
#include "proj2.h"

/* Globals */

FILE *file = NULL;

int action_number_identif,riders_waiting_indentif, active_rides_identif, proces_finished_identif, bus_actual_cap_identif, active_bus_identif, riders_waiting_to_enter_identif;
int* action_number, *riders_waiting, *active_rides, *proces_finished, *bus_actual_cap, *active_bus, *riders_waiting_to_enter;

sem_t *sem_action, *sem_rider_depatch, *sem_rider_wants_to_board, *sem_bus_on_stop, *sem_rider_boarding, *sem_rider_done;

/*
 * @brief reports error state during argumet parsing
*/
void err_mesg() {
  fprintf(stderr, "ERROR: ARGUEMNTS: Error occured when parsing file arguments\n"
                  "Run file with this sample ./proj2 R C ART ABT\n"
                  "R - number of riders A > 0\n"
                  "C - capacity of bus  C > 0\n"
                  "ART - maximal period of time in milliseconds which represents rider generation time\n"
                  "ABT - maximal period of time in milliseconds which represents bus generation time\n"
                  "Every param is whole number !\n");
  exit(RUNTIME_ERR);
}
void err_unlink() {
  fprintf(stderr, "ERROR : sem_unlink : an error occured during this operation\n");
  exit(RUNTIME_ERR);
}
void err_shmctl() {
  fprintf(stderr, "ERROR : shmctl : an error occured during this operation\n");
  exit(RUNTIME_ERR);
}
void err_close() {
  fprintf(stderr, "ERROR : sem_close : an error occured during this operation\n");
  exit(RUNTIME_ERR);
}
/*
 * @brief closing semaphores and freeing shared memory + file close !
*/
void free_sources() {

  /*
   * closing the named semaphore referred to by sem_
  */
  if((sem_close(sem_rider_wants_to_board)) == -1){err_close();}
  if((sem_close(sem_bus_on_stop)) == -1){err_close();}
  if((sem_close(sem_rider_boarding)) == -1){err_close();}
  if((sem_close(sem_action)) == -1){err_close();}
  if((sem_close(sem_rider_done)) == -1){err_close();}
  if((sem_close(sem_rider_depatch)) == -1){err_close();}

  /*
   * Marks the segment of shared memory to be destroyed
  */
  if(shmctl(action_number_identif, IPC_RMID, NULL) == -1 ){err_shmctl();}
  if(shmctl(riders_waiting_indentif, IPC_RMID, NULL) == -1){err_shmctl();}
  if(shmctl(active_rides_identif, IPC_RMID, NULL) == -1){err_shmctl();}
  if(shmctl(proces_finished_identif, IPC_RMID, NULL) == -1){err_shmctl();}
  if(shmctl(bus_actual_cap_identif, IPC_RMID, NULL) == -1){err_shmctl();}
  if(shmctl(riders_waiting_to_enter_identif, IPC_RMID, NULL) == -1){err_shmctl();}

  fclose(file); //closing file
}

int main(int argc, char const *argv[]) {
  int riders_numb, bus_capacity, rider_gen_time, bus_gen_time = 0;

  if (argc < 5) {
    err_mesg();
  }
  else if (argc > 5) {
    err_mesg();
  }

  riders_numb = strtol(argv[1], NULL, 10);
  bus_capacity = strtol(argv[2], NULL, 10);
  rider_gen_time = strtol(argv[3], NULL, 10);
  bus_gen_time = strtol(argv[4], NULL, 10);

  if(riders_numb <= 0) {
    fprintf(stderr, "ERROR: ARGUMENTS: Error occured when parsing arguments, argument: %d A > 0!!\n",riders_numb);
    exit(RUNTIME_ERR);
  }
  if(bus_capacity <= 0) {
    fprintf(stderr, "ERROR: ARGUMENTS: Error occured when parsing arguments, argument: %d C > 0!!\n",bus_capacity);
    exit(RUNTIME_ERR);
  }
  if(rider_gen_time < 0 || rider_gen_time > 1000) {
    fprintf(stderr, "ERROR: ARGUMENTS: Error occured when parsing arguments, argument: %d ART >= 0 && ART <= 1000!!\n", rider_gen_time);
    exit(RUNTIME_ERR);
  }
  if (bus_gen_time < 0 || bus_gen_time > 1000) {
    fprintf(stderr, "ERROR: ARGUMENTS: Error occured when parsing arguments, argument: %d ABT >= 0 && ABT <= 1000!!\n", bus_gen_time);
    exit(RUNTIME_ERR);
  }
  if((file = fopen("proj2.out", "w+")) == NULL) {
    fprintf(stderr, "ERROR: file doesn't exists and can not be created - permission denied\n");
    exit(RUNTIME_ERR);
  }
  setbuf(file, NULL); //clears out file after new execution

  /* Shared memory for synchronization */

  action_number_identif = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
  action_number = (int *)shmat(action_number_identif, NULL, 0);  //Index of action

  riders_waiting_indentif = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
  riders_waiting = (int *)shmat(riders_waiting_indentif, NULL, 0); //number of rides waiting on bus stop

  active_rides_identif = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
  active_rides = (int *)shmat(active_rides_identif, NULL, 0); //boarded riders waiting to bus end -> to be able to finish

  proces_finished_identif = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
  proces_finished = (int *)shmat(proces_finished_identif, NULL, 0); //counts number of riders that finished their ride

  bus_actual_cap_identif = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
  bus_actual_cap = (int *)shmat(bus_actual_cap_identif, NULL, 0);  //number of riders present in the bus

 /* repesents bus position:
  *   1 represents bus is on stop
  *   2 represents bus is travelling
 */
 active_bus_identif = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
 active_bus = (int *)shmat(active_bus_identif, NULL, 0);

 riders_waiting_to_enter_identif = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
 riders_waiting_to_enter = (int *)shmat(riders_waiting_to_enter_identif, NULL, 0); //number of riders processes that started and want to enter the stop

//  Shared memory variables inicialization
  *action_number = 1; //  A
  *riders_waiting = 0;//  CR
  *proces_finished = 0;
  *active_bus = 1;  //starting state
  *active_rides = 0;
  *riders_waiting_to_enter = 0;

  /* Semaphores initialization */

  sem_action = sem_open("sem_for_actions_0456", O_CREAT | O_EXCL, 0666, 1);
  sem_rider_depatch = sem_open("sem_for_finish_0457", O_CREAT | O_EXCL, 0666, 0);
  sem_rider_wants_to_board = sem_open("sem_for_waiting_0458", O_CREAT | O_EXCL, 0666, 0);
  sem_bus_on_stop = sem_open("sem_for_rider_done_0459", O_CREAT | O_EXCL, 0666, 0);
  sem_rider_boarding = sem_open("sem_for_riders_boarding_0461", O_CREAT | O_EXCL, 0666,0);
  sem_rider_done = sem_open("sem_for_0462", O_CREAT | O_EXCL, 0666, 0);


  pid_t bus_procces = fork(); //creating first main process
  if (bus_procces == -1 ) {
    fprintf(stderr, "ERROR: FORK : fork collapsed while creating main process\n");
    free_sources();
    exit(RUNTIME_ERR);
  }

  if (bus_procces == 0) {
    pid_t rider_procces = fork();
    if (rider_procces == -1 ) {
      fprintf(stderr, "ERROR: FORK : fork collapsed when creating process for riders\n");
      free_sources();
      exit(RUNTIME_ERR);
     }

    if(rider_procces == 0) {
        bus_procces = fork();
        if (bus_procces == -1) {
          fprintf(stderr, "ERROR: FORK : fork collapsed when creating process for bus\n");
          free_sources();
          exit(RUNTIME_ERR);
        }

        if(bus_procces == 0) {  //starting process bus
          sem_wait(sem_action);
          fprintf(file, "%d : BUS : start\n", *action_number);
          *action_number += 1;
          sem_post(sem_action);
          while(*proces_finished < riders_numb) { //cycle until we serve all riders

            sem_wait(sem_action); //to ensure noone will be able to write to file
            fprintf(file, "%d : BUS : arrival\n", *action_number);
            *action_number += 1;  //cycle counter for actions
            *active_bus = 1;  //bus is present on the stop
            sem_post(sem_action); //unlocks semaphore for writing to fiel to other process

            if(*riders_waiting > 0) { //someone wants to board
              sem_wait(sem_action);
              fprintf(file, "%d : BUS : start boarding: %d\n",*action_number, *riders_waiting );
              *action_number += 1;
              sem_post(sem_action);
              while(*riders_waiting > 0 && *bus_actual_cap < bus_capacity) {  //until bus is full or fullfiled its capacity
                sem_post(sem_rider_wants_to_board); //free one rider to board
                *bus_actual_cap +=1;  //capacity++
                sem_wait(sem_rider_boarding); //waiting for rider to board
              }
              sem_wait(sem_action);
              fprintf(file, "%d : BUS : end boarding: %d\n", *action_number, *riders_waiting ); //no one to serve/full bus -> end
              *action_number += 1;
              sem_post(sem_action);
            }
            sem_wait(sem_action);
            fprintf(file, "%d : BUS : depart\n", *action_number );
            *action_number += 1;
            *active_bus = 0;  // bus left the stop riders can come
            sem_post(sem_action);

            for (int i = 0; i < *riders_waiting_to_enter; i++) {
              sem_post(sem_bus_on_stop);  //freening riders who are waiting to enter the stop
            }

            //simulation of action for bus
            if(bus_gen_time > 0) {
              srand(time(NULL));
              int wait = rand() % bus_gen_time; //random value
              usleep(wait *1000); // *1000 because usleep works with milliseconds
            }

            sem_wait(sem_action);
            fprintf(file, "%d : BUS : end\n",*action_number );
            *action_number += 1;
            *active_bus = 1;  //bus is again present on the stop
            sem_post(sem_action);
            if(*active_rides != 0) {  //our bus wasn't empty
              for (int i = 0; i <= *active_rides; i++) {
                sem_post(sem_rider_done); //rider can finish after this semaphore increments it's value
                sem_wait(sem_rider_depatch);  //bus waits for rider to finish

              }
            }

            if(*proces_finished == riders_numb) { //all riders finished -> bus finish
              sem_wait(sem_action);
              fprintf(file, "%d : BUS : finish\n", *action_number );
              *action_number += 1;
              sem_post(sem_action);
            }
          }
        }
    } //rider wasn't created successfully
    else {
      for(int i = 0; i < riders_numb; i++) {

        rider_procces = fork(); //process rider created
        if (rider_procces == -1) {
          fprintf(stderr, "ERROR: FORK : fork collapsed when creating process for riders\n");
          free_sources();
          exit(RUNTIME_ERR);
        }

        if(rider_procces == 0) {

          if(rider_gen_time > 0) {  //generating rider
            srand(time(NULL));
            int wait = rand() % rider_gen_time;
            usleep(wait * 1000);
          }

          sem_wait(sem_action);
          fprintf(file, "%d : RID %d : start\n",*action_number, i+1);
          *riders_waiting_to_enter += 1;  //number of riders waiting to enter
          *action_number += 1;
          sem_post(sem_action);

          if (*active_bus == 1) { // bus on the stop -> riders have to wait
            sem_wait(sem_bus_on_stop);
          }

          sem_wait(sem_action);
          *riders_waiting += 1; //rider entered the stop
          fprintf(file, "%d : RID %d : enter: %d\n",*action_number, i+1 , *riders_waiting );
          *action_number += 1;
          *riders_waiting_to_enter -= 1;  //decrements counter for waiting riders(who want enter to the stop)
          sem_post(sem_action);

          sem_wait(sem_rider_wants_to_board); //wait until there is a bus to board to

          sem_wait(sem_action);

          fprintf(file, "%d : RID %d : boarding\n",*action_number, i+1 );
          *action_number += 1;
          *riders_waiting -= 1; //(riders waiting to board)--
          *active_rides += 1; //rider on the board
          sem_post(sem_action);
          sem_post(sem_rider_boarding); //rider ended boarding

          sem_wait(sem_rider_done);

          sem_wait(sem_action);
          fprintf(file, "%d : RID %d : finish\n",*action_number, i+1 );
          *action_number += 1;
          *active_rides -= 1; //rider got of the bus
          *proces_finished += 1;  //rider finished successfully
          *bus_actual_cap -= 1; //capacity--
          sem_post(sem_action);
          sem_post(sem_rider_depatch);

          break;

        }
      }
    }
  }
  else {
    wait(NULL);
    /*
     * removing named semaphores by their id
    */
    if(sem_unlink("sem_for_actions_0456") == -1){err_unlink();}
    if(sem_unlink("sem_for_finish_0457") == -1){err_unlink();}
    if(sem_unlink("sem_for_waiting_0458") == -1){err_unlink();}
    if(sem_unlink("sem_for_rider_done_0459") == -1){err_unlink();}
    if(sem_unlink("sem_for_riders_boarding_0461") == -1){err_unlink();}
    if(sem_unlink("sem_for_0462") == -1){err_unlink();}
    }

    free_sources(); //free shared memory and close semaphores
    exit(RUN_OK); // exit code on success
}
