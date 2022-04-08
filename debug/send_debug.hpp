#ifndef SEND_DEBUG_H
#define SEND_DEBUG_H

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sstream>

class SendDebug
{
public:
    void send(char input[200]);
    void send(std::string input);
    void send(int input);
    
private:
    struct my_msgbuf
    {
        long mtype;
        char mtext[200];
    };
};

#endif
