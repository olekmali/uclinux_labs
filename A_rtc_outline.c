//* Outline for your RTC access program for the Lab *
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <linux/rtc.h>

/*
struct rtc_time {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;     // unused
    int tm_yday;     // unused
    int tm_isdst;    // unused
};

ICTL commands:
#define RTC_RD_TIME     ...
#define RTC_SET_TIME    ...
interrupts (unsigned long frequency)
#define RTC_IRQP_READ   ...
#define RTC_IRQP_SET    ...
#define RTC_UIE_ON      ...
#define RTC_UIE_OFF     ...
reading from the file will block each time until interrupt run
alarms
#define RTC_ALM_READ    ...
#define RTC_ALM_SET     ...
#define RTC_AIE_ON      ...
#define RTC_AIE_OFF     ...
reading from the file will block each time until alarm run
and many more
...

See the example posted in the course notes for instruction how to implements the commands above

*/

#define RTC_DEV "/dev/clock"
// Note: revert it to the line below if you were not successful with UDEV part of the lab
// #define RTC_DEV "/dev/rtc0"

int main ()
{
    struct rtc_time     currentTime;
    unsigned long int   currentStatus;
    int                 rtc_fd;

    printf( "Hello, my name is __________\n\n" );


    rtc_fd = open(RTC_DEV, O_RDWR);
    if ( rtc_fd == -1 )
    {
        printf( "Couldn't open rtc\n" );
        return -1;
    }

    // read the time, can also set the time if you are root
    if ( ioctl( rtc_fd, RTC_RD_TIME, &currentTime ) == -1 )
    {
        printf( "Couldn't execute ioctl to read time on rtc\n" );
        return -1;
    }

    // use ioctl to set RTC_UIE_ON, NULL,
    // exit if an error - error will cause read(,,) to stall

    // read the sizeof(currentStatus) Bytes into the currentStatus variable using read(..,..,..)
    // Note: the program will wait until a clock interrupt occurs
    // and then returns a status returned from each serviced interrupt

    // use ioctl to set RTC_UIE_OFF, NULL
    // print error but do not exit on error

    // close the handle
    
    // ... print contents of the current time ...
    // ... print contents of the status and analyze ...

    return(0);
}