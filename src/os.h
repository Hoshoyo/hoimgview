#pragma once

double os_time_us();
char* unicode_to_utf8(unsigned int* unicode, int length);
#if defined(_WIN32) || defined(_WIN64)
PCHAR* CommandLineToArgvA(PCHAR CmdLine, int *_argc);
#endif