//
// Logging.cpp
//	General C++ Universal Code
//
// Implementations for convenience (global functions).
//
// These are actually intended to (eventually) be overridden/
//	overwritten with platform-specific reimplementations.
//
// Created 2/4/19 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#include "Logging.h"
#include <stdarg.h>


#if defined(__APPLE__) && defined(__MACH__)	// for Mac console application
	#define ENDL	"\r\n" << flush
#else
	#define ENDL	endl
#endif


// Assumes..: enum Tier { ERROR, WARN, NOTE };
const char* Prefix[] = { "ERROR! ", "Warning: ", "Note: ", "" };


void Log(Tier tier, string message) {
	cout << Prefix[tier] << message << ENDL;
}

void Log(Tier tier, const char* format, ...)
{
	char buffer[1024];
	va_list vargs;
	va_start(vargs, format);
	vsnprintf(buffer, sizeof buffer, format, vargs);
	va_end(vargs);
	cout << Prefix[tier] << buffer << ENDL;
}


// Note that you may need this if not using endl: (which includes it)
//	fflush(stdout);
// or do
//	std::cout << "\n" << std::flush;

// For console, this is an option too:
//	fprintf(stderr, "error message\n");
