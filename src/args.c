#include "args.h"


int scanFunc(
	const char * restrict input,
	const char * restrict format,
	char * restrict outbuf
)
{
	assert(input  != NULL);
	assert(format != NULL);
	assert(outbuf != NULL);
	
	// Find the slash
	const char * formtemp = format;
	
	for (; *formtemp != '\0'; ++formtemp)
	{
		if (*formtemp == '/')
		{
			break;
		}
	}
	
	const char * fullFMatch = format;
	size_t required = 0;
	if (*formtemp == '/')
	{
		required = (size_t)atoll(format);
		fullFMatch = formtemp + 1;
	}

	formtemp = (*formtemp != '\0') ? formtemp + 1 : formtemp;
	for (; *formtemp != '\0'; ++formtemp)
	{
		if (*formtemp == '=')
		{
			break;
		}
	}

	size_t fullFMatchLen = (size_t)(formtemp - fullFMatch);
	required = (required == 0) ? fullFMatchLen : required;

	input += (input[0] == '-') + (input[1] == '-');
	input += (input[0] == '/');

	// First match the full
	bool found = false;
	for (size_t i = 0; i < fullFMatchLen; ++i)
	{
		if (input[i] == '=')
		{
			if (i < required || *formtemp != '=')
			{
				return -1;
			}
			input += i + 1;
			found = true;
			break;
		}
		else if (input[i] == '\0')
		{
			if (i < required)
			{
				return -1;
			}
			break;
		}
		else if (fullFMatch[i] != input[i])
		{
			return -1;
		}
	}
	if (!found)
	{
		input += fullFMatchLen + 1;
	}

	if (*formtemp == '=')
	{
		++formtemp;
		size_t maxChars = (size_t)atoll(formtemp);
		size_t inpLen = strlen(input);
		maxChars = (inpLen < maxChars) ? inpLen : maxChars;
		memcpy(outbuf, input, maxChars);
		outbuf[maxChars] = '\0';
	}

	return 1;
}

bool scanarg(
	int argc,
	char * const * restrict argv,
	bool * restrict helpflag,
	bool * restrict marked,
	bool ignoremarked,
	const char * restrict format,
	char * restrict buf
)
{
	assert(argc >= 1);
	assert(argv != NULL);
	assert(helpflag != NULL);
	assert(marked != NULL);
	assert(format != NULL);
	assert(buf != NULL);

	if (argc == 1)
	{
		buf[0] = '\0';
		return false;
	}
	for (int i = 1; i < argc; ++i)
	{
		if (scanFunc(argv[i], "1/help", buf) >= 0 || scanFunc(argv[i], "?", buf) >= 0)
		{
			*helpflag = true;
			return true;
		}
		else if (ignoremarked || !marked[i])
		{
			int res = scanFunc(argv[i], format, buf);
			if (res > 0 || strcmp(argv[i], format) == 0)
			{
				marked[i] = true;
				return true;
			}
		}
	}

	buf[0] = '\0';
	for (int i = 1; i < argc; ++i)
	{
		if (!marked[i])
		{
			strcpy(buf, argv[i]);
			break;
		}
	}

	return false;
}
bool scanargs(
	int argc,
	char * const * restrict argv,
	bool * restrict helpflag,
	char * const * restrict formats,
	const bool * restrict isOptional,
	char * restrict bufs,
	size_t bufSize,
	size_t numArgs
)
{
	assert(argc >= 1);
	assert(argv != NULL);
	assert(helpflag != NULL);
	assert(formats != NULL);
	assert(isOptional != NULL);
	assert(bufs != NULL);
	assert(bufSize >= 2);
	assert(numArgs >= 1);	

	bool * marked = calloc((size_t)argc, sizeof(bool));
	if (marked == NULL)
	{
		bufs[0] = '\0';
		return false;
	}

	for (size_t i = 0; i < numArgs; ++i)
	{
		if (!scanarg(
			argc, argv, helpflag,
			marked, false,
			formats[i], &bufs[bufSize * i]
		))
		{
			if (!isOptional[i])
			{
				free(marked);
				strcpy(bufs, &bufs[bufSize * i]);
				return false;
			}
			else
			{
				bufs[bufSize * i] = '\0';
			}
		}
		if (*helpflag)
		{
			free(marked);
			return true;
		}
	}

	free(marked);
	return true;
}

void printhelp(const char * restrict app)
{
	assert(app != NULL);

	printf(
		"Correct usage:\n"
		"\n"
		"%s --server --port=<port number>\n"
		"%s --client --ip=<IP address> --port=<port number>\n"
		"\n"
		"All switches:\n"
		"  --help         -  Shows this page\n"
		"  --?            -  Shows this page\n"
		"  --server       -  Specifies server mode\n"
		"  --client       -  Specifies client mode\n"
		"  --ip=<addr>    -  Specifies <addr> as the IPv4 address to connect to\n"
		"  --port=<port>  -  Specifies <port> as the port number in the range of 0 to 65535\n"
		"\n"
		"Methods for closing the program:\n"
		"Ctrl+Alt+Shift+Z\n"
		"Ctrl+Alt+Shift+Q\n",
		app, app
	);
}


bool handleArgs(
	int argc,
	char * const * restrict argv,
	size_t bufSize,
	char bufs[NUM_BUFS][bufSize],
	bool * restrict helpflag,
	bool * restrict isServer
)
{
	assert(argc >= 1);
	assert(argv != NULL);
	assert(argv[0] != NULL);
	assert(bufSize > 1);
	assert(bufs != NULL);
	assert(helpflag != NULL);
	assert(isServer != NULL);

	*helpflag = false;
	char * const serverFormats[NUM_SERVERFORMATS] = {
		"1/server",
		"1/port=6",
	};
	bool serverOptional[NUM_SERVERFORMATS] = {
		false,
		false
	};
	char * const clientFormats[NUM_CLIENTFORMATS] = {
		"1/client",
		"1/port=6",
		"1/ip=16"
	};
	bool clientOptional[NUM_CLIENTFORMATS] = {
		false,
		false,
		false
	};

	*isServer = true;
	if (!scanargs(argc, argv, helpflag, serverFormats, serverOptional, (char *)bufs, MAX_BUF, NUM_SERVERFORMATS))
	{
		if (!*helpflag)
		{
			*isServer = false;
			if (!scanargs(argc, argv, helpflag, clientFormats, clientOptional, (char *)bufs, MAX_BUF, NUM_CLIENTFORMATS))
			{
				if (argc > 1)
				{
					fprintf(stderr, "Un-recognized command-line option '%s'\n", bufs[0]);
					if (!*helpflag)
					{
						return false;
					}
				}
				else
				{
					*helpflag = true;
				}
			}
		}
	}

	if (*helpflag)
	{
		printhelp(argv[0]);
	}

	return true;
}
