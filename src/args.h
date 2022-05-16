#ifndef ARGS_H
#define ARGS_H

#include "common.h"

#define MAX_BUF MAX_PATH
#define NUM_BUFS 3

#define NUM_SERVERFORMATS 2
#define NUM_CLIENTFORMATS 3

int scanFunc(
	const char * restrict input,
	const char * restrict format,
	char * restrict outbuf
);

bool scanarg(
	int argc,
	char * const * restrict argv,
	bool * restrict helpflag,
	bool * restrict marked,
	bool ignoremarked,
	const char * restrict format,
	char * restrict buf
);
bool scanargs(
	int argc,
	char * const * restrict argv,
	bool * restrict helpflag,
	char * const * restrict formats,
	const bool * restrict isOptional,
	char * restrict bufs,
	size_t bufSize,
	size_t numArgs
);

void printhelp(const char * restrict app);



bool handleArgs(
	int argc,
	char * const * restrict argv,
	size_t bufSize,
	char bufs[NUM_BUFS][bufSize],
	bool * restrict helpflag,
	bool * restrict isServer
);

#endif
