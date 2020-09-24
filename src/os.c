#include <time.h>

#if defined(__linux__)
#include <unistd.h>

void os_usleep(u64 microseconds)
{
	usleep(microseconds);
}

r64 os_time_us()
{
	struct timespec t_spec;
	clock_gettime(CLOCK_MONOTONIC_RAW, &t_spec);
	u64 res = t_spec.tv_nsec + 1000000000 * t_spec.tv_sec;
	return (r64)res / 1000.0;
}
#else
#include <windows.h>
static double perf_frequency;
static void os_set_query_frequency() {
	LARGE_INTEGER li = { 0 };
	QueryPerformanceFrequency(&li);
	perf_frequency = (double)(li.QuadPart);
}
double os_time_us() {
	static initialized = 0;
	if (!initialized) {
		os_set_query_frequency();
		initialized = 1;
	}

	LARGE_INTEGER li = { 0 };
	QueryPerformanceCounter(&li);
	return ((double)(li.QuadPart) / perf_frequency) * 1000000.0;
}

PCHAR *CommandLineToArgvA(PCHAR CmdLine, int *_argc) {
  PCHAR *argv;
  PCHAR _argv;
  ULONG len;
  ULONG argc;
  CHAR a;
  ULONG i, j;

  BOOLEAN in_QM;
  BOOLEAN in_TEXT;
  BOOLEAN in_SPACE;

  len = strlen(CmdLine);
  i = ((len + 2) / 2) * sizeof(PVOID) + sizeof(PVOID);

  argv = (PCHAR *)LocalAlloc(LMEM_FIXED, i + (len + 2) * sizeof(CHAR));

  _argv = (PCHAR)(((PUCHAR)argv) + i);

  argc = 0;
  argv[argc] = _argv;
  in_QM = FALSE;
  in_TEXT = FALSE;
  in_SPACE = TRUE;
  i = 0;
  j = 0;

  while (a = CmdLine[i]) {
    if (in_QM) {
      if (a == '\"') {
        in_QM = FALSE;
      } else {
        _argv[j] = a;
        j++;
      }
    } else {
      switch (a) {
      case '\"':
        in_QM = TRUE;
        in_TEXT = TRUE;
        if (in_SPACE) {
          argv[argc] = _argv + j;
          argc++;
        }
        in_SPACE = FALSE;
        break;
      case ' ':
      case '\t':
      case '\n':
      case '\r':
        if (in_TEXT) {
          _argv[j] = '\0';
          j++;
        }
        in_TEXT = FALSE;
        in_SPACE = TRUE;
        break;
      default:
        in_TEXT = TRUE;
        if (in_SPACE) {
          argv[argc] = _argv + j;
          argc++;
        }
        _argv[j] = a;
        j++;
        in_SPACE = FALSE;
        break;
      }
    }
    i++;
  }
  _argv[j] = '\0';
  argv[argc] = NULL;

  (*_argc) = argc;
  return argv;
}
#endif

int
ustring_unicode_to_utf8(unsigned int unicode, char* buffer)
{
  char* start = buffer;
  char* result = buffer;
  {
    if (unicode <= 0x7f)
    {
      *result++ = (unsigned char) unicode;
    }
    else if (unicode >= 0x80 && unicode <= 0x7ff)
    {
      unsigned char b1 = 0xc0 | (unicode >> 6);
      unsigned char b2 = 0x80 | ((unicode & 0x3f) | 0x30000000);
      *result++ = b1;
      *result++ = b2;
    }
    else if (unicode >= 0x800 && unicode <= 0xffff)
    {
      unsigned char b1 = 0xe0 | (unicode >> 12);
      unsigned char b2 = 0x80 | (((unicode >> 6) & 0x3f) | 0x30000000);
      unsigned char b3 = 0x80 | ((unicode & 0x3f) | 0x30000000);
      *result++ = b1;
      *result++ = b2;
      *result++ = b3;
    }
    else if (unicode >= 0x00010000 && unicode <= 0x001fffff)
    {
      unsigned char b1 = 0xf0 | (unicode >> 18);
      unsigned char b2 = 0x80 | (((unicode >> 12) & 0x3f) | 0x30000000);
      unsigned char b3 = 0x80 | (((unicode >> 6) & 0x3f) | 0x30000000);
      unsigned char b4 = 0x80 | ((unicode & 0x3f) | 0x30000000);
      *result++ = b1;
      *result++ = b2;
      *result++ = b3;
      *result++ = b4;
    }
  }
  return (int) (result - start);
}

char*
unicode_to_utf8(unsigned int* unicode, int length)
{
	char* mem = calloc(length * 4 + 1, sizeof(char));
    char* res = mem;
	for(int i = 0; i < length; ++i)
	{
		mem += ustring_unicode_to_utf8(unicode[i], mem);
	}
	return res;
}
