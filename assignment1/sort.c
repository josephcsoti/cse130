/*
 * Sources Used:
 * [0] https://stackoverflow.com/questions/21387349/how-does-the-fork-know-whether-it-is-in-child-process-and-in-parent-process
 * [1] https://stackoverflow.com/questions/32165540/copy-a-structure-to-a-shared-memory-in-c-using-posix-standard
 * [2] https://stackoverflow.com/questions/3902215/using-memcpy-to-copy-a-range-of-elements-from-an-array
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include "merge.h"

/* LEFT index and RIGHT index of the sub-array of ARR[] to be sorted */
void singleProcessMergeSort(int arr[], int left, int right) 
{
  if (left < right) {
    int middle = (left+right)/2;
    singleProcessMergeSort(arr, left, middle); 
    singleProcessMergeSort(arr, middle+1, right); 
    merge(arr, left, middle, right); 
  } 
}

/* 
 * Merges using mutiple processes
 */
void multiProcessMergeSort(int arr[], int left, int right) 
{

  if (left >= right) return;

  // define a midpoint
  int mid = (left+right)/2;

  // Generate a key for mem usage
  key_t key = ftok("jcsoti", 481516);

  // create shared memory key
  int sm_size = right + 1;
  int sm_key = shmget(key, sm_size*sizeof(int), IPC_CREAT);

  printf("created sm of size %d \n", sm_size);

  // attach to shared memory
  void *shared_mem = (int*)shmat(sm_key, 0, 0);
  
  printf("attached to sm \n");

  // copy right side of local memory into shared memory
  // Source [1] [2]
  printf("size:%d\n", sm_size);

  memcpy(shared_mem, arr, sizeof(int) * sm_size); // local -> shared

  printf("copied to sm w/ size %d \n", sm_size);

  // fork
  pid_t child = fork();

  printf("forked \n");

  // Source [0]
  // Process is a parent
  if(child != 0){

    printf("p is parent \n");

    // sort left side of local memory
    ////multiProcessMergeSort(arr, left, mid);
    singleProcessMergeSort(arr, left, mid)

    //wait for child to finish?
    wait(0);

    //copy shared memory to right side of local memory
    int dest_size = right - mid;
    int dest_start = mid;
    memcpy(arr+dest_start, shared_mem, sizeof(int) * dest_size); // shared -> local
    
    // detach from shared memory
    shmdt(shared_mem);
    
    // destroy shared memory
    shmctl(sm_key, IPC_RMID, 0);

    // merge ENTIRE local memory
    merge(arr, left, mid, right); 
  }
  // Proccess is a child
  else if(child == 0){

    printf("p is child \n");

    // attach to shared memory
    shared_mem = shmat(sm_key, 0, 0);

    //sort shared memory
    /////multiProcessMergeSort(shared_mem, left, right);

    // detach from shared memory
    shmdt(shared_mem);

    return;
  }
  // Error ???
  else {

    printf("p is error?");

    return;
  }

}
