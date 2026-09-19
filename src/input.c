//
// Created by kassie on 19/09/2026.
//

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

char getPressedKey()
{
	HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
	DWORD count;

	if (h == INVALID_HANDLE_VALUE)
		return 0;

	if (!GetNumberOfConsoleInputEvents(h, &count) || count == 0)
		return 0;

	INPUT_RECORD record;

	while (count--)
	{
		DWORD read;

		if (!ReadConsoleInputA(h, &record, 1, &read))
			return 0;

		if (record.EventType == KEY_EVENT &&
			record.Event.KeyEvent.bKeyDown &&
			record.Event.KeyEvent.uChar.AsciiChar != 0)
		{
			const char c = record.Event.KeyEvent.uChar.AsciiChar;

			if (c == 13) // If key is newline (on Windows, CR LF), return just LF.
				return 10;

			return c;
		}
	}

	return 0;
}
#else // POSIX/Linux
char getPressedKey()
{
	return 0; // TODO: getPressedKey() working for linux
}
#endif