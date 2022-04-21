// Copyright 2022 Albin Persson
#ifndef INCLUDE_SEND_DEBUG_HPP_
#define INCLUDE_SEND_DEBUG_HPP_

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sstream>
#include <string>


class SendDebug {
 public:
    int preSend();
    void send(char input[200]);
    void send(int input);
    void postSend(char input[200], int msqid);
    SendDebug();
 private:
    struct my_msgbuf {
        int64_t mtype;
        char mtext[200];
    };
};

#endif  // INCLUDE_SEND_DEBUG_HPP_
