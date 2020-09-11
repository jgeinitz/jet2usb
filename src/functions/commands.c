/*
 * commands.c
 *
 *  Created on: 11.09.2020
 *      Author: gei
 */

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <syslog.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>




//#include <commands.h>

static struct sockaddr_in			dataAddress;
#   define 						MAX_CLIENTS 			10
    int 						max_clients = 			MAX_CLIENTS;
    int 						cmd_socket[MAX_CLIENTS];


int get_max_cmd_socket(int old_max_sd, fd_set *readfds) {
	int i, sd, newmax;
	newmax = old_max_sd;
	for (i = 0; i < max_clients; i++) {
		sd = cmd_socket[i];
	    if (sd > 0)
	         FD_SET(sd, readfds);
	    // highest file descriptor number,
	    // need it for the select function
	    if (sd > newmax)
	         newmax = sd;
	}
	return newmax;

}

int init_cmd_listener(int *fds, int port) {

    int s = 1;

    if ((*fds = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        syslog(LOG_ERR, "cmd side - socket create failed: %m");
        return 1;
    }
    if (setsockopt(*fds, SOL_SOCKET, SO_REUSEADDR,
            (char *) &s, sizeof(s)) < 0) {
        syslog(LOG_ERR, "cmd side - set sockopt fail: %m");
    }
    dataAddress.sin_family = AF_INET;
    dataAddress.sin_addr.s_addr = /*INADDR_ANY*/ inet_addr("127.0.0.1");
    dataAddress.sin_port = htons(port);
    if (bind(*fds, (struct sockaddr *) &dataAddress, sizeof(dataAddress)) < 0) {
    	syslog(LOG_ERR, "cmd side - cannot bind: %m");
        return 1;
    }
    syslog(LOG_DEBUG, "cmd side - listening on %d", port);
    if (listen(*fds, 10) < 0) {
        syslog(LOG_ERR, "cmd side - cannot listen: %m");
        return 1;
    }
    return 0;
}

int talkTo(int sd, int *term, int verbose) {
    char buffer[255];
    int amount;
    int i;
    char *prompt = "100 Jet:\r\n";


    if ( verbose )
        syslog(LOG_DEBUG, "Command channel entered\n");

    for (i=0; i<255; i++)
        buffer[i]='\0';
    if ( (amount=read(sd,buffer,255)) < 0 ) {
        return 1;
    }
    if ( amount == 0 ) {
        if ( verbose ) {
            syslog(LOG_DEBUG,"Command channel EOF\n");
        }
        return 1;
    }
    if ( amount < 255 )
    	amount++;

    buffer[amount] = '\0';

    switch ( buffer[0] ) {
        case 'q': {
        	char *t = "200 Quit this command connection\r\n";
        	(void) write(sd,t,strlen(t));
            return 1;
        }
        case 't': {
        	char *t = "200 Shutting down\r\n";
          	(void) write(sd,t,strlen(t));
            *term = 1;
            return 0;
        }
        default: {
            char *t = "201 Unknown command.\r\n201 Valid commands are:\r\n201 t    terminate program\r\n201 q    close command channel\r\n";
            (void)write(sd,t, strlen(t));
            //(void)write(sd, buffer, strlen(buffer));
        }
    }
    (void) write(sd, prompt, strlen(prompt));
    return 0;
}

void process_command(int verbose, fd_set *readfds, int *term) {
    int sd, i;
    for (i = 0; i < max_clients; i++) {
        sd = cmd_socket[i];
        if (FD_ISSET(sd, readfds)) {
            if (talkTo(sd, term, verbose)) {
                if (sd != 0) {
                    close(sd);
                    cmd_socket[i] = 0;
                }
            }
        }
    }

}


int new_command_socket(int verb, int master) {
    int new_socket;
    int addrlen;
    int i;

    addrlen = sizeof (dataAddress);
    if ((new_socket = accept(master,(struct sockaddr *) &dataAddress,
            &addrlen)) < 0) {
        syslog(LOG_ERR, "accept error: %m");
        return 1;
    } else if (verb) {
        syslog(LOG_DEBUG, "new command socket %d\n", new_socket);
    }
    for (i = 0; i < max_clients; i++) {
        //if position is empty
        if (cmd_socket[i] == 0) {
            cmd_socket[i] = new_socket;
            break;
        }
    }
    {
        char *hello = "201 Command interface started\n200 enter cmd\n";
        write(new_socket, hello, strlen(hello));
    }
    if (verb)
        syslog(LOG_INFO, "command socket %d entered at position %d\n",
            new_socket, i);

	return 0;
}
