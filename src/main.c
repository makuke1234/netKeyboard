#include "common.h"
#include "input.h"
#include "client.h"
#include "server.h"
#include "args.h"

int main(int argc, char ** argv)
{
	SetConsoleCtrlHandler(NULL, TRUE);

	if (!udp_init())
	{
		fprintf(stderr, "UDP initialization failed!\n");
		return 1;
	}

	bool helpflag, isServer;
	char bufs[NUM_BUFS][MAX_BUF];

	if (!handleArgs(argc, argv, MAX_BUF, bufs, &helpflag, &isServer))
	{
		return 2;
	}
	else if (helpflag)
	{
		return 3;
	}

	int buflen = BUFLEN;

	// Opens output file in binary mode
	
	buflen = (buflen < BUFLEN) ? BUFLEN : buflen;
	char * buffer = malloc((size_t)buflen);
	if (buffer == NULL)
	{
		fprintf(stderr, "Error allocating memory for the buffer!\n");
		return 1;
	}

	uint16_t port = (uint16_t)atoi(bufs[1]);
	
	if (isServer)
	{
		printf("Configuration port: %hu\n", port);
	
		udpServer_t server;
		if (!udpServer_open(&server, port))
		{
			fprintf(stderr, "Error initializing server!\n");
			return 1;
		}

		keyboardInputHandler(&server.u, buffer, buflen);

		udpServer_close(&server);
	}
	else
	{
		const char * ip = bufs[2];
		printf("Configuration IP: %s, port: %hu\n", ip, port);

		udpClient_t client;
		if (!udpClient_open(&client, ip, port))
		{
			fprintf(stderr, "Error initializing client!\n");
			return 1;
		}

		keyboardOutputHandler(&client.u);

		udpClient_close(&client);
	}

	free(buffer);

	udp_free();

	return 0;
}
