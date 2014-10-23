#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

#define ETIMEOUT 110
/*******************************************************************************
                            HEAP
*******************************************************************************/

/* max array size for heap */

#define MAXHEAP 100

/* DATA could be any record */
typedef int DATA;

typedef struct heap{
    DATA data[MAXHEAP];
    
    /* number if nodes within the heap */
    int size;           
} HEAP;

/* computes and returns i's parent index */
int parent(HEAP *h, int i)
{    
    if(i > h -> size || i < 1)
    {
        printf("Parent: impossible index %d\n", i);
        exit(1);
    }

    /* top has no parent */
    if(i == 1)
        return 0;
    else
        return i/2;
}

/* compares a and b: returns -1 if(a < b)
                     returns 0  if(a == b)
                     returns +1 if(a > b)
*/
int compare(DATA a, DATA b)
{    
    if(a < b)
        return -1;
    else if(a == b)
        return 0;
    else
        return 1;
}

/* computes and returns i's left child index */
int left(HEAP *h, int i)
{
    if(i > h -> size || i < 1)
    {
        printf("Left: impossible index %d\n", i);
        exit(1);
    }
    
    /* i on lowest layer has no left child */
    if(i * 2 > h -> size)
        return 0;
 
    /* return index of left child */   
    return i*2;
}

/* computes and returns i's right child index */
int right(HEAP *h, int i)
{
    if(i > h -> size || i < 1)
    {
        printf("Right: impossible index %d\n", i);
        exit(1);
    }
    
    /* i on lowest layer has no right child */
    if(i * 2+1 > h -> size)
        return 0;
 
    /* return index of right child */   
    return i*2+1;
}

/* inserts node into heap */
void insert(HEAP *h, DATA d)
{   
    int i, j;
    
    /* special case of empty heap */
    if(h -> size == 0)
    {
        h -> size++;
        h -> data[1] = d;
    }
    else
    {
        /* create hole at bottom of heap */
        h -> size++;
        i = h -> size;
        
        /* move hole to position where 'd' can be inserted */
        while(1)
        {
            j = i;
            i = parent(h,i);
            
            /* hole must now be at top so insert here */
            if(!i)
            {
                h -> data[j] = d;
                return;
            }
            
            /* heap OK so insert parent here */
            if(compare(d,h -> data[i]) >= 0)
            {
                h -> data[j] = d;
                return;
            }
            
            /* bubble parent value down, hole up */
            h -> data[j] = h -> data[i];
        }
    }
}

/* returns and removes top element from the heap */
DATA getTop(HEAP *h)
{
    DATA top, last;
    int i, j, k;
    
    if(h -> size == 0)
    {
        printf("getTop: empty heap\n");
        exit(1);
    }
    
    top = h -> data[1];
    
    /* special case for heap with only 1 Node */
    if(h -> size == 1)
    {
        h -> size--;
        return top;
    }
    else
    {
        /* unlink and store last node in heap */
        last = h -> data[h -> size--];
        i = 1;
        
        /* rearrange heap */
        while(1)
        {
            j = left(h,i);
            k = right(h,i);
            
            /* if 'i' has both left and right children */
            if(j && k)
            {   
                /* if heap is OK, then quit loop */
                if(compare(last, h -> data[j]) < 0 && compare(last, h -> data[k]) < 0)
                    break;
                 
                /* move hole down to left */
                if(compare(h -> data[j], h -> data[k]) <= 0)
                {
                    h -> data[i] = h -> data[j];
                    i = j;
                }
                else
                {
                    /* move hole down to right */   
                    h -> data[i] = h -> data[k];
                    i = k;
                }
            }
            /* 'i' has a left child but no right child */
            else if(j)
            {
                /* if heap is OK, then quit loop */
                if(compare(last, h -> data[j]) < 0)
                    break;
                
                /* move hole to left */
                h -> data[i] = h -> data[j];
                i = j;
            }
            /* Node 'i' now at bottom of heap */
            else
                break;
        }
        
        /* replace unlinked last Node in hole, now at lowest layer */
        h -> data[i] = last;
        return top;
    }
}


/*******************************************************************************
                            Global Data Types
*******************************************************************************/

HEAP heap;

/* initialises a static mutex and condition variable with default attributes */
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t condition = PTHREAD_COND_INITIALIZER;

/* representing the time currently being generated */
time_t generatedTime;

/* threads to generate and initialise wake up */
pthread_t guest_thread, waiter_thread;

/* alarms to keep track of wake ups */
int pendingAlarms = 0;
int expiredAlarms = 0;

/*******************************************************************************
                            Thread Handlers
*******************************************************************************/

void *guest_handler()
{   
    /* cancel is ignored until set to ENABLED */
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    /* when state is set to ENABLED, cancel will be immediately acted upon */
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    /* guest thread holds mutex and prevents access to shared data */
    pthread_mutex_lock(&mutex);
    
    /* pushes cancellation cleanup routine onto cleanup stack*/
    pthread_cleanup_push(free, &mutex);
    
    /* generate random time for room number and future wake up time */
    srand(time(NULL));         
    int randRoom = rand() % 9999;
    int addRand = rand() % 100;
    /* between current time and next 100 seconds */
    generatedTime = time(NULL) + addRand;
        
    /* insert time into heap */
    insert(&heap, (DATA)generatedTime);
       
    /* convert time value into a date and time string */
    char *register_time = ctime(&generatedTime);
    printf("%s%8d %s\n", "Register: ", randRoom, register_time);
 
    /* increase pending alarms */
    pendingAlarms++;

    /* unlock shared data */
    pthread_mutex_unlock(&mutex);
    
    /* remove cleanup handlers from cleanup stack */
    pthread_cleanup_pop(0);
    
    /* pending cancel is acted upon */
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

    return(NULL);
}

void *waiter_handler()
{
    /* cancel is ignored until set to ENABLED */
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    /* when state is set to ENABLED, cancel will be immediately acted upon */
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    /* waiter thread holds mutex and prevents access to shared data */
    pthread_mutex_lock(&mutex);
    
    /* pushes cancellation cleanup routine onto cleanup stack*/
    pthread_cleanup_push(free, &mutex);

    /* transfers time into seconds */
    struct timespec ts;
    ts.tv_sec = generatedTime;

    while(true)
    {
        /* blocks until the specified time of day has passed */
        int err = pthread_cond_timedwait(&condition, &mutex, &ts);
      
        /* if time has arrived, act upon the wake up */
        if(err == ETIMEOUT)
        {
            /* increase expired alarms, decrease pending alarms */
            expiredAlarms++;
            pendingAlarms--; 
            
            /* retrieve the top node of the heap (i.e. earliest time) */
            time_t wakeup_time = (DATA)getTop(&heap);
            
            /* print wake up time to screen */
            char *print_wakeup = ctime(&wakeup_time);
            (void) printf("%s%11s%s", "Wake up:","",print_wakeup);
            
            printf("\nExpired alarms:  %d\n", expiredAlarms);
            printf("Pending alarms:  %d\n\n", pendingAlarms);
            break;
        }
    }
    
    /* unlock shared data */
    pthread_mutex_unlock(&mutex);
    
    /* remove cleanup handlers from cleanup stack */
    pthread_cleanup_pop(0);
    
    /* pending cancel is acted upon */
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

}

void catch_signal()
{
    /* cancel guest thread */
    printf("\nThe guest thread is cleaning up...\n");
    pthread_cancel(guest_thread);
    printf("The guest thread says goodbye.\n");
    
    /* cancel waiter thread */
    printf("The waiter thread is cleaning up...\n");
    pthread_cancel(waiter_thread);
    printf("The waiter thread says goodbye.\n");  
    
    /* reset pending alarms */  
    pendingAlarms = 0;
    printf("%s%5s%d\n", "Pending alarms:", "",pendingAlarms);
    
    /* exit the program, due to all functionality performed */
    exit(0);    
}

/*******************************************************************************
                            Main Program
*******************************************************************************/

int main()
{
    srand(time(NULL));      
    
    /* if 'Ctrl-C' is pressed, send signal to signal handler */
    signal(SIGINT, catch_signal);    
    
    while(1)
    {
        /* create guest thread */
        pthread_create(&guest_thread, NULL, guest_handler, (void *)NULL); 
        
        /* sleep for random time between 1 and 5 seconds */
        int sleepTime = rand() % 5 + 1;
        sleep(sleepTime);  
        
        /* create waiter thread */
        pthread_create(&waiter_thread, NULL, waiter_handler, (void *)NULL);
    }

    return(0);
}
