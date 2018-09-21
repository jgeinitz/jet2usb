/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
# include "fun.h"

typedef struct t_MyString {
    int     len;
    int     maxlen;
    uint8_t *buffer;
} MyString;

typedef struct t_MyStringListe {
    struct t_MyStringListe *next;
    MyString               *text;
} MyStringListe;

MyStringListe *hexdumptext = NULL;


MyString *newMyString(int maxlen) {
    MyString *t;
    uint8_t  *c;
    if ( (t=malloc(sizeof(MyString))) == NULL ) {
        fprintf(stderr,"PANIC: OUT OF MEMORY");
        syslog(LOG_ERR,"OUT OF MEMORY");
        exit(250);
    }
    t->maxlen = maxlen;
    t->len = 0;
    if ( (c=malloc(maxlen*(sizeof(uint8_t)))) == NULL ) {
        fprintf(stderr,"PANIC: OUT OF MEMORY (II)");
        syslog(LOG_ERR,"OUT OF MEMORY (II)");
        exit(250);
    }
    t->buffer = c;
    return t;
}

void deleteMyString(MyString *t ) {
    if ( t == NULL ) {
        return; // Avoid double free
    }
    if ( t->buffer != NULL ) {
        free(t->buffer);
    }
    free(t);
    return;
}

void newMyStringListe(MyStringListe *base, int maxlen) {
    if ( (base=malloc(sizeof(MyStringListe))) == NULL ) {
        syslog(LOG_ERR,"OUT OF MEMORY in MyStringListe");
        exit(253);
    }
    base->text = newMyString(maxlen);
    base->next = NULL;
    return;    
}

void deleteMyStringListe(MyStringListe *base) {
    MyStringListe *t,*u;
    t = base;
    while ( t != NULL ) {
        deleteMyString(t->text);
        u = t;
        t = t->next;
        free(u);
    }
}


/************************************************************************/


void setup() {

}

void parseCommandline() {

}

void prepareSockets() {

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
