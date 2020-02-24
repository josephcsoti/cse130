
/*********************************************************************
 *
 * Copyright (C) 2020 David C. Harrison. All right reserved.
 *
 * You may not use, distribute, publish, or modify this code without 
 * the express written permission of the copyright holder.
 *
 ***********************************************************************/
#include <pthread.h>
#include <stdio.h>
#include "manpage.h"

#define MAX_MANPAGE_PARTS 7

// List of condittions
pthread_mutex_t mx;
pthread_mutex_t mx_2;
pthread_cond_t cv[MAX_MANPAGE_PARTS];
pthread_t threads[MAX_MANPAGE_PARTS];

// thread func
void* thread_func_paragraph(void* data);

/*
 * See manpage.h for details.
 *
 * As supplied, shows random single messages.
 */
void manpage() 
{

  //Init mutex
  pthread_mutex_init(&mx, NULL);
  pthread_mutex_init(&mx_2, NULL);

  // init 7 cvs
  for(int i=0; i<MAX_MANPAGE_PARTS; i++){
    pthread_cond_init(&cv[i], NULL);
  }

  // create 7 threads  
  for(int i=0; i<MAX_MANPAGE_PARTS; i++){
    pthread_create(&threads[i], NULL, thread_func_paragraph, NULL);
  }

  // Join to prevent program from ending too early
  for(int i=0; i<MAX_MANPAGE_PARTS; i++){
    pthread_join(threads[i], NULL);
  }

  // DONE
  return;
}

void* thread_func_paragraph(void* data){

  int p_id = getParagraphId();

  // Aquire lock #1
  pthread_mutex_lock(&mx);

  // If not first, then wait for condvar
  if(p_id != 0){
    pthread_cond_wait(&cv[p_id], &mx); // Wait until condvar is changed
  }

  // Free lock #1
  pthread_mutex_unlock(&mx);

  // Aquire lock #2
  pthread_mutex_lock(&mx_2);
  
  showParagraph();
  
  // Signal next thread if n < nth - 1 thread
  if(p_id < MAX_MANPAGE_PARTS - 1){
    int next_id = p_id + 1;
    pthread_cond_signal(&cv[next_id]);
  }

  // Destroty current cond
  pthread_cond_destroy(&cv[p_id]);

  // Free lock #2
  pthread_mutex_unlock(&mx_2); 

  return 0;
}
