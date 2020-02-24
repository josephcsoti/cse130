
#include "cartman.h"

#include <pthread.h>
#include <stdio.h>

// Help hold cart data
struct CartData {
  unsigned int cart;
  enum track track;
  enum junction junction;
};

// Mutexs for 1st and 2nd junction
pthread_mutex_t mx;
pthread_mutex_t mx_2;

// Condvars and "bool" arrays
pthread_cond_t junction_cv[5];
int junctions[5];

// Func def.
void* thread_arrive(void* data);
enum junction sister_junction(enum track track, enum junction junction);

/*
 * You need to implement this function, see cartman.h for details 
 */
void arrive(unsigned int cart, enum track track, enum junction junction) 
{

  // Allocate new thread on stack
  pthread_t thread;

  // Create obj to pass in arguements
  struct CartData data = {cart, track, junction};

  // Create new thread
  pthread_create(&thread, NULL, thread_arrive, (void* ) &data);

  // Wait 'til it ends
  pthread_join(thread, NULL);

  return;
}

/*
 * You need to implement this function, see cartman.h for details 
 */
void depart(unsigned int cart, enum track track, enum junction junction) 
{

  // find its sister junction
  enum junction sister_junc = sister_junction(track, junction);

  //printf("Cart %d: departing! releaseing %d and %d...\n", cart, junction, sister_junc);

  // Release both juncs
  release(cart, junction);
  release(cart, sister_junc);

  // Update bool arr
  junctions[junction] = 0;
  junctions[sister_junc] = 0;

  // Signal only ONE thread that BOTH junctions are open
  pthread_cond_signal(&junction_cv[junction]);
  pthread_cond_signal(&junction_cv[sister_junc]);

  //printf("Cart %d: released! %d and %d...\n", cart, junction, sister_junc);

  return;
}

/*
 * You need to implement this function, see cartman.h for details 
 */
void cartman() 
{
  //Init mutex
  pthread_mutex_init(&mx, NULL);
  pthread_mutex_init(&mx_2, NULL);

  // init the cvs
  for(int i=0; i<5; i++){
    pthread_cond_init(&junction_cv[i], NULL);
  }

}

void* thread_arrive(void* data){

  // Convert
  struct CartData* cd = (struct CartData *) data;

  // Extract details
  unsigned int cart = cd->cart;
  enum track track = cd->track;
  enum junction junction = cd->junction;

  //printf(">> Cart %d: Arrived @ track:%d junc:%d\n", cart, track, junction);

  // Find sister junction
  enum junction sister_junc = sister_junction(track, junction);

  //printf("Cart %d: Trying junc %d & %d\n", cart, junction, sister_junc);

  // Create lock for 1st junc
  pthread_mutex_lock(&mx);

  //printf("Cart %d: (1/2) Waiting on %d...\n", cart, junction);
  
  // If taken, wait
  while(junctions[junction])
    pthread_cond_wait(&junction_cv[junction], &mx); // Wait for 1st junction

  // Mark junc as taken
  junctions[junction] = 1;

  pthread_mutex_unlock(&mx);

  // Create lock for 2nd junc
  pthread_mutex_lock(&mx_2);

  //printf("Cart %d: (2/2) Waiting on %d...\n", cart, sister_junc);

  // if taken, wait
  while(junctions[sister_junc])
    pthread_cond_wait(&junction_cv[sister_junc], &mx_2); // Wait for 2nd junction

  // Mark junc as taken
  junctions[sister_junc] = 1;

  pthread_mutex_unlock(&mx_2);

  // Both junctions are free so reserve
  //printf("Cart %d: Both junctions free, reserving...\n", cart);
  reserve(cart, junction);
  reserve(cart, sister_junc);

  // cross at junction
  printf("Cart %d: reserved! crossing...\n", cart);
  cross(cart, track, junction); 

  return 0;
}

/*
 * Return the "sister" junction of a track/junction
 */
enum junction sister_junction(enum track track, enum junction junction){
   switch(track){
    case Red:    return 0 + (junction + 1)%2;  // 0 & 1 --> 1 & 0
    case Green:  return 1 + (junction + 0)%2;  // 1 & 2 --> 2 & 1
    case Blue:   return 2 + (junction - 1)%2;  // 2 & 3 --> 3 & 2
    case Yellow: return 3 + (junction - 2)%2;  // 3 & 4 --> 4 & 3
    case Black:  return junction == 0 ? 4 : 0;  // 4 & 0 --> 0 & 4
  }

  //Something went REALLY wrong but
  return 999;
}