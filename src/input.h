#ifndef INPUT_H
#define INPUT_H

#include "common.h"

struct udp;

#define MAX_RECORD 128
#define BUFLEN 32

void keyboardInputHandler(struct udp * restrict udp, char * restrict buffer, int buflen);
void keyboardOutputHandler(struct udp * restrict udp);


#endif
