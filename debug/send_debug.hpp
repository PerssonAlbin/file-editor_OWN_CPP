#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string>

#define PERMS 0644
struct my_msgbuf {
   long mtype;
   char mtext[200];
};

int debug(char input[200]) {
    struct my_msgbuf buf;
    int msqid;
    int len;
    key_t key;
    

    if ((key = ftok("msgq.txt", 'B')) == -1) {
        perror("ftok");
        exit(1);
    }

    if ((msqid = msgget(key, PERMS)) == -1) {
        perror("msgget");
        exit(1);
    }
    //printf("message queue: ready to send messages.\n");
    //printf("Enter lines of text, ^D to quit:\n");
    

    //buf.mtext = input;
    int x;
    while(input[x] != '\0')
    {
        buf.mtext[x] = input[x];
        x += 1;
    }
    buf.mtype = x;

    /* remove newline at end, if it exists */
    buf.mtext[buf.mtype] = '\0';
    if (msgsnd(msqid, &buf, buf.mtype+1, 0) == -1) /* +1 for '\0' */
        perror("msgsnd");
   return 0;
}
