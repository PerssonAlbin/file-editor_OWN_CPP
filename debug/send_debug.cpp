#include "send_debug.hpp"

#define PERMS 0644

void SendDebug::send(char input[200])
{
    struct my_msgbuf buf;
    int msqid;
    int len;
    key_t key;

    if ((key = ftok("msgq.txt", 'B')) == -1)
    {
        perror("ftok");
        exit(1);
    }

    if ((msqid = msgget(key, PERMS)) == -1)
    {
        perror("msgget");
        exit(1);
    }

    int x = 0;
    while(input[x] != '\0' && input[x] != '\n')
    {
        buf.mtext[x] = input[x];
        x += 1;
    }
    buf.mtype = x;

    /* remove newline at end, if it exists */
    buf.mtext[buf.mtype] = '\0';
    if (msgsnd(msqid, &buf, buf.mtype+1, 0) == -1) /* +1 for '\0' */
        perror("msgsnd");
}

void SendDebug::send(std::string input)
{
    struct my_msgbuf buf;
    int msqid;
    int len;
    key_t key;

    if ((key = ftok("msgq.txt", 'B')) == -1)
    {
        perror("ftok");
        exit(1);
    }

    if ((msqid = msgget(key, PERMS)) == -1)
    {
        perror("msgget");
        exit(1);
    }

    int x = 0;
    while(input[x] != '\0' && input[x] != '\n')
    {
        buf.mtext[x] = input[x];
        x += 1;
    }
    buf.mtype = x;

    /* remove newline at end, if it exists */
    buf.mtext[buf.mtype] = '\0';
    if (msgsnd(msqid, &buf, buf.mtype+1, 0) == -1) /* +1 for '\0' */
        perror("msgsnd");
}

void SendDebug::send(int input)
{
    struct my_msgbuf buf;
    int msqid;
    int len;
    key_t key;

    if ((key = ftok("msgq.txt", 'B')) == -1)
    {
        perror("ftok");
        exit(1);
    }

    if ((msqid = msgget(key, PERMS)) == -1)
    {
        perror("msgget");
        exit(1);
    }
    std::ostringstream n_str;
    n_str << input;
    std::string input_str = n_str.str();
    int x = 0;
    while(input_str[x] != '\0' && input_str[x] != '\n')
    {
        buf.mtext[x] = input_str[x];
        x += 1;
    }
    buf.mtype = x;

    /* remove newline at end, if it exists */
    buf.mtext[buf.mtype] = '\0';
    if (msgsnd(msqid, &buf, buf.mtype+1, 0) == -1) /* +1 for '\0' */
        perror("msgsnd");
}
