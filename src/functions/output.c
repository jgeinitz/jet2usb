/*
 * output.c
 *
 *  Created on: 11.09.2020
 *      Author: gei
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <unistd.h>
#include <string.h>

static char pname[256];

int testprinter(char *p, int *printerfd, int tst) {

    if ( tst ) {
        syslog(LOG_DEBUG, "testprinter: simulated printer is ok\n");
        return 0;
    }
    strncpy(pname, p, 256);
    if ( (*printerfd=open(p,O_RDWR )) == -1 ) {
    	syslog(LOG_ERR,"cannot open %s \"%m\"", p);
        return 1;
    }
    return 0;
}

#define BUFSIZE 16384
int copyfrom(int *print_fd, int *datasocket, int verbose) {
	char buf[BUFSIZE];
	int l;

	if ((l = read(*print_fd, buf, BUFSIZE - 1)) < 0) {
		syslog(LOG_ERR, "printer sent data and disappeared");
		close(*print_fd);
		*print_fd = 0;
	}
	if (l == 0) {
		if (verbose) {
			syslog(LOG_INFO, "received 0 bytes from printer");
		}
		close(*print_fd);
		if ( (*print_fd=open(pname,O_RDWR)) == -1 ) {
			*print_fd = 0;
		}
	} else {
		buf[l + 1] = '\0';
		if (verbose)
			syslog(LOG_INFO,
					"got %d bytes from Printer at %d sending to socket %d",
					l, *print_fd, *datasocket);
        if ( (verbose & 64) ) {
            int i;
            fprintf(stderr,"+---++++\n");
            for (i=0; i< l; i++ ) {
                fprintf(stderr, "%02X %c ", (unsigned short int)buf[i], (unsigned char) (buf[i] & 0xff) );
            }
            fprintf(stderr,"\n++++----\n");
            fflush(stderr);
        }

		write(*datasocket, buf, l);
	}
	if (*print_fd == 0)
		return 1;
	return 0;
}
