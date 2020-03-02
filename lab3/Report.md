# Lab 3 Report

## Tests Passed
    1/ 1 tests/userprog/args-none
    1/ 1 tests/userprog/args-single
    1/ 1 tests/userprog/args-multiple
    1/ 1 tests/userprog/args-many
    1/ 1 tests/userprog/args-dbl-space

    - "create" system call.
    1/ 1 tests/userprog/create-empty

## Tests Failed

    - "create" system call.
    ** 0/ 1 tests/userprog/create-long
    ** 0/ 1 tests/userprog/create-normal
    ** 0/ 1 tests/userprog/create-exists

    - "open" system call.
    ** 0/ 1 tests/userprog/open-missing
    ** 0/ 1 tests/userprog/open-normal
    ** 0/ 1 tests/userprog/open-twice

    - "read" system call.
    ** 0/ 1 tests/userprog/read-normal
    ** 0/ 1 tests/userprog/read-zero

    - "write" system call.
    ** 0/ 1 tests/userprog/write-normal
    ** 0/ 1 tests/userprog/write-zero

    - "close" system call.
    ** 0/ 1 tests/userprog/close-normal

    - "exec" system call.
    ** 0/ 1 tests/userprog/exec-once
    ** 0/ 1 tests/userprog/exec-multiple

    - "wait" system call.
    ** 0/ 1 tests/userprog/wait-simple
    ** 0/ 1 tests/userprog/wait-twice

## Rationale de Design

First I'll talk about how I solved the main arguemnt problem...

`start_process()`
  
  - Here we make a copy of the commands given
  - Extract the filename
  - Try to load
  - Push commands onto the stack (more on this later)
  - We also have a `sema` here and we do a Up operation

`process_execute()`

  - Again,  we make a copy of the commands given
  - Again, extract the filename
  - init our `sema`
  - create a thread named filename
  - Do a down op on the same `sema`

We have this `sema` here to prevent the process from finishing before the thread finishes.

This next part is the buulk of the code:

`push_command()`

  1) Break up our cmd input into tokens `str_tok()`
  2) Store these tokens
  3) Now work backwords and loop through these tokens
     1) Here is where we first update `esp`
        1) we add strlen + 1 here for a `/0`  char
     2) we copy the token to this `esp`
  4) We word-align for efficency
  5) We end the argv array with no data
  6) Now we store/copy over the addy of the args
  7) write the # of args
  8) write a fake return
  9) done!

`create-empty`
 - For this we added a switch case
 - create a handler/sys
 - create_handler
   - we read from the itr frame
   - we store the result of calling our sys
 - sys_create
   - check the args passed in
   - here we have a `sema` to make atomic operations
   - We check the staus and return the correct one

---
Joseph Csoti - CSE 130 Winter 2020