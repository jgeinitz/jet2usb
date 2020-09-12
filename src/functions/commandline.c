
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

int parsearguments(int ac, char **av,
        int *verbose,
        int *jetport,
        int *cmdport,
        int *TestMode,
        char *p, int p_max_len
    ) {
    
    void usage(char *name) {
        fprintf(stderr,"usage: %s [-j port] [-c port] [-p device] [-v] [-T]\n"
                "\t--jetport port\t-j port  \tlisten port for data\n"
                "\t--printer name\t-p device\tsend data to this device\n"
                "\t--cmdport port\t-c port  \tspecial command interface\n"
                "\t--verbose     \t-v       \tbe verbose\n"
        		"\t--Debug level \t-D level \tset debug level\n"
                "\t--testmode    \t-T       \tinternal test mode\n",
                name
                );
    }
    
    while (1) {
        //int this_option_optind = optind ? optind : 1;
        int option_index = 0;
        int c;
        
        static struct option long_options[] = {
            {"jetport", required_argument, 0, 'j'},
            {"printer", required_argument, 0, 'p'},
            {"cmdport", required_argument, 0, 'c'},
			{"Debug",   required_argument, 0, 'D'},
            {"verbose", no_argument,       0, 'v'},
            {"testmode", no_argument,      0, 'T'},
            { 0, 0, 0, 0}
        };
        c = getopt_long(ac, av, "j:p:c:D:vT", long_options, &option_index);
        if (c == -1) break;
        switch (c) {
            case 0:
                fprintf(stderr,"option %s", long_options[option_index].name);
                if (optarg)
                    fprintf(stderr," with arg \"%s\"", optarg);
                printf("\n");
                break;
            case 'v':
                *verbose = 1;
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
            case 'D':
            	sscanf(optarg,"%d", verbose);
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
                fprintf(stderr,"%s: -%c not yet implemented\n", av[0], c);
                return 1;
        }
    }
    return 0;
}


