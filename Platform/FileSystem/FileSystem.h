//
// FileSystem.h
//	Vulkan Platform Abstraction
//
// Provide OS-like file system functionality in
//	a platform-independent manner.
//
// 5/19/19 Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef FileSystem_h
#define FileSystem_h

#include <iostream>			//
#include <fstream>			// basic file operations
#include <iomanip>			//	(not for herein, but includer's convenience)
using std::ofstream;		//


//#define OVERRIDE_SDL

#ifdef OVERRIDE_SDL
	#include "LocalFileSystem.h"
	#include "VulkanPlatform.h"
#else
	#include "PlatformSDL.h"
	#include "AppConstants.h"
#endif


class FileSystem : public LocalFileSystem
{
		// MEMBERS
private:
	vector<char> buffer;

		// METHODS
public:
		// app-specific directories
	static string AppSpecificWorkingDirectory()
	{
		string resultPath =
			#ifdef OVERRIDE_SDL
		LocalFileSystem::AppSpecificWorkingDirectory();
				SDL_GetPrefPath(AppConstants.CompanyName, AppConstants.ProjectName);
			#else
			#endif
		return resultPath;
	}

		// asset-specific file operations
	static string ShaderFileFullPath(StrPtr fileName);
	static string TextureFileFullPath(StrPtr fileName);

	vector<char> ReadShaderFile(const string& shaderFilename);
	vector<char> ReadTextureFile(const string& imageFilename);
private:
	vector<char> readFile(const string& fileName, const char* subdirectoryName);
	vector<char> readFile(const string& pathName);
};

#endif // FileSystem_h
