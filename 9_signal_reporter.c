//* A demo to illustrate the usage of CRON *
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

// replace word YOURSIGNAL with a positive number of your choice except 9

/* the signal handler function */
void handle_YOURSIGNAL(int) { //1
        printf("User Signal Received by <insert_your_name_here>\n");
}

int main()
{
        int sleeptime = 0;
        struct sigaction handler;
        handler.sa_handler = handle_YOURSIGNAL; //2
        handler.sa_flags = 0;
        sigemptyset(&handler.sa_mask);
        sigaction(YOURSIGNAL, &handler, NULL); //3
        sleeptime = sleep(3600); // wait for the first signal received so that the interval can be measured
        while(1)
        {
            sleeptime = sleep(3600); //Sleep for an hour to prevent the program being loaded into memory when it is not needed.
            if(sleeptime!=0) //Program will sleep until it receives a signal. On signal sleep returns with remaining amount of time on sleep interval.
            {
                printf("You have configured Cron to trigger this program every %d seconds.\n",(3600-sleeptime));
            }
            else
            {
                printf("You do not have Cron configured to trigger this program or your trigger interval is greater than one hour.\n");
            }
            //Do Nothing
        }
    return(0);
}
