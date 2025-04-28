//* Dr. Malinowski's demo Web Server Daemon for Docker Container on port 88 *
// v. 2024 with corrected enforced security and updated to the current C++ standard
#include "tnplib.h"

#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <pwd.h>        // getpwnam(const char*)
#include <grp.h>        // getgrnam(const char*)
#include <unistd.h>     // setuid(.), setgid(.)

#include <iostream>
using namespace std;

#define _DEBUG 1


inline void D(const char *errormessage) {
#ifdef _DEBUG
    cerr << errormessage << endl;
#endif
}

const unsigned short    SERVER_PORT_NR = 88;
const unsigned short    SERVER_QLEN    = 10;
const char              SERVER_FILE_NM[] = "/var/log/http-counter/counter.txt";
const char              SERVER_UNAME[] = "counter";
const char              SERVER_GNAME[] = "counter";

static unsigned long    counter =  0;
static SOCKET           serve   = -1;


/* the signal handler function */
void handle_shutdown(int) {
    FILE* cntfile;
    D("INFO:  Stopping http-counter server");
    if (isValidSocket(serve)) {
        TCPStopServerTCP(serve);
        SocketLibEnd();
        D("INFO:  Server http-counter stopped");
    }
    D("INFO:  Saving the counter state");
    cntfile = fopen(SERVER_FILE_NM, "w");
    if (cntfile!=0) {
        int status = fprintf(cntfile,"%lu\n", counter);
        if (status<=0) D("ERROR: Unable to store the current count");
        fclose(cntfile);
    } else {
        D("ERROR: Unable to create a file to store the current count");
    }
    exit(0);
}

void SimpleHTTPServer(unsigned short int port, unsigned short int qlen, const char cntfname[]) {
    char buffer[16];    // buffer for converting numerical data to string
    char webpage[4096]; // make sure that the HTTP reply and the Web page fits inside
    int  status;
    FILE* cntfile;

    // set up graceful termination of the server socket
    {
        struct sigaction handler;
        handler.sa_handler = handle_shutdown; 
        handler.sa_flags = 0;
        sigemptyset(&handler.sa_mask);
        sigaction(SIGKILL, &handler, NULL);
        sigaction(SIGTERM, &handler, NULL);
        sigaction(SIGINT,  &handler, NULL);
    }

    SocketLibStart();

    // start the server on a port below 1024 as root
    D("INFO:  Starting http-counter server");
    serve = TCPStartServer(port, qlen, 1);
    if (!isValidSocket(serve)) {
        D("ERROR: Cannot start the server");
        D("STOP:  Am I running as root?");
        D("STOP:  Is another copy of the server running?");
        SocketLibEnd();
        exit(1);
    }
    D("Server http-counter started");

    // and now limit the server ability to act as a root
    // in case the service is attacked and compromised
    // first change the group as you still need to be root to do it
    struct group *grp = getgrnam(SERVER_GNAME);
    if (!grp || setgid(grp->gr_gid) <0) {
        D("ERROR: Server http-counter failed to change its group ID");
        D("STOP:  Am I running as root?");
        D("STOP:  Is the counter group set up?");
        SocketLibEnd();
        exit(1);
    } else {
        D("Server http-counter changed its group ID to counter");
    }
    // now give up the root privileges
    struct passwd *pwd = getpwnam(SERVER_UNAME);
    if (!pwd || setuid(pwd->pw_uid) <0) {
        D("ERROR: Server http-counter failed to change its user ID");
        D("STOP:  Am I running as root?");
        D("STOP:  Is the counter user set up?");
        SocketLibEnd();
        exit(1);
    } else {
        D("Server http-counter changed its user ID to counter");
    }



    cntfile = fopen(cntfname, "r");
    if (cntfile!=0) {
        status = fscanf(cntfile," %lu", &counter);
        if (status<=0) {
            D("INFO:  Unable to read the previous count");
            D("INFO:  Count restarts at 0");
            counter=0;
        } else {
            D("INFO:  Count value restored");
        }
        fclose(cntfile);
    } else {
        counter=0;
        D("INFO:  Count starts at 0");
    }

    D("INFO:  Daemon started");

    for (;;) {
        SOCKET visitor = TCPWaitForConnection(serve);
//      TCPSetNoDelay(visitor, true); // send each data chunk immediately

        // In this simple server we do not care about the request header
        // but may want to display it for demonstration and debugging purposes
        status = TCPRecvLine(visitor, buffer, sizeof(buffer) );
        if (status<0) continue;
        D(buffer);

        // In this simple server we do not care about the reminder of the request header
        do {
            status = TCPRecvDumpLine(visitor);
        } while (status>0);

        webpage[0]='\0';
        // HTTP Header
        strcat(webpage, "HTTP/1.1 200 Ok\r\n");
        strcat(webpage, "Server: AVR32 NGW Simple Demo Server\r\n");
        // Date: must be provided unless the server has no reliable clock - then it must not be provided
        strcat(webpage, "Connection: close\r\n");
        strcat(webpage, "Cache-Control: no-cache\r\nExpires: 0\r\nETag: \"\"\r\n");
        strcat(webpage, "Content-Type: text/html; charset=ISO-8859-1\r\n");
        strcat(webpage, "\r\n");

        // HTML Page header
        strcat(webpage, "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01//EN\">\r\n");
        strcat(webpage, "<html>\r\n<head>\r\n");
        strcat(webpage, "<meta http-equiv=\"Content-type\" content=\"text/html; charset=iso-8859-1\">\r\n");
        strcat(webpage, "<title>A simple dynamic Web page</title></head>\r\n<body>\r\n");

        strcat(webpage, "<p>"); 
        counter++; sprintf(buffer, "%lu", counter); strcat(webpage, buffer);
        strcat(webpage, "</p>\r\n");

        // HTML Page footer
        strcat(webpage, "</body>\r\n</html>\r\n");

        TCPSendLine(visitor, webpage);

        TCPPrepClose(visitor);
        TCPStopClient(visitor);
        D("INFO:  Counted ++");

        // Save the state every that many counts, in case power went off suddenly
        if (0==counter%1000) {
            cntfile = fopen(cntfname, "w");
            if (cntfile!=0) {
                status = fprintf(cntfile,"%lu\n", counter);
                fclose(cntfile);
                if (status<=0) {
                    D("ERROR: Updating persistent coutner failed!");
                }
            } else {
                D("ERROR: Updating persistent coutner failed!");
            }
        }
    } /* server loop */

    SocketLibEnd();
}



int main()
{
    SimpleHTTPServer(SERVER_PORT_NR, SERVER_QLEN, SERVER_FILE_NM);
    return(0);
}
