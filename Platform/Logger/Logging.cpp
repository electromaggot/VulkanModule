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
#include "AppConstants.h"
#include "AppSettings.h"
#include "FileSystem.h"

static void logToFile(Tier, const char*);


//#define DEBUG_LOW


#if defined(__APPLE__) && defined(__MACH__)
	#if TARGET_OS_IPHONE	// for Xcode debug console
		#define ENDL	"\n" << flush
	#else					// for Mac console application
		#define ENDL	"\r\n" << flush
	#endif
#else
	#define ENDL	endl
#endif


// Assumes..: enum Tier { ERROR, WARN, NOTE, RAW, HANG, LOW };
const char* Prefix[] = { "ERROR! ", "Warning: ", "Note: ", "", "", "" };

string logFileName;
const char* pLogFileName = nullptr;


void Log(Tier tier, string message) {
	#ifndef DEBUG_LOW
	if (tier == LOW) return;
	#endif
	cout << Prefix[tier] << message;
	if (tier != HANG)  cout << ENDL;

	if (AppConstants.Settings.isDebugLogToFile)
		logToFile(tier, message.c_str());
}

void Log(Tier tier, const char* format, ...)
{
	#ifndef DEBUG_LOW
	if (tier == LOW) return;
	#endif

	char buffer[1024];
	va_list vargs;
	va_start(vargs, format);
	vsnprintf(buffer, sizeof buffer, format, vargs);
	va_end(vargs);
	cout << Prefix[tier] << buffer;
	if (tier != HANG) cout << ENDL;

	if (AppConstants.Settings.isDebugLogToFile)
		logToFile(tier, buffer);
}

void logToFile(Tier tier, const char* buffer)
{
	if (!pLogFileName) {
		logFileName = FileSystem::AppLocalStorageDirectory() + AppConstants.DebugLogFileName;
		pLogFileName = logFileName.c_str();
	}
	ofstream	settingsFile;
	settingsFile.open(logFileName, ofstream::out | ofstream::app);
	if (settingsFile.is_open())
	{
		settingsFile << Prefix[tier] << buffer;
		if (tier != HANG) settingsFile << endl << flush;

		settingsFile.close();
	}
}


// Note that you may need this if not using endl: (which includes it)
//	fflush(stdout);
// or do
//	std::cout << "\n" << std::flush;

// For console, this is an option too:
//	fprintf(stderr, "error message\n");
