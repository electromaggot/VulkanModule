//
// SDL FileSystem.cpp
//	Platform-independent via SDL
//
// SDL's file/directory abstractions have become our preferred method
//	to handle each platform's idea of a "working directory" "executable
//	directory" "user/app-specific storage" etc.
//
// 8/16/20 Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#include "VulkanPlatform.h"		// for std::string etc.
#include "PlatformSDL.h"		// for SDL #includes
#include "VulkanConfig.h"		// for company/project names


static	char*	dirnameExecutableBase	= nullptr;
static	char*	dirnameAppLocalStorage	= nullptr;


class LocalFileSystem
{
protected:	// (discourage standalone (non-derived) instantiation)

	~LocalFileSystem() {
		if (dirnameExecutableBase)
			SDL_free(dirnameExecutableBase);
		if (dirnameAppLocalStorage)
			SDL_free(dirnameAppLocalStorage);
	}

	// Path of Executable
	//
	static string exeAccompaniedFullPath(const string& fileName, const char* subdirectoryName)
	{
		if (! dirnameExecutableBase)
			dirnameExecutableBase = SDL_GetBasePath();

		return string(dirnameExecutableBase) + subdirectoryName + fileName;
	}

	// App-specific Per-user Storage Path
	//
	static string appLocalStorageDirectory()
	{
		if (! dirnameAppLocalStorage)
			dirnameAppLocalStorage = SDL_GetPrefPath(TheVulkanConfig().companyName, TheVulkanConfig().projectName);

		return dirnameAppLocalStorage;
	}

	static std::string failsafeDataDir()
	{
		std::string homeDir;
		#ifdef _WIN32
			const char* appData = std::getenv("APPDATA");
			if (appData) {
				homeDir = std::string(appData) + "\\TuneTrip";
			} else {
				homeDir = ".tunetrip";
			}
		#elif defined(__APPLE__)
			const char* home = std::getenv("HOME");
			if (home) {
				homeDir = std::string(home) + "/Library/Application Support/TuneTrip";
			} else {
				homeDir = ".tunetrip";
			}
		#else
			// Linux/Unix: use XDG_DATA_HOME or ~/.local/share
			const char* xdgData = std::getenv("XDG_DATA_HOME");
			if (xdgData) {
				homeDir = std::string(xdgData) + "/tunetrip";
			} else {
				const char* home = std::getenv("HOME");
				if (home) {
					homeDir = std::string(home) + "/.local/share/tunetrip";
				} else {
					homeDir = ".tunetrip";
				}
			}
		#endif
		return homeDir;
	}
};
