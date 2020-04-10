//
// Shader.h
//	Vulkan Setup
//
// Shader abstraction: generically specify shaders.
//
// 5/19/19 Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef Shader_h
#define Shader_h


enum ShaderType {

	VERTEX		= VK_SHADER_STAGE_VERTEX_BIT,
	FRAGMENT 	= VK_SHADER_STAGE_FRAGMENT_BIT,

	GEOMETRY	= VK_SHADER_STAGE_GEOMETRY_BIT,
	TESSELATION	= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,

	COMPUTE		= VK_SHADER_STAGE_COMPUTE_BIT
};


struct Shader
{
	ShaderType	fileType;
	StrPtr		nameShaderFile;
	StrPtr		nameEntrypointFunction;
};

typedef vector<Shader> Shaders;
/*struct Shaders
{
	int		nShaders;
	Shader	array[];
};*/


#endif // Shader_h
