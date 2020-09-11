
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <getopt.h>
#include <syslog.h>
#include <errno.h>

#define TRUE 1

#if 0
static int typeval;
#define PS 1
#define PCL 0
#define UNSPEC -1

void resetGuesser() {
    typeval = UNSPEC;
}

void guessJob(char *buf, int max) {
    int i;
    
    if ( typeval != UNSPEC ) return;
    for ( i=0; (i< (max-1))&&(buf[i+1]!='\0') ; i++ ) {
        if ( (buf[i]== '%') && (buf[i+1]== '!')) {
            typeval = PS;
            return;
        }
    }
    typeval=PCL;
}

int isPS() {
    if ( typeval == PS )
        return 1;
    return 0;
}
#endif

int parsearguments(int ac, char **av,
        int *verbose,
        int *jetport,
        int *cmdport,
        int *TestMode,
        char *p, int p_max_len
    ) {
    
    void usage(char *name) {
        printf("usage: %s [-j port] [-c port] [-p device] [-v] [-T]\n"
                "\t--jetport port\t-j port  \tlisten port for data\n"
                "\t--printer name\t-p device\tsend data to this device\n"
                "\t--cmdport port\t-c port  \tspecial command interface\n"
                "\t--verbose     \t-v       \tincrease verbosity\n"
                "\t--testmode    \t-T       \tinternal test mode\n",
                name
                );
    }
    
    while (1) {
        int this_option_optind = optind ? optind : 1;
        int option_index = 0;
        int c;
        
        static struct option long_options[] = {
            {"jetport", required_argument, 0, 'j'},
            {"printer", required_argument, 0, 'p'},
            {"cmdport", required_argument, 0, 'c'},
            {"quiet",   no_argument,       0, 'q'},
            {"testmode", no_argument,      0, 'T'},
            { 0, 0, 0, 0}
        };
        c = getopt_long(ac, av, "j:p:c:qT", long_options, &option_index);
        if (c == -1) break;
        switch (c) {
            case 0:
                printf("option %s", long_options[option_index].name);
                if (optarg)
                    printf(" with arg \"%s\"", optarg);
                printf("\n");
                break;
            case 'q':
                *verbose = 0;
                break;
            case 'p':
                strncpy(p,optarg,p_max_len);
                break;
            case 'j':
                sscanf(optarg,"%d", jetport);
                break;
            case 'c':
                sscanf(optarg,"%d", cmdport);
                break;
            case 'T':
                printf("Entering Test Mode\n");
                *TestMode = 1;
                *verbose +=9;
                break;
            case '?':
                usage(av[0]);
                return 1;;
            default:
                printf("%s: -%c not yet implemented\n", av[0], c);
                return 1;
        }
    }
    return 0;
}



#define RBUFS 2049
int copyTo(int from, int to, int verbose, int testmode) {
    int bytes;
    char buffer[RBUFS];
    if ( (bytes=read(from,buffer,RBUFS-1)) < 0 ) {
        if ( verbose ) {
            syslog(LOG_INFO,"read error on socket");
        }
        return 1;
    }
    if ( bytes > 0 ) {
        if ( verbose ) {
            syslog(LOG_INFO,"rcv'd %d bytes from socket %d will send to %d",
                    bytes, from, to);
        }
        //guessJob(buffer,255);
        if ( testmode ) {
            int i;
            for (i=0; i<bytes; i++ ) {
                if ( !(i % 16) ) printf("\n%04d ", i);
                printf("%02X ", (unsigned short int)buffer[i]);
            }
        } else {
            if ( write(to,buffer,bytes) < 0 ) {
                syslog(LOG_ERR,"write error to printer %m");
                return 1;
            }
        }
        return 0;
    } else { // bytes MUST be 0
        if ( verbose )
            syslog(LOG_INFO,"received 0 bytes assume EOF");
/*        if ( testmode ) {
            if ( isPS() ) {
                printf("sending additional ^D\n");
            }
            printf("EOF detected sending <ESC>%%-12345X\n");
        } else {
            if ( isPS() ) {
                char b = (char )4;
                syslog(LOG_INFO,"send EOF char because of PS");
                (void)write(to,&b,1);
            }
            syslog(LOG_INFO,"sent PJL reset");
            strcpy(buffer,"\e%-12345X");
            (void)write(to,buffer,strlen(buffer));
        }
        */
        return 1;
    }
}

