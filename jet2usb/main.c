/* 
 * File:   main.c
 * Author: Juergen Geinitz
 *
 * License: GPL
 * 
 * Started on 25. August 2018, 07:15
 */

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <sys/select.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "utilbox.h"

char *me;

/*
 * 
 */
int main(int ac, char** av) {

#define P_MAX_LEN 64
    me = av[0];

    int                 i;
    char                printer[P_MAX_LEN];
    int                 data_master_fd  = 0;
    int                 cmd_master_fd   = 0;
    int                 print_fd        = 0;
    int                 verbose         = 0;
    int                 TestMode        = 0;
    int                 terminating     = 0;
    int                 jetport         = 9100;
    int                 cmdport         = 9191;
    struct sockaddr_in6 dataAddress;
    struct sockaddr_in  cmdAddress;
    struct timeval      tv;
#define MAX_CLIENTS 10
    int                 max_clients     = MAX_CLIENTS;
    int                 client_socket;
    int                 cmd_socket[MAX_CLIENTS];
    fd_set              readfds;
    int                 max_sd;

    {
        int flag = (LOG_CONS | LOG_PID);
        if (verbose)
            flag |= LOG_PERROR;
        openlog(NULL, flag, LOG_LPR);
    }
    syslog(LOG_INFO, "starting");
    strncpy(printer, "/dev/usb/lp0", P_MAX_LEN);


    if (parsearguments(ac, av,
            &verbose,
            &jetport, &cmdport,
            &TestMode,
            printer, P_MAX_LEN)) {
        
        fprintf(stderr, "%s: terminating on argument errors.\n", me);
        syslog(LOG_ERR, "terminating on argument errors.");
        exit(1);
    }

    if (TestMode) {
        printf("Testmode: I won't use a real printer\n");
    }

    if (verbose > 0) {
        syslog(LOG_INFO, "jetdirect port is %d", jetport);
        syslog(LOG_INFO, "command port is %d", cmdport);
        syslog(LOG_INFO, "using printer \"%s\"", printer);
    }

    client_socket = 0;

    for (i = 0; i < MAX_CLIENTS; i++)
        cmd_socket[i] = 0;

    if (testprinter(printer, TestMode)) {
        syslog(LOG_ERR, "cannot access printer %s EXIT", printer);
        return 1;
    }

    /* fetch hp printer data from printer (toner,...)
     **********************************************/

    if (init_data_listener(&data_master_fd, jetport, &dataAddress)) {
        syslog(LOG_ERR, "cannot start data listener EXIT");
        return 1;
    } else if (verbose > 8) {
        printf("data listener started\n");
    }

    if (init_cmd_listener(&cmd_master_fd, cmdport, &cmdAddress)) {
        syslog(LOG_ERR, "cannot start command listener EXIT");
        return 1;
    } else if (verbose > 8 ) {
        printf("command listener started\n");
    }

    while (!terminating) {
        int activity;
        int sd;
        /*
         ***** select prepare
         */
        if (verbose > 8)
            printf("mainloop top\n");

        FD_ZERO(&readfds);

        FD_SET(cmd_master_fd, &readfds);
        max_sd = cmd_master_fd;

        FD_SET(data_master_fd, &readfds);
        if (data_master_fd > max_sd) max_sd = data_master_fd;

        if (print_fd != 0) {
            FD_SET(print_fd, &readfds);
            if (print_fd > max_sd) max_sd = print_fd;
        }

        if (client_socket != 0) {
            FD_SET(client_socket, &readfds);
            if (client_socket > max_sd) max_sd = client_socket;
        }

        for (i = 0; i < max_clients; i++) {
            sd = cmd_socket[i];
            if (sd > 0)
                FD_SET(sd, &readfds);
            // highest file descriptor number,
            // need it for the select function
            if (sd > max_sd)
                max_sd = sd;
        }

        tv.tv_sec = 300;
        tv.tv_usec = 0;

        activity = select(max_sd + 1, &readfds, NULL, NULL, &tv);
        if (activity == 0) {
            syslog(LOG_DEBUG, "select timeout");
            continue;
        }
        if ((activity < 0) && (errno != EINTR)) {
            char *emsg = strerror(errno);
            syslog(LOG_ERR, "select error: %s", emsg);
        }

        /**********************************************
         * new data socket
         **********************************************/
        if (FD_ISSET(data_master_fd, &readfds)) {
            int addrlen;

            if (verbose > 8)
                printf("new data socket request\n");

            if (client_socket != 0) {
                syslog(LOG_ERR, "can only handle one connection");
            } else {
                addrlen = sizeof (dataAddress);
                if ((client_socket = accept(data_master_fd, (struct sockaddr *) &dataAddress, &addrlen)) < 0) {
                    char *emsg = strerror(errno);
                    syslog(LOG_ERR, "accept: %s", emsg);
                } else {
                    FILE *prt;
                    if (verbose) {
                        syslog(LOG_INFO, "new data socket %d\n", client_socket);
                    }
                    if ( (prt=fopen(printer,"a+"))== NULL ) {
                        syslog(LOG_ERR,"Printer open error %m");
                        exit (1);
                    }
                    print_fd = fileno(prt);
                }
                resetGuesser();
            }
        }
        /**********************************************
         * new command socket
         **********************************************/
        if (FD_ISSET(cmd_master_fd, &readfds)) {
            int new_socket;
            int addrlen;
            int i;

            addrlen = sizeof (cmdAddress);
            if ((new_socket = accept(cmd_master_fd, (struct sockaddr *) &cmdAddress, &addrlen)) < 0) {
                char *emsg = strerror(errno);
                syslog(LOG_ERR, "accept: %s", emsg);
                return 1;
            } else if (verbose > 8 ) {
                printf("new command socket %d\n", new_socket);
            }
            for (i = 0; i < max_clients; i++) {
                //if position is empty
                if (cmd_socket[i] == 0) {
                    cmd_socket[i] = new_socket;
                    break;
                }
            }
            if ( verbose ) 
                syslog(LOG_INFO,"command socket %d entered at position %d\n",
                    new_socket, i);
        }
        /**********************************************
         * process data
         **********************************************/
        if ( FD_ISSET(client_socket, &readfds)) {
        if (copyTo(client_socket, print_fd, verbose, TestMode)) {
            if (verbose)
                syslog(LOG_INFO, "copyTo returned 1 closing");
            if (client_socket) {
                close(client_socket);
                client_socket = 0;
            }
            if (print_fd) {
                close(print_fd);
                print_fd = 0;
            }
        }
        }
        /**********************************************
         * process command
         **********************************************/
        for (i = 0; i < max_clients; i++) {
            sd = cmd_socket[i];
            if (FD_ISSET(sd, &readfds)) {
                if (talkTo(sd, &terminating, verbose)) {
                    if ( sd != 0 ) {
                    close(sd);
                    cmd_socket[i] = 0;
                    }
                }
            }
        }
        /**********************************************
         * error unknown select
         **********************************************/
        /**********************************************
         * 
         **********************************************/
        //terminating = 1;

    }
    /**********************************************
     * 
     **********************************************/

    syslog(LOG_INFO, "program ends.");
    return (EXIT_SUCCESS);
}
