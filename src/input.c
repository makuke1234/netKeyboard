#include "input.h"
#include "udp.h"

static struct
{
	udp_t * udp;
	volatile bool bBreakFlag;
	DWORD tid;

} LLKbdInputProc_data = {
	.udp        = NULL,
	.bBreakFlag = false
};


LRESULT CALLBACK LLKbdInputProc(int nCode, WPARAM wp, LPARAM lp)
{
	bool eat = false, down = false, isactive = GetForegroundWindow() == GetConsoleWindow();

	static bool codes[256] = { 0 };

	const KBDLLHOOKSTRUCT * p = (KBDLLHOOKSTRUCT *)lp;

	switch (wp)
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		down = true;
		codes[p->vkCode] = true;
		break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		down = false;
		codes[p->vkCode] = false;
		break;
	}
	switch (wp)
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		{
			INPUT_RECORD ir = { 0 };

			ir.EventType = KEY_EVENT;
			KEY_EVENT_RECORD * kev = &ir.Event.KeyEvent;

			kev->bKeyDown = down;
			kev->wVirtualKeyCode  = (WORD)p->vkCode;
			kev->wVirtualScanCode = (WORD)p->scanCode;

			if ((codes[VK_LCONTROL] || codes[VK_RCONTROL]) && (codes[VK_LSHIFT] || codes[VK_RSHIFT]) &&
				(codes[VK_LMENU] || codes[VK_RMENU]) &&
				((kev->wVirtualKeyCode == 'Q') || (kev->wVirtualKeyCode == 'Z')) )
			{
				LLKbdInputProc_data.bBreakFlag = true;
				PostThreadMessageW(LLKbdInputProc_data.tid, WM_CLOSE, 0, 0);
			}
			else
			{
				if (isactive)
				{
					udp_write(LLKbdInputProc_data.udp, &ir, sizeof(INPUT_RECORD));
					eat = true;
				}
			}
			break;
		}
	}

	return eat ? 1 : CallNextHookEx(NULL, nCode, wp, lp);
}

void keyboardInputHandler(udp_t * restrict udp, char * restrict buffer, int buflen)
{
	udpThread_t udpThread;
	udpThread_init(udp, &udpThread);
	if (!udpThread_read(&udpThread, buffer, buflen, stdout))
	{
		fprintf(stderr, "Async server receiver thread creation failed!\n");
		exit(1);
	}
	
	LLKbdInputProc_data.udp = udp;
	LLKbdInputProc_data.tid = GetCurrentThreadId();

	HHOOK hhkLL = SetWindowsHookExW(WH_KEYBOARD_LL, &LLKbdInputProc, NULL, 0);

	printf("Ready to send data!\n");

	MSG msg;
	BOOL bret;
	while (((bret = GetMessageW(&msg, NULL, 0, 0)) != 0) && !LLKbdInputProc_data.bBreakFlag)
	{
		if (bret == -1)
		{
			break;
		}
		printf("Message\n");
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	UnhookWindowsHookEx(hhkLL);

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
