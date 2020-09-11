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
#include <string.h>
#include <sys/select.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "functions/functions.h"
#include "version.h"

static char *me;

/***
 * setup th syslog mechanism
 * verbosity
 */
void messagesetup(int verbose) {
    int flag = (LOG_CONS | LOG_PID);
    if (verbose)
        flag |= LOG_PERROR;

    openlog(NULL, flag, LOG_LPR);
}

/*
 *
 *
 *
 *
 */
int main(int ac, char** av) {

#   define 						P_MAX_LEN 				64

    int 						i;
    char 						printer[P_MAX_LEN];
    int 						data_master_fd = 		0;
    int 						cmd_master_fd = 		0;
    int 						print_fd = 				0;
    int							print_answer_send_to = -1;
    int 						verbose = 				1;
    int 						TestMode = 				0;
    int 						terminating =			0;
    int 						jetport = 				9100;
    int 						cmdport = 				9191;
    struct timeval tv;
    int 						client_socket = 		0;
    fd_set 						readfds;
    int							max_sd;

    me = av[0];

    strncpy(printer, "/dev/usb/lp0", P_MAX_LEN);


    if (parsearguments(
    		ac, av,
            &verbose,
            &jetport,
			&cmdport,
            &TestMode,
            printer, P_MAX_LEN)) {

        fprintf(stderr, "%s: terminating on argument errors.\n", me);
        syslog(LOG_ERR, "terminating on argument errors.");
        exit(1);
    }

    messagesetup(verbose);

    syslog(LOG_DEBUG, "=========================");
    syslog(LOG_INFO,  "%s version %d.%d build %s", me, MAJOR, MINOR, BUILD);

    if (TestMode) {
        syslog(LOG_INFO, "Testmode: I won't use a real printer");
    }

    if (verbose) {
        syslog(LOG_INFO, "verbosity level %d", verbose);

    	syslog(LOG_INFO, "jetdirect port %d", jetport);
        syslog(LOG_INFO, "command port   %d", cmdport);
        syslog(LOG_INFO, "using printer  \"%s\"", printer);
    }

    if (testprinter(printer, TestMode)) {
        syslog(LOG_ERR, "FATAL: cannot access printer %s", printer);
        exit(1);
    }


    if (init_data_listener(&data_master_fd, jetport)) {
        syslog(LOG_ERR, "FATAL: cannot start data listener");
        exit(1);
    } else if (verbose) {
        syslog(LOG_DEBUG,"data listener started");
    }

    if (init_cmd_listener(&cmd_master_fd, cmdport)) {
        syslog(LOG_ERR, "FATAL: cannot start command listener");
        exit(1);
    } else if (verbose) {
        syslog(LOG_DEBUG, "command listener started");
    }

    while (!terminating) {
        int activity;
        /*
         ***** select prepare
         */
        if (verbose)
            syslog(LOG_DEBUG,"mainloop top");

        FD_ZERO(&readfds);

        FD_SET(cmd_master_fd, &readfds);
        max_sd = cmd_master_fd;

        FD_SET(data_master_fd, &readfds);
        if (data_master_fd > max_sd)
        	max_sd = data_master_fd;

        if (print_fd != 0) {
            FD_SET(print_fd, &readfds);
            if (print_fd > max_sd)
            	max_sd = print_fd;
        }

        if (client_socket != 0) {
            FD_SET(client_socket, &readfds);
            if (client_socket > max_sd)
            	max_sd = client_socket;
        }


        max_sd = get_max_cmd_socket(max_sd, &readfds);

        tv.tv_sec = 300;
        tv.tv_usec = 0;
        if ( TestMode )
        	tv.tv_sec = 15;

        /*******************************************************
         * the great select
         */
        activity = select(max_sd + 1, &readfds, NULL, NULL, &tv);
        /*
         *******************************************************
         */

        if (activity == 0) {
        	if ( verbose )
        		syslog(LOG_DEBUG, "select timeout");
            continue;
        }
        if ((activity < 0) && (errno != EINTR)) {
            syslog(LOG_ERR, "select error: %m");
            terminating = 1;
            continue;
        }


        /**********************************************
         * new data socket
         **********************************************/
        if (FD_ISSET(data_master_fd, &readfds)) {
        	int rc = new_data_socket(verbose, data_master_fd, &client_socket );
        }
        /**********************************************
         * new command socket
         **********************************************/
        if (FD_ISSET(cmd_master_fd, &readfds)) {
        	int rc = new_command_socket(verbose, cmd_master_fd);
        }
        /**********************************************
         * process data
         **********************************************/
        if (FD_ISSET(client_socket, &readfds)) {
            if (copyTo(client_socket, print_fd, verbose, TestMode)) {
                if (verbose)
                    syslog(LOG_INFO, "copyTo returned 1 closing");
                if (client_socket) {
                    close(client_socket);
                    client_socket = 0;
                }
                if (print_fd) {
                    if (!TestMode) {
                        close(print_fd);
                        print_fd = 0;
                    }
                    print_answer_send_to = -1;
                }
            } else
                if (verbose)
                syslog(LOG_INFO, "copyTo ok");
        }
        /* 
         * is there a message from the printer?
         */
        if (FD_ISSET(print_fd, &readfds)) {
            char buf[16384];
            int l;

            if ((l = read(print_fd, buf, 16384 - 1)) < 0) {
                syslog(LOG_ERR, "printer sent us data and disappeared");
                close(print_fd);
                print_fd = 0;
                print_answer_send_to = -1;
            }
            if (l == 0) {
                if (verbose) {
                    syslog(LOG_INFO, "received 0 bytes from printer");
                }
#if 0
                if (!TestMode) {
                    close(print_fd);
                    print_fd = 0;
                }
                close(print_answer_send_to);
                if (client_socket != 0)
                    close(client_socket);
#endif
            } else {
                buf[l + 1] = '\0';
                if (print_answer_send_to != -1) {
                    if (verbose)
                        syslog(LOG_INFO,
                            "got %d bytes from Printer at %d sending to socket %d",
                            l, print_fd, print_answer_send_to);
                    if (verbose) {
                        syslog(LOG_DEBUG, "<%s> sent from %d to %d\n", buf,
                                print_fd, print_answer_send_to);
                    }
//                    if (verbose > 1) sysloghexdump(buf, l);
                    write(print_answer_send_to, buf, l);
                }
            }
        }
        /**********************************************
         * process command
         **********************************************/
        process_command(verbose, &readfds, &terminating);
        /**********************************************
         * error unknown select
         **********************************************/
        /**********************************************
         * 
         **********************************************/
        //terminating = 1;

    }
    /**********************************************
     * bye
     **********************************************/
    syslog(LOG_INFO, "program ends.");
    return (EXIT_SUCCESS);
}
