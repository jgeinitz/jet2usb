/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include "fun.h"

int verbosity = 0x0b;
#define VERBOSE_MSG     1
#define VERBOSE_DBG     8
#define VERBOSE_HEXDUMP 2

int                 jetport = 9100;
int                 cmdport = 9191;
struct sockaddr_in6 dataAddress;
struct sockaddr_in  cmdAddress;
#define MAX_CLIENTS 10
int                 max_clients = MAX_CLIENTS;
int                 client_socket[MAX_CLIENTS];
int                 cmd_socket[MAX_CLIENTS];
fd_set              readfds;
fd_set              writefds;

int                 datafd;
int                 cmdfd;

typedef struct t_MyString {
    int len;
    int maxlen;
    uint8_t *buffer;
} MyString;

typedef struct t_MyStringListe {
    struct t_MyStringListe *next;
    MyString *text;
} MyStringListe;

MyString *newMyString(int maxlen) {
    MyString *t;
    uint8_t *c;
    if ((t = malloc(sizeof (MyString))) == NULL) {
        fprintf(stderr, "PANIC: OUT OF MEMORY");
        syslog(LOG_ERR, "OUT OF MEMORY");
        exit(250);
    }
    t->maxlen = maxlen;
    t->len = 0;
    if ((c = malloc(maxlen * (sizeof (uint8_t)))) == NULL) {
        fprintf(stderr, "PANIC: OUT OF MEMORY (II)");
        syslog(LOG_ERR, "OUT OF MEMORY (II)");
        exit(250);
    }
    t->buffer = c;
    return t;
}

void deleteMyString(MyString *t) {
    if (t == NULL) {
        return; // Avoid double free
    }
    if (t->buffer != NULL) {
        free(t->buffer);
    }
    free(t);
    return;
}

void newMyStringListe(MyStringListe *base, int maxlen) {
    if ((base = malloc(sizeof (MyStringListe))) == NULL) {
        syslog(LOG_ERR, "OUT OF MEMORY in MyStringListe");
        exit(253);
    }
    base->text = newMyString(maxlen);
    base->next = NULL;
    return;
}

void deleteMyStringListe(MyStringListe *base) {
    MyStringListe *t, *u;
    t = base;
    while (t != NULL) {
        deleteMyString(t->text);
        u = t;
        t = t->next;
        free(u);
    }
}

#define DUMPLENGTH 8

void hexdump(MyString *in, char *introText, int doSyslog, int doPrintf) {
    int i, j;
    char string1[DUMPLENGTH * 4 + 8];
    char string2[DUMPLENGTH + 8];
    char tempString[DUMPLENGTH * 4 + 8];

    if (doSyslog) syslog(LOG_DEBUG, ">>>>>>> %s", introText);
    if (doPrintf) printf(">>>>>>> %s\n", introText);
    for (j = 0; j < in->len; j += DUMPLENGTH) {
        (void) strncpy(tempString, "", 2);
        (void) sprintf(string1, "%04X", (int) j);
        (void) strncpy(string2, "", 2);
        for (i = 0; i < DUMPLENGTH; i++) {
            (void) strncpy(tempString, string1, ((DUMPLENGTH * 4) + 8));
            if ((i + j) > in->len) {
                (void) sprintf(string1, "%s   ", tempString);
            } else {
                (void) sprintf(string1, "%s %02X", tempString, in->buffer[j + i]);
                (void) strncpy(tempString, string2, DUMPLENGTH + 8);
                (void) sprintf(string2, "%s%c", tempString, (char) in->buffer[j + i]);
            }
        }
        if (doSyslog)
            syslog(LOG_DEBUG, "%s %s", string1, string2);
        if (doPrintf) {
            (void) printf("%s %s\n", string1, string2);
        }
    }
}

/************************************************************************/


void setup(char *myname, char *version) {
    if (verbosity & VERBOSE_DBG) {
        printf("%s %s ready\n", myname, version);
        printf("Compile \"%s\" \"%s\"\n", __DATE__, __TIME__);
    }
    if (verbosity) {
        syslog(LOG_INFO, version);
        syslog(LOG_INFO, "Compile \"%s\" \"%s\"", __DATE__, __TIME__);
    }
    return;
}

void parseCommandline() {

}

void prepareSockets() {
    int opt = 1;
    
    if ((datafd = socket(AF_INET6, SOCK_STREAM, 0)) == 0) {
        syslog(LOG_ERR, "data side - socket create failed %m");
        exit(1);
    }
    if (setsockopt(datafd, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(opt)) < 0) {
        syslog(LOG_ERR, "data side - set sockopt fail: %m");
    }
    dataAddress.sin6_family = AF_INET6;
    dataAddress.sin6_addr   = in6addr_any;
    dataAddress.sin6_port   = htons(jetport);
    if (bind(datafd, (struct sockaddr *)& dataAddress, sizeof(dataAddress) ) < 0) {
        syslog(LOG_ERR, "data side - cannot bind: %m");
        exit(1);
    }
    syslog(LOG_DEBUG, "data side - listening on %d", jetport);
    if (listen(datafd, 1) < 0) {
        syslog(LOG_ERR, "data side - cannot listen: %m");
        exit(1);
    }
    /* 2nd -- command port */
    if ((cmdfd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        syslog(LOG_ERR, "cmd side - socket create failed: %m");
        exit(1);
    }
    if (setsockopt(cmdfd, SOL_SOCKET, SO_REUSEADDR,(char *) &opt, sizeof(opt)) < 0) {
        syslog(LOG_ERR, "cmd side - set sockopt fail: %m");
    }
    cmdAddress.sin_family = AF_INET;
    cmdAddress.sin_addr.s_addr = /*INADDR_ANY*/ inet_addr("127.0.0.1");
    cmdAddress.sin_port = htons(cmdport);
    if (bind(cmdfd, (struct sockaddr *) &cmdAddress, sizeof(cmdAddress)) < 0) {
        syslog(LOG_ERR, "cmd side - cannot bind: %m");
        exit(1);
    }
    syslog(LOG_DEBUG, "cmd side - listening on %d", cmdport);
    if (listen(cmdfd, 10) < 0) {
        syslog(LOG_ERR, "cmd side - cannot listen: %m");
        exit(1);
    }
}

void prepareSelect() {

}

int performSelect() {
    return -1;

}

void readDataFromSocket() {

}

void readCommandSocket() {

}

void readDataFromPrinter() {

}

void writeDataToPrinter() {

}

void writeDataToSocket() {

}

void writeCommandToPrinter() {

}

void writeCommandSocket() {

}

void acceptDataSocket() {

}

void acceptCommandSocket() {

}

int terminateRequest() {
    return 1;
}
