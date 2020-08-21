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
#include "AppConstants.h"		// for company/project names


static	char* SDLCALL	dirnameExecutableBase	= nullptr;
static	char* SDLCALL	dirnameAppLocalStorage	= nullptr;


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
		if (! dirnameAppLocalStorage) {
			dirnameAppLocalStorage = SDL_GetPrefPath(AppConstants.CompanyName, AppConstants.ProjectName);
			Log(RAW, "STORAGE %s", dirnameAppLocalStorage);
		}
		return dirnameAppLocalStorage;
	}
};
