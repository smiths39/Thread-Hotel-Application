## Description

The application required the use of a heap. The heap was created to allow first orderings of any data type, in this case being time. 
Each node within the heap consisted of an alarm time (omitting room number – as documented in problems.txt). 
Each alarm time was sorted in relation to the earliest time generated. 

The guest thread, generates wake up calls and adds them to the heap.
The method that performs the actions, acquires the mutex and inserts the generated time into the heap. 
The time is placed in a node and is sorted correctly within the heap. 
The pending alarms is increased to track the current amount of alarms generated.
As the operation is complete, the mutex is unlocked.

The waiter thread is initialised and acquires the mutex lock. 
Its purpose is to wait on alarms previously generated to expire. 
It halts any operations being performed until the time positioned at the top of the heap has arrived. 
The use of ETIMEOUT is significantly important as it indicates the generated future time equals the current Linux time. We retrieve the top node of the heap and transfer the data extracted into a data type of time. The wake up message is printed to the screen and the current values of alarms are modified. All operations performed are completed and the mutex is unlocked. The operation of both threads is then looped.

The operation of both threads continue until the user has pressed Ctrl-C. 
A signal handler is installed in preparation for that occurrence. 
Once pressed, a signal is sent to the signal handler who performs all cancellation procedures on both threads. 
Each thread's cancellation is set to 'disabled' once all functionality has completed and it is safe to cancel. 
Cleanup handlers are installed in each threads, which free's any resource previously obtained in the programs execution. 
Once each thread's mutex is unlocked, the cleanup handlers are removed and the cancellation, called from the signal handler, is acted upon. 
