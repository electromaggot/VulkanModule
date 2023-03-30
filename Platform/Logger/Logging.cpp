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


void LogStartup()	// Provide a startup sanity check...
{					//		but can be a bit tricky, see DEV NOTE below.
	Log(RAW, "RUNNING %s", AppConstants.getExePath());
	Log(RAW, "STORAGE %s", FileSystem::AppLocalStorageDirectory().c_str());
	Log(RAW, "CONFIGS %s", AppConstants.Settings.filePath.c_str());
}


// Assumes..: enum Tier { ERROR, WARN, NOTE, RAW, SAME, LOW };
const char* Prefix[] = { "ERROR! ", "Warning: ", "Note: ", "", "", "" };

string logFileName;
const char* pLogFileName = nullptr;


void Log(Tier tier, string message) {
	#ifndef DEBUG_LOW
	if (tier == LOW)  return;
	#endif
	cout << Prefix[tier] << message;
	if (tier != SAME)  cout << ENDL;

	if (AppConstants.Settings.isDebugLogToFile)
		logToFile(tier, message.c_str());
}

void Log(Tier tier, const char* format, ...)
{
	#ifndef DEBUG_LOW
	if (tier == LOW)  return;
	#endif

	char buffer[1024];
	va_list vargs;
	va_start(vargs, format);
	vsnprintf(buffer, sizeof buffer, format, vargs);
	va_end(vargs);
	cout << Prefix[tier] << buffer;
	if (tier != SAME)  cout << ENDL;

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
		if (tier != SAME)  settingsFile << endl << flush;

		settingsFile.close();
	}
}


// Note that you may need this if not using endl: (which includes it)
//	fflush(stdout);
// or do
//	std::cout << "\n" << std::flush;

// For console, this is an option too:
//	fprintf(stderr, "error message\n");


/* DEV NOTE
"What runs first" can be confusing, but is often implied by the ordering of
lines in the log file.  On startup, for user (i.e. dev!) sanity, we wish
first-thing to log paths used for these components: app, storage, settings
However, if logged when each initializes, their order seems the reverse of
what one would expect.  Hence LogStartup() herein, in one place, with a
"most significant first" ordering of its messages.  However consider the
following actual order-of-initialization:
1. AppConstants struct initializes before even main() runs...
   AppSettings instantiates:
    1.1. Opens/inits from: Settings.json
           but to get that file path:
      1.1.1. Set in FileSystem::AppLocalStorageDirectory()
	  1.1.2. This would normally log the STORAGE path.
	1.2. Sets: isDebugLogToFile   i.e. for ^^^ this line, too late for
	                                   it to go into the log if true.
    1.3. Here's where the CONFIGS line would log.
2. main() now runs, gets exePath from argv[0]...
   Would log the RUNNING path.
Result - atop the log we would have seen:
   STORAGE  (but not if isDebugLogToFile defaults false before set true)
   CONFIGS
   RUNNING
which seems backwards.  Waiting to log all of this until LogStartup(),
not only is STORAGE logged only if its CONFIG is true, now we see a more
hierarchical ordering of the path messages:
   RUNNING
   STORAGE
   CONFIGS
*/
