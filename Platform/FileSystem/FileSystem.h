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
using std::ifstream;		//


//#define OVERRIDE_SDL

#ifdef OVERRIDE_SDL
	#include "LocalFileSystem.h"
#else
	#include "FileSystemSDL.h"
#endif


class FileSystem : LocalFileSystem
{
		// MEMBERS
private:
	vector<char> buffer;

		// METHODS
public:
		// app-specific/platform-specific directories, expose implementations publicly
	static string ExeAccompaniedFullPath(const string& fileName, StrPtr subDirectory)
	{
		return LocalFileSystem::exeAccompaniedFullPath(fileName, subDirectory);
	}
	static string AppLocalStorageDirectory()
	{
		return LocalFileSystem::appLocalStorageDirectory();
	}

		// asset-specific file operations
	static string ShaderFileFullPath(StrPtr fileName);
	static string TextureFileFullPath(StrPtr fileName);
	static string FontFileFullPath(StrPtr fileName);

	vector<char> ReadShaderFile(const string& shaderFilename);
	vector<char> ReadTextureFile(const string& imageFilename);
private:
	vector<char> readFile(const string& fileName, const char* subdirectoryName, const char* showFileType);
	vector<char> readFile(const string& pathName);
};

#endif // FileSystem_h
