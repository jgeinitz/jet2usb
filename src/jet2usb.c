/* 
 * File:   main.c
 * Author: Juergen Geinitz
 *
 * License: GPL
 * 
 * Started on 25. August 2018, 07:15
 */

#include "jet2usb.h"
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

void fatalexit() {
	fprintf(stderr,"ERROR EXIT -- see syslog\n");
	exit(1);
}
/*
 *
 *
 *
 *
 */
int main(int ac, char** av) {

#   define 						P_MAX_LEN 				64


    char 						printer[P_MAX_LEN];
    int 						data_master_fd = 		0;
    int 						cmd_master_fd = 		0;
    int 						print_fd = 				0;
    int 						verbose = 				0;
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
        exit(1);;
    }

    messagesetup(verbose);

    syslog(LOG_DEBUG, "=========================");
    syslog(LOG_INFO,  "%s version %d.%d build %s", me, MAJOR, MINOR, BUILD);

    if (TestMode) {
        syslog(LOG_INFO, "Testmode: I won't use a real printer");
    }

    if (verbose) {
        syslog(LOG_INFO, "verbose/debug level %d", verbose);

    	syslog(LOG_INFO, "jetdirect port %d", jetport);
        syslog(LOG_INFO, "command port   %d", cmdport);
        syslog(LOG_INFO, "using printer  \"%s\"", printer);
    }

    if (testprinter(printer, TestMode)) {
        syslog(LOG_ERR, "FATAL: cannot access printer %s", printer);
        fatalexit();
    }


    if (init_data_listener(&data_master_fd, jetport)) {
        syslog(LOG_ERR, "FATAL: cannot start data listener");
        fatalexit();
    } else if (verbose) {
        syslog(LOG_DEBUG,"data listener started");
    }

    if (init_cmd_listener(&cmd_master_fd, cmdport)) {
        syslog(LOG_ERR, "FATAL: cannot start command listener");
        fatalexit();
    } else if (verbose) {
        syslog(LOG_DEBUG, "command listener started");
    }

    while (!terminating) {
        int activity;

        if (verbose > 100) syslog(LOG_DEBUG,"mainloop top");

        /* prepare select */
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
        max_sd = get_max_cmd_socket(max_sd, &readfds);

        tv.tv_sec = 300;
        tv.tv_usec = 0;
        if ( TestMode ) tv.tv_sec = 15;

        /*******************************************************
         * the great select
         */
        activity = select(max_sd + 1, &readfds, NULL, NULL, &tv);
        /* look and process unusual states
         ******************************************************/
        if (activity == 0) {
        	if ( verbose > 100) syslog(LOG_DEBUG, "select timeout");
            continue;
        }
        if ((activity < 0) && (errno != EINTR)) {
            syslog(LOG_ERR, "select error: %m");
            terminating = 1;
            continue;
        }
        /**********************************************
         * new data channel requested - open it */
        if (FD_ISSET(data_master_fd, &readfds)) {
        	int rc = new_data_socket(verbose, data_master_fd, &client_socket );
        	if ( rc ) return rc;
        }
        /**********************************************
         * new command channel - open */
        if (FD_ISSET(cmd_master_fd, &readfds)) {
        	int rc = new_command_socket(verbose, cmd_master_fd);
        	if ( verbose ) syslog(LOG_DEBUG,"new command returned %d", rc);
        }
        /**********************************************
         * copy data from data socket to printer*/
        if (FD_ISSET(client_socket, &readfds)) {
        	int rc = copyTo(&client_socket, print_fd, verbose, TestMode);
        	if ( verbose ) syslog(LOG_DEBUG,"new data channel returned %d", rc);
        }
        /**********************************************
         * data from printer send it to data socket */
        if (FD_ISSET(print_fd, &readfds)) {
        	terminating = copyfrom(&print_fd, &client_socket, verbose);
        }
        /**********************************************
         * process command */
        process_command(verbose, &readfds, &terminating);
    } /* while ! terminating */
    /**********************************************
     * bye
     **********************************************/
    syslog(LOG_INFO, "program ends.");
    return (EXIT_SUCCESS);
}
