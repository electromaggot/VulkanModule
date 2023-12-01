//
// FileSystem.cpp
//	Vulkan Platform Abstraction
//
// See matched header file for definitive main comment.
//
// Should be thread-safe if each thread instantiates its own.
//
// 5/19/19 Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#include "FileSystem.h"

#include <fstream>


const StrPtr SHADER_SUBDIRECTORY  = "compiledShaders/";

const StrPtr TEXTURE_SUBDIRECTORY = "textures/";

const StrPtr MODEL_SUBDIRECTORY = "models/";

const StrPtr FONT_SUBDIRECTORY = "fonts/";


// Directory hierarchy project-specific conventions

string FileSystem::ShaderFileFullPath(StrPtr fileName)
{
	return ExeAccompaniedFullPath(fileName, SHADER_SUBDIRECTORY);
}

string FileSystem::TextureFileFullPath(StrPtr fileName)
{
	return ExeAccompaniedFullPath(fileName, TEXTURE_SUBDIRECTORY);
}

string FileSystem::ModelFileFullPath(const char* fileName)
{
	return ExeAccompaniedFullPath(fileName, MODEL_SUBDIRECTORY);
}
string FileSystem::ModelFileFullPath(const string& fileName)
{
	return ExeAccompaniedFullPath(fileName.c_str(), MODEL_SUBDIRECTORY);
}


string FileSystem::FontFileFullPath(StrPtr fileName)
{
	return ExeAccompaniedFullPath(fileName, FONT_SUBDIRECTORY);
}


vector<char> FileSystem::ReadShaderFile(const string& shaderFilename)
{
	return readFile(shaderFilename, SHADER_SUBDIRECTORY, "shader");
}

vector<char> FileSystem::ReadTextureFile(const string& imageFilename)
{
	return readFile(imageFilename, TEXTURE_SUBDIRECTORY, "texture");
}


vector<char> FileSystem::readFile(const string& fileName, const char* subdirectoryName,
														  const char* showFileType)
{
	const string fullPath = ExeAccompaniedFullPath(fileName, subdirectoryName);
	Log(RAW, "Read: %s - file: %s", showFileType, fullPath.c_str());
	return readFile(fullPath);
}


// Standard Library file operations, hence generic.

vector<char> FileSystem::readFile(const string& pathName)
{
	std::ifstream file(pathName, std::ios::ate | std::ios::binary);

	if (!file.is_open())
		Fatal("Failed to open file: \"" + pathName + "\"!");

	size_t fileSize = (size_t) file.tellg();
	buffer.resize(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}
