//
// Logging.h
//	General C++ Universal Code
//
// Convenience logging.
//
// Created 2/4/19 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef Logging_h
#define Logging_h

#include "Universal.h"
#include <stdarg.h>
#include <cstring>	// (to build on Linux side)

#undef ERROR // since on Windows, wingdi.h #defines ERROR


extern void LogStartup();


enum Tier { ERROR,	// For: major problem, although non-fatal.
			WARN,	//		minor problem, probably ignorable.
			NOTE,	//		inform progress.	// This tier and below may be "noise" and filtered out.
			RAW,	//	 Won't prefix line with tier-indicator-string (works well after SAME).
			SAME,	//		like NOTE but no newline at line-end, keep next message on SAME line.
			LOW };	//		low-level debugging, could be too verbose.

extern void Log(Tier tier, string message);
extern void Log(Tier tier, const char* format, ...);

inline int Fatal(string message) { throw runtime_error("FATAL: " + message); }
// (Inlined because if inside a non-void function, an otherwise-meaningless return value is required after it.)

inline int Fatal(const char* format, ...) {
	char buffer[1024] = "FATAL: ";
	size_t fatalen = strlen(buffer);
	va_list vargs;
	va_start(vargs, format);
	vsnprintf(buffer + fatalen, sizeof buffer - fatalen, format, vargs);
	va_end(vargs);
	throw runtime_error(buffer);
}

#endif	// Logging_h


/* DEV NOTE -------------------------
//	If only C facilities available...
#include <stdio.h>	// printf
#include <stdlib.h>	// exit
void Fatal(const char* string) {
	printf("%s\r\n", string);
	exit(-1);
}
*/
