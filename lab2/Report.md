# CSE 130 - Lab 2 Report

## Results of Test:
```
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

SUMMARY BY TEST SET

Test Set                                      Pts Max  % Ttl  % Max
--------------------------------------------- --- --- ------ ------
tests/threads/Rubric.basic                     40/ 40  40.0%/ 40.0%
tests/threads/Rubric.advanced                   0/ 30   0.0%/ 30.0%
tests/threads/Rubric.stretch                    0/ 20   0.0%/ 20.0%
tests/threads/Rubric.extreme                    0/ 10   0.0%/ 10.0%
--------------------------------------------- --- --- ------ ------
Total                                                  40.0%/100.0%

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

SUMMARY OF INDIVIDUAL TESTS

Basic: Priority based ready queue (tests/threads/Rubric.basic):
            20/20 tests/threads/alarm-priority
            10/10 tests/threads/priority-preempt
             5/ 5 tests/threads/priority-change
             5/ 5 tests/threads/priority-fifo

        - Section summary.
              4/  4 tests passed
             40/ 40 points subtotal

Advanced: Priority waiting for locks, semaphores, and condition variables (tests/threads/Rubric.advanced):
          ** 0/15 tests/threads/priority-sema
          ** 0/15 tests/threads/priority-condvar

        - Section summary.
              0/  2 tests passed
              0/ 30 points subtotal

Stretch: Single level priority donation, including multiple donations (tests/threads/Rubric.stretch):
          ** 0/ 3 tests/threads/priority-donate-single
          ** 0/ 3 tests/threads/priority-donate-one
          ** 0/ 3 tests/threads/priority-donate-lower
          ** 0/ 3 tests/threads/priority-donate-multiple
          ** 0/ 2 tests/threads/priority-donate-multiple2
          ** 0/ 3 tests/threads/priority-donate-sema
          ** 0/ 3 tests/threads/priority-donate-condvar

        - Section summary.
              0/  7 tests passed
              0/ 20 points subtotal

Extreme: Nested and chained priority donation (tests/threads/Rubric.extreme):
          ** 0/ 5 tests/threads/priority-donate-nest
          ** 0/ 5 tests/threads/priority-donate-chain

        - Section summary.
              0/  2 tests passed
              0/ 10 points subtotal
```

## Git Diff
```
 lab2/pintos/src/threads/thread.c | 61 ++++++++++++++++++++++++++++++++++++----
 1 file changed, 56 insertions(+), 5 deletions(-)
```

## My Design

The core of my design has to do with how I insert items into the `ready_list` and check the prioirties.

Pintos orginally does this like:
```
list_push_back(&all_list, &t->allelem)
```

However this "appends" or attaches to the tail with no regard to prioirity.

So when pintos calls `next_thread_to_run()`, it will just pop off the `HEAD` so it just choose the threads in the order that they were recieved (Sort of a FIFO)
```
if (list_empty(&ready_list))
        return idle_thread;
    else
        return list_entry(list_pop_front(&ready_list), struct thread, elem);
```

Thus we need to fix this. There are two soultions that come to mind:

1. Search for the correct thread and return that to run
2. Insert the threads in the correct order and keep popping off the head

Option 1 may seem like it would work at first but it would get really messy really fast and probaly wont allow us to expand to add other features. So we should implement option 2

Let's note all the places we should do this "smart insert"

 - `thread_unblock(struct thread *t)`
 - `thread_yield(void)`
 - `init_thread(struct thread *t, const char *name, int priority)`

 
### thread_unblock
Here the thread now is unblocked and ready to run so we add it to the `ready_list`
```
void
thread_unblock(struct thread *t)
{
    ...

    list_insert_ordered(&ready_list, &t->elem, &compare_priority_func, NULL);

    ...
}
```

### thread_yeild
Here the current thread is going to yeild so we also add it to the `ready_list` to be picked up again
```
void
thread_yield(void)
{
    struct thread *cur = thread_current();

    ...
    
    if (cur != idle_thread) {
        list_insert_ordered(&ready_list, &cur->elem, &compare_priority_func, NULL);
    }

    ...

    schedule();

}
```

### init_thread
Here the thread is newly created and it has just started, so we add it to the `all_list`
```
static void
init_thread(struct thread *t, const char *name, int priority)
{
    ...

    // Set thread vars

    ...

    list_insert_ordered(&all_list, &t->allelem, &compare_priority_func, NULL);

    ...
}
```

### compare_priority_func

As you can see all these inserts also call a function called `compare_priority_func`. Lets take a look at what that does
```
/* Compare Thread A vs. Thread B in priority level*/
bool compare_priority_func(const struct list_elem *a, const struct list_elem *b, void *aux) {
  
  // Get values
  int a_priority = list_entry(a, struct thread, elem)->priority;
  int b_priority = list_entry(b, struct thread, elem)->priority;
  
  // is A > B?
  return a_priority > b_priority;
}
```

This function (a list-less func.) basically allows the insert code to know which thread has a higher priority, given A and B. Thus is can insert into the ready/all list in the correct order


## Priority Check
Now this alone wouldnt pass most of the tests unless we add one key feature: prioirity checks and changes

Here, our changes mostly affect
- `tid_t
thread_create(const char *name, int priority, thread_func *function, void *aux)`
- `void thread_set_priority(int new_priority)`

### thread_create
Here we have to do a simple check -- if the thread that we are creating has a higher priority than the current thread we should yeild immidety to let that other thread run. Thus we add a simple check at the end
```
tid_t
thread_create(const char *name, int priority, thread_func *function, void *aux)
{
    ...

    // Check if new thread has a higher p then the current thread's p
    if (thread_get_priority() < priority) thread_yield();

    return ...;
}
```

### thread_set_priority
This method mainly deals with changes in priority, either increase or a decrease
```
void
thread_set_priority(int new_priority)
{

    ...

    // update the priority
    struct thread *t_current = thread_current();
    t_current->priority = new_priority;

    if (!list_empty (&ready_list)) {
        // Get the head which should have the HIGHEST p
        struct thread *t_highest = list_entry(list_begin(&ready_list), struct thread, elem);
        
        // If the largest p is still higher than our new p, then pause the current thread
        if (t_highest->priority > new_priority) thread_yield();
    }

    ...
}
```
The main idea behind this approach is to:
1. Update the thread's p
2. Make sure the list isnt empty
3. Grab the head of the `ready_list` which should give us the thread w/ the highest p
4. Compare the head thread and the current thread
5. If this p is still higher that our current p, this thread should yeild

Once all this is implemented we should pass all the basic tests! Yay!

---
Written by Joseph Csoti (Feb. 2020)