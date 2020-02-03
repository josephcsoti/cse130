#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "merge.h"

#define MIN_THREADS 4
#define MAX_THREADS 32

struct Data {
  int t_id;
  int segment_size;
  int *arr_p;
};

void* mt_mergesort(void* data);
void merge_h(int arr[], int left, int right);
pthread_t threads[4];

/* LEFT index and RIGHT index of the sub-array of ARR[] to be sorted */
void singleThreadedMergeSort(int arr[], int left, int right) {
  if (left < right) {
    int middle = (left + right) / 2;
    singleThreadedMergeSort(arr, left, middle);
    singleThreadedMergeSort(arr, middle + 1, right);
    merge(arr, left, middle, right);
  }
}

/*
 * MergeSort implemented using multiple threads
 */
void multiThreadedMergeSort(int arr[], int left, int right) {
  // Find thread number from array size
  int len = right + 1;
  int threads_to_use = 4;
  int segment_size = len / threads_to_use;

  // printf("Arr was %d, so threads=%d and seg_size:%d\n", len, threads_to_use, segment_size);

  // Allocate space to hold data
  struct Data data[threads_to_use];

  // Create apporriate number of threads
  for (int t_id = 0; t_id < threads_to_use; t_id++) {
    // printf("CREATED: T_%d\n", t_id);
    struct Data d = {t_id, segment_size, &arr[0]};
    data[t_id] = d;
    pthread_create(&threads[t_id], NULL, mt_mergesort, (void *) &data[t_id]);
  }

  // Join all the created threads, and wait for them to finish
  for (int t_id = 0; t_id < threads_to_use; t_id++) {
    // printf("JOINED: T_%d\n", t_id);
    pthread_join(threads[t_id], NULL);
  }

  // Merge the halves
  merge_h(arr, 0, right);

  return;
}

/*
 * Thread func to complete mergesort
 */
void* mt_mergesort(void* data) {
  struct Data* d = (struct Data *) data;

  int t_id = d->t_id;
  int segment_size = d->segment_size;
  int* arr = d->arr_p;

  // printf("got t_id:%d ss:%d\n", t_id, segment_size);

  int segment_start = segment_size * t_id;
  int segment_end = (segment_size * (t_id + 1)) - 1;

  // printf("mt_mergesort: start-%d end-%d\n", segment_start, segment_end);

  singleThreadedMergeSort(arr, segment_start, segment_end);


  if(t_id == 0){
    // wait for 2
    pthread_join(threads[1], NULL);
    merge_h(arr, 0, segment_start + 2*segment_size - 1);
  } 
  else if(t_id == 2){
    // wait for 4
    pthread_join(threads[3], NULL);
    merge_h(arr, segment_start, segment_start + 2*segment_size - 1);
  }

  return 0;
}

/*
 * Merge helper, so we dont have to pass in a midpoint everytime
 */
void merge_h(int arr[], int left, int right){
  // printf("Merging %d<=%d<=%d\n", left, (left+right)/2, right);
  merge(arr, left, (left+right)/2, right);
}