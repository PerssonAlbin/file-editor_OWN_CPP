// Copyright 2022 Albin Persson
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string>
#include <iostream>

#define PERMS 0644
struct my_msgbuf {
    int64_t mtype;
    char mtext[200];
};

int main(void) {
    struct my_msgbuf buf;
    int msqid;
    key_t key;
    system("touch msgq.txt");
    if ((key = ftok("msgq.txt", 'B')) == -1) {
        perror("ftok");
        exit(1);
    }
    /* connect to the queue */
    if ((msqid = msgget(key, PERMS|IPC_CREAT)) == -1) {
        perror("msgget");
        exit(1);
    }
    printf("message queue: ready to receive messages.\n");

    for (;;) { /* normally receiving never ends but just to make conclusion */
                /* this program ends with string of end */
        if (msgrcv(msqid, &buf, sizeof(buf.mtext), 0, 0) == -1) {
            perror("msgrcv");
            exit(1);
        }
        std::cout << "recvd: " << buf.mtext << "\n";
    }
    printf("message queue: done receiving messages.\n");
    system("rm msgq.txt");
    return 0;
}
