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
int                 datafd;
int                 printfd;
int                 cmdfd[MAX_CLIENTS], currentCmdFd;
fd_set              readfds;
fd_set              writefds;

static int                 dataMasterFd;
static int                 cmdMasterFd;
static int                 max_sd;

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

void newMyStringListe(MyStringListe *base) {
    if ((base = malloc(sizeof (MyStringListe))) == NULL) {
        syslog(LOG_ERR, "OUT OF MEMORY in MyStringListe");
        exit(253);
    }
    base->text = NULL;
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

void MyStringListeInsertEnd(MyStringListe *head, MyString *s) {
    MyStringListe *t, *u;
    if ( head == NULL ) {
        newMyStringListe(&head);
        head->text = s;
        return;
    }
    for ( t=head; t->next != NULL ; t = t->next )
        ;
    newMyStringListe(&u);
    t->next = u;
    u->text = s;
}

void MyStringListeInsertFront(MyStringListe *head, MyString*s) {
    MyStringListe *t, *u;
    if ( head == NULL ) {
        newMyStringListe(&head);
        head->text = s;
        return;
    }
    newMyStringListe(&u);
    u->text = s;
    u->next = head;
    head = u;

}

MyString MyStringListeFetchEnd(MyStringListe *head) {
    MyStringListe *t, *u;
    MyString *s;
    if ( head == NULL ) return NULL;
    t = head;
    u = NULL;
    while ( t->next != NULL ) {
        u = t;
        t = t->next;
    }
    s = t->text;
    if ( t->next != NULL ) {
        u->next = t->next;
        free(t->next);
    }
    free(t);
    t = NULL;
    return(s);
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


void setup(char *version) {
    char *msg = "compiled \"%s\" \"%s\"%s";
    if (verbosity & VERBOSE_DBG) {
        printf("%s ready\n", version);
        printf(msg, __DATE__, __TIME__, "\n");
    }
    if (verbosity) {
        syslog(LOG_INFO, version);
        syslog(LOG_INFO, "Compile \"%s\" \"%s\"", __DATE__, __TIME__,"");
    }
    return;
}

void parseCommandline() {

}

void prepareSockets() {
    int opt = 1;
    
    if ((dataMasterFd = socket(AF_INET6, SOCK_STREAM, 0)) == 0) {
        syslog(LOG_ERR, "data side - socket create failed %m");
        exit(1);
    }
    if (setsockopt(dataMasterFd, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(opt)) < 0) {
        syslog(LOG_ERR, "data side - set sockopt fail: %m");
    }
    dataAddress.sin6_family = AF_INET6;
    dataAddress.sin6_addr   = in6addr_any;
    dataAddress.sin6_port   = htons(jetport);
    if (bind(dataMasterFd, (struct sockaddr *)& dataAddress, sizeof(dataAddress) ) < 0) {
        syslog(LOG_ERR, "data side - cannot bind: %m");
        exit(1);
    }
    syslog(LOG_DEBUG, "data side - listening on %d", jetport);
    if (listen(dataMasterFd, 1) < 0) {
        syslog(LOG_ERR, "data side - cannot listen: %m");
        exit(1);
    }
    /* 2nd -- command port */
    if ((cmdMasterFd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        syslog(LOG_ERR, "cmd side - socket create failed: %m");
        exit(1);
    }
    if (setsockopt(cmdMasterFd, SOL_SOCKET, SO_REUSEADDR,(char *) &opt, sizeof(opt)) < 0) {
        syslog(LOG_ERR, "cmd side - set sockopt fail: %m");
    }
    cmdAddress.sin_family = AF_INET;
    cmdAddress.sin_addr.s_addr = /*INADDR_ANY*/ inet_addr("127.0.0.1");
    cmdAddress.sin_port = htons(cmdport);
    if (bind(cmdMasterFd, (struct sockaddr *) &cmdAddress, sizeof(cmdAddress)) < 0) {
        syslog(LOG_ERR, "cmd side - cannot bind: %m");
        exit(1);
    }
    syslog(LOG_DEBUG, "cmd side - listening on %d", cmdport);
    if (listen(cmdMasterFd, 10) < 0) {
        syslog(LOG_ERR, "cmd side - cannot listen: %m");
        exit(1);
    }
}

void prepareSelect() {
    int i, sd;
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_SET(cmdMasterFd, &readfds);
    max_sd = cmdMasterFd;
    FD_SET(dataMasterFd, &readfds);
    if (dataMasterFd > max_sd) max_sd = dataMasterFd;
    if ( printfd > 0 ) {
        FD_SET(printfd, &readfds);
        FD_SET(printfd, &writefds);
        if (printfd > max_sd) max_sd = printfd;        
    }
    if ( datafd > 0 ) {
        FD_SET(datafd, &readfds);
        FD_SET(datafd, &writefds);
        if (datafd > max_sd) max_sd = datafd;
    }
    for (i = 0; i < max_clients; i++) {
        sd = cmdfd[i];
        if (sd > 0) {
            FD_SET(sd, &readfds);
            FD_SET(datafd, &writefds);
            if (sd > max_sd) max_sd = sd;
        }
    }
}

enum selectActions performSelect() {
    int activity, i;
    struct timeval tv = {60, 0};
    activity = select(max_sd + 1, &readfds, &writefds, NULL, &tv);
    if ((activity < 0) && (errno != EINTR)) {
        syslog(LOG_ERR, "select error: %m");
        return error;
    }
    if ( activity == 0 ) return timeout;
    if ( verbosity & 0x8 ) {
        (void)printf("Select data avail on %d FDs\n", activity);
    }
    if (FD_ISSET(dataMasterFd, &readfds)) return newData;
    if (FD_ISSET(cmdMasterFd, &readfds)) return newCmd;

    if (FD_ISSET(datafd, &readfds)) return readData;
    if (FD_ISSET(datafd, &writefds)) return writeData;
    
    if (FD_ISSET(printfd, &readfds)) return readPrinter;
    if (FD_ISSET(printfd, &writefds)) return writePrinter;
    for (i = 0; i < max_clients; i++) {
        if (FD_ISSET(cmdfd[i], &readfds)) {
            currentCmdFd = cmdfd[i];
            return readCmd;
        }
    }
    for (i = 0; i < max_clients; i++) {
        if (FD_ISSET(cmdfd[i], &writefds)) {
            currentCmdFd = cmdfd[i];
            return writeCmd;      
        }
    }
    
    return error;
}

void selectTimeout() {
    if (verbosity & 0x08)
        printf("Select timeout\n");
    return;
}

void readDataFromSocket() {
    if (verbosity & 0x08)
        printf("read data from spooler to buffer\n");
}

void readCommandSocket() {
    printf("read command from command channel\n");
}

void readDataFromPrinter() {
    printf("read data from printer tu buffer\n");
}

void writeDataToPrinter() {
    printf("write data from buffer to printer\n");
}

void writeDataToSocket() {
    printf("write data from buffer to print spooler\n");
}

void writeCommandToPrinter() {
    printf("send command to printer\n");
}

void writeCommandSocket() {
    printf("write command result\n");
}

void acceptDataSocket() {
    printf("accept Data socket\n");
}

void acceptCommandSocket() {
    printf("accept command socket\n");
}

int terminateRequest() {
    return 1;
}
