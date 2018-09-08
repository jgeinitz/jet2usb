
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

static int typeval;
#define PS 1
#define PCL 0
#define UNSPEC -1

void Write(int f, char* b, int l ) {
    int rc;
    rc = write(f,b,l);
    return;
}

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
            {"verbose", no_argument,       0, 'v'},
            {"testmode", no_argument,      0, 'T'},
            { 0, 0, 0, 0}
        };
        c = getopt_long(ac, av, "j:p:c:vT", long_options, &option_index);
        if (c == -1) break;
        switch (c) {
            case 0:
                printf("option %s", long_options[option_index].name);
                if (optarg)
                    printf(" with arg \"%s\"", optarg);
                printf("\n");
                break;
            case 'v':
                *verbose += 1;
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


int testprinter(char *p, int tst) {
    FILE *pfd;
    
    if ( tst ) {
        printf("testprinter: simulated printer is ok\n");
        return 0;
    }
    if ( (pfd=fopen(p,"wb+")) == NULL ) {
        return 1;
    }
    fclose(pfd);
    return 0;
}

int init_data_listener(int *fds, int port, struct sockaddr_in6 *dataAddress ) {
    int opt = TRUE;
    

    
    if ((*fds = socket(AF_INET6, SOCK_STREAM, 0)) == 0) {
        char *emsg = strerror(errno);
        syslog(LOG_ERR, "data side - socket create failed %s", emsg);
        return 1;
    }
    if (setsockopt(*fds, SOL_SOCKET, SO_REUSEADDR,
            (char *) &opt, sizeof(opt)) < 0) {
        char *emsg = strerror(errno);
        syslog(LOG_ERR, "data side - set sockopt fail: %s", emsg);
    }
    dataAddress->sin6_family = AF_INET6;
    dataAddress->sin6_addr   = in6addr_any;
    dataAddress->sin6_port   = htons(port);
    if (bind(*fds,(struct sockaddr *) dataAddress, sizeof(*dataAddress) ) < 0) {
        char *emsg = strerror(errno);
        syslog(LOG_ERR, "data side - cannot bind: %s", emsg);
        return 1;
    }
    syslog(LOG_DEBUG, "data side - listening on %d", port);
    if (listen(*fds, 5) < 0) {
        char *emsg = strerror(errno);
        syslog(LOG_ERR, "data side - cannot listen: %s", emsg);
        return 1;
    }
    return 0;
}

int init_cmd_listener(int *fds, int port, struct sockaddr_in *dataAddress) {
    int opt = TRUE;

    if ((*fds = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        char *emsg = strerror(errno);
        syslog(LOG_ERR, "cmd side - socket create failed: %s", emsg);
        return 1;
    }
    if (setsockopt(*fds, SOL_SOCKET, SO_REUSEADDR,
            (char *) &opt, sizeof(opt)) < 0) {
        char *emsg = strerror(errno);
        syslog(LOG_ERR, "cmd side - set sockopt fail: %s", emsg);
    }
    dataAddress->sin_family = AF_INET;
    dataAddress->sin_addr.s_addr = /*INADDR_ANY*/ inet_addr("127.0.0.1");
    dataAddress->sin_port = htons(port);
    if (bind(*fds, (struct sockaddr *) dataAddress, sizeof(*dataAddress)) < 0) {
        char *emsg = strerror(errno);
        syslog(LOG_ERR, "cmd side - cannot bind: %s", emsg);
        return 1;
    }
    syslog(LOG_DEBUG, "cmd side - listening on %d", port);
    if (listen(*fds, 10) < 0) {
        char *emsg = strerror(errno);
        syslog(LOG_ERR, "cmd side - cannot listen: %s", emsg);
        return 1;
    }
    return 0;
}

int talkTo(int sd, int *term, int verbose) {
    char buffer[255];
    int amount;
    int i;
    char *prompt = "\r\nJet:\r\n";
    
    if ( verbose > 8 )
        printf("Command Talk entered\n");
    Write(sd, prompt, strlen(prompt));
    for (i=0; i<255; i++)
        buffer[i]='\0';
    if ( (amount=read(sd,buffer,255)) < 0 ) {
        return 1;
    }
    if ( amount == 0 ) {
        if ( verbose > 8 ) {
            printf("Command channel EOF\n");
        }
        return 1;
    }
    if ( amount < 255 ) amount++;
    buffer[amount] = '\0';
    
    switch ( buffer[0] ) {
        case 'q':
            return 1;
        case 't':
            *term = 1;
            break;
        default: {
            char *t = "Unknown: ";
            Write(sd,t, strlen(t));
            Write(sd, buffer, strlen(buffer));
        }
    }
    return 0;
}

#define RBUFS 327681
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
        guessJob(buffer,255);
        if ( testmode ) {
            int i;
            for (i=0; i<bytes; i++ ) {
                if ( !(i % 16) ) printf("\n%04d ", i);
                printf("%02X ", (unsigned short int)buffer[i]);
            }
        } else {
            Write(to,buffer,bytes);
            /*if ( isPS()) {
                int i;
                for ( i = 0; i<bytes; i++)
                    if ( buffer[i] = (char)4 )
                        return 1;
            }*/
        }
        return 0;
    } else { // bytes MUST be 0
        if ( verbose )
            syslog(LOG_INFO,"received 0 bytes assume EOF");
        if ( testmode ) {
            if ( isPS() ) {
                printf("sending additional ^D\n");
            }
            printf("EOF detected sending <ESC>%%-12345X\n");
        } else {
            if ( isPS() ) {
                char b = (char )4;
                syslog(LOG_INFO,"send EOF char because of PS");
                Write(to,&b,1);
            }
            syslog(LOG_INFO,"sent PJL reset");
            strcpy(buffer,"\e%-12345X");
            Write(to,buffer,strlen(buffer));
        }
        return 1;
    }
}

#if 0
int demomain() {
    /* Variable and structure definitions. */
    int sd, sd2, rc, length = sizeof (int);
    int totalcnt = 0, on = 1;
    char temp;
#define BufferLength 255
    char buffer[BufferLength];
    struct sockaddr_in serveraddr;
    struct sockaddr_in their_addr;

    fd_set read_fd;
    struct timeval timeout;
    timeout.tv_sec = 15;
    timeout.tv_usec = 0;

    /* The socket() function returns a socket descriptor */
    /* representing an endpoint. The statement also */
    /* identifies that the INET (Internet Protocol) */
    /* address family with the TCP transport (SOCK_STREAM) */
    /* will be used for this socket. */
    /************************************************/
    /* Get a socket descriptor */
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Server-socket() error");
        /* Just exit */
        exit(-1);
    } else
        printf("Server-socket() is OK\n");

    /* The setsockopt() function is used to allow */
    /* the local address to be reused when the server */
    /* is restarted before the required wait time */
    /* expires. */
    /***********************************************/
    /* Allow socket descriptor to be reusable */
    if ((rc = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof (on))) < 0) {
        perror("Server-setsockopt() error");
        close(sd);
        exit(-1);
    } else
        printf("Server-setsockopt() is OK\n");

    /* bind to an address */
    memset(&serveraddr, 0x00, sizeof (struct sockaddr_in));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(33/*SERVPORT*/);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    printf("Using %s, listening at %d\n", inet_ntoa(serveraddr.sin_addr), 33/*SERVPORT*/);

    /* After the socket descriptor is created, a bind() */
    /* function gets a unique name for the socket. */
    /* In this example, the user sets the */
    /* s_addr to zero, which allows the system to */
    /* connect to any client that used port 3005. */
    if ((rc = bind(sd, (struct sockaddr *) &serveraddr, sizeof (serveraddr))) < 0) {
        perror("Server-bind() error");
        /* Close the socket descriptor */
        close(sd);
        /* and just exit */
        exit(-1);
    } else
        printf("Server-bind() is OK\n");

    /* The listen() function allows the server to accept */
    /* incoming client connections. In this example, */
    /* the backlog is set to 10. This means that the */
    /* system can queue up to 10 connection requests before */
    /* the system starts rejecting incoming requests.*/
    /*************************************************/
    /* Up to 10 clients can be queued */
    if ((rc = listen(sd, 10)) < 0) {
        perror("Server-listen() error");
        close(sd);
        exit(-1);
    } else
        printf("Server-Ready for client connection...\n");

    /* The server will accept a connection request */
    /* with this accept() function, provided the */
    /* connection request does the following: */
    /* - Is part of the same address family */
    /* - Uses streams sockets (TCP) */
    /* - Attempts to connect to the specified port */
    /***********************************************/
    /* accept() the incoming connection request. */
    int sin_size = sizeof (struct sockaddr_in);
    if ((sd2 = accept(sd, (struct sockaddr *) &their_addr, &sin_size)) < 0) {
        perror("Server-accept() error");
        close(sd);
        exit(-1);
    } else
        printf("Server-accept() is OK\n");

    /*client IP*/
    printf("Server-new socket, sd2 is OK...\n");
    printf("Got connection from the f***ing client: %s\n", inet_ntoa(their_addr.sin_addr));

    /* The select() function allows the process to */
    /* wait for an event to occur and to wake up */
    /* the process when the event occurs. In this */
    /* example, the system notifies the process */
    /* only when data is available to read. */
    /***********************************************/
    /* Wait for up to 15 seconds on */
    /* select() for data to be read. */
    FD_ZERO(&read_fd);
    FD_SET(sd2, &read_fd);
    rc = select(sd2 + 1, &read_fd, NULL, NULL, &timeout);
    if ((rc == 1) && (FD_ISSET(sd2, &read_fd))) {
        /* Read data from the client. */
        totalcnt = 0;

        while (totalcnt < BufferLength) {
            /* When select() indicates that there is data */
            /* available, use the read() function to read */
            /* 100 bytes of the string that the */
            /* client sent. */
            /***********************************************/
            /* read() from client */
            rc = read(sd2, &buffer[totalcnt], (BufferLength - totalcnt));
            if (rc < 0) {
                perror("Server-read() error");
                close(sd);
                close(sd2);
                exit(-1);
            } else if (rc == 0) {
                printf("Client program has issued a close()\n");
                close(sd);
                close(sd2);
                exit(-1);
            } else {
                totalcnt += rc;
                printf("Server-read() is OK\n");
            }
        }
    }
    else if (rc < 0) {
        perror("Server-select() error");
        close(sd);
        close(sd2);
        exit(-1);
    } /* rc == 0 */ else {
        printf("Server-select() timed out.\n");
        close(sd);
        close(sd2);
        exit(-1);
    }
    /* Shows the data */
    printf("Received data from the f***ing client: %s\n", buffer);
    /* Echo some bytes of string, back */
    /* to the client by using the write() */
    /* function. */
    /************************************/
    /* write() some bytes of string, */
    /* back to the client. */
    printf("Server-Echoing back to client...\n");
    rc = write(sd2, buffer, totalcnt);
    if (rc != totalcnt) {
        perror("Server-write() error");
        /* Get the error number. */
        rc = getsockopt(sd2, SOL_SOCKET, SO_ERROR, &temp, &length);
        if (rc == 0) {
            /* Print out the asynchronously */
            /* received error. */
            errno = temp;
            perror("SO_ERROR was: ");
        } else
            printf("Server-write() is OK\n");

        close(sd);
        close(sd2);
        exit(-1);
    }
    /* When the data has been sent, close() */
    /* the socket descriptor that was returned */
    /* from the accept() verb and close() the */
    /* original socket descriptor. */
    /*****************************************/
    /* Close the connection to the client and */
    /* close the server listening socket. */
    /******************************************/
    close(sd2);
    close(sd);
    exit(0);
    return 0;
}
#endif

