// Copyright 2022 Albin Persson
#include <string.h>
#include "include/send_debug.hpp"

#define PERMS 0644

SendDebug::SendDebug() {
    // needs to be here in order to intialize the class
}

int SendDebug::preSend() {
    int msqid;
    key_t key;

    if ((key = ftok("msgq.txt", 'B')) == -1) {
        perror("ftok");
        exit(1);
    }

    if ((msqid = msgget(key, PERMS)) == -1) {
        perror("msgget");
        exit(1);
    }
    return msqid;
}

void SendDebug::send(char input[200]) {
    int msqid = preSend();
    postSend(input, msqid);
}

void SendDebug::send(int input) {
    int msqid = preSend();

    std::ostringstream n_str;
    n_str << input;
    std::string input_str = n_str.str();
    
    char copy[200];
    memcpy(copy, input_str.c_str(), input_str.size());

    postSend(copy, msqid);
}

void SendDebug::postSend(char input[200], int msqid) {
    struct my_msgbuf buf;

    int x = 0;
    while (input[x] != '\0' && input[x] != '\n') {
        buf.mtext[x] = input[x];
        x += 1;
    }
    buf.mtype = x;

    /* remove newline at end, if it exists */
    buf.mtext[buf.mtype] = '\0';
    if (msgsnd(msqid, &buf, buf.mtype+1, 0) == -1) /* +1 for '\0' */
        perror("msgsnd");
}