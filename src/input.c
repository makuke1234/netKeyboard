#include "input.h"
#include "udp.h"

void keyboardInputHandler(udp_t * restrict udp, char * restrict buffer, int buflen)
{
	udpThread_t udpThread;
	udpThread_init(udp, &udpThread);
	if (!udpThread_read(&udpThread, buffer, buflen, stdout))
	{
		fprintf(stderr, "Async server receiver thread creation failed!\n");
		exit(1);
	}
	
	INPUT_RECORD records[MAX_RECORD];
	HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);

	printf("Ready to send data!\n");

	while (1)
	{
		DWORD nEvents;
		if ((kbhit() || (GetAsyncKeyState(VK_CONTROL) & 0x8000)) && ReadConsoleInputW(hStdIn, records, MAX_RECORD, &nEvents) && (nEvents > 0))
		{
			// Parse all events to buffer and possibly send out messages
			bool breakFlag = false;
			for (DWORD i = 0; i < nEvents; ++i)
			{
				INPUT_RECORD * event = &records[i];
				if (event->EventType == KEY_EVENT)
				{
					const KEY_EVENT_RECORD * kev = &event->Event.KeyEvent;

					if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) && ((GetAsyncKeyState(VK_LSHIFT) & 0x8000) || (GetAsyncKeyState(VK_RSHIFT) & 0x800)) &&
						((GetAsyncKeyState(VK_LMENU) & 0x8000) || (GetAsyncKeyState(VK_RMENU) & 0x8000)) &&
						((kev->wVirtualKeyCode == 'Q') || (kev->wVirtualKeyCode == 'Z')) )
					{
						breakFlag = true;
						break;
					}
				}
			}

			if (breakFlag)
			{
				break;
			}
			else if (nEvents > 0)
			{
				udp_write(udp, records, (int)(nEvents * sizeof(INPUT_RECORD)));
			}

		}
		Sleep(1);
	}

	printf("Exiting...\n");
	udpThread_stopRead(&udpThread);
}
void keyboardOutputHandler(struct udp * restrict udp)
{
	INPUT_RECORD records[MAX_RECORD];

	char str[BUFLEN];
	sprintf(str, "Connected!\n");
	udp_write(udp, str, (int)strlen(str));

	udpThread_t udpThread;
	udpThread_init(udp, &udpThread);
	if (!udpThread_readKbdOut(&udpThread, records, MAX_RECORD))
	{
		fprintf(stderr, "Async server receiver thread creation failed!\n");
		exit(1);
	}
	
	INPUT_RECORD inpRecords[MAX_RECORD];
	HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);

	printf("Ready to receive data!\n");

	while (1)
	{
		DWORD nEvents;
		if ((kbhit() || (GetAsyncKeyState(VK_CONTROL) & 0x8000)) && ReadConsoleInputW(hStdIn, inpRecords, MAX_RECORD, &nEvents) && (nEvents > 0))
		{
			// Parse all events to buffer and possibly send out messages
			bool breakFlag = false;
			for (DWORD i = 0; i < nEvents; ++i)
			{
				const INPUT_RECORD * event = &inpRecords[i];
				if (event->EventType == KEY_EVENT)
				{
					const KEY_EVENT_RECORD * kev = &event->Event.KeyEvent;

					if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) && ((GetAsyncKeyState(VK_LSHIFT) & 0x8000) || (GetAsyncKeyState(VK_RSHIFT) & 0x800)) &&
						((GetAsyncKeyState(VK_LMENU) & 0x8000) || (GetAsyncKeyState(VK_RMENU) & 0x8000)) &&
						((kev->wVirtualKeyCode == 'Q') || (kev->wVirtualKeyCode == 'Z')) )
					{
						breakFlag = true;
						break;
					}
				}
				else
				{
					printf("\nUnknown key combination!\n");
				}
			}

			if (breakFlag)
			{
				break;
			}

		}
		Sleep(1);
	}

	printf("Exiting...\n");
	udpThread_stopRead(&udpThread);
}
