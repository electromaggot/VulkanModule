//
// UniformBufferLiterals.h
//	Vulkan Add-ons
//
// Define specific, commonly-used UBO concretions.
//
// These are "overlaid" into a new UniformBuffer object, its ShaderStage set, and
//	emplaced into an array of DescribEd objects, from which Descriptors constructs.
// The app writes into UBO fields as defined below, then those values pass to the
//	shader via its "layout(binding = " index, which needs to match on both sides.
//
// Created 6/14/19 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef UniformBufferLiterals_h
#define UniformBufferLiterals_h

#include "VertexAbstract.h"


enum DestinationStage {
	DESTINATION_VERTEX_STAGE	= VK_SHADER_STAGE_VERTEX_BIT,
	DESTINATION_FRAGMENT_STAGE	= VK_SHADER_STAGE_FRAGMENT_BIT,
	DESTINATION_UNKNOWN			= -1
};


//-- Model-/View-/Projection-matrix UBOs -------------------------------------
// Primarily destined for the Vertex Stage, so that will be their default below.
//  For convenience, these can assume two forms:

// One for simplistic cases where only a single model matrix is needed...
//
struct UBO_MVP {
	alignas(16)		mat4 model	= mat4(1.0f);
	alignas(16)		mat4 view	= mat4(1.0f);
	alignas(16)		mat4 proj	= mat4(1.0f);
};

// ...and a more-refined form where View-Projection matrices for the camera are
//	global thus passed separately, with Model matrix excluded, instead considered
//	"local" thus transferred per-Object3D, done via UBO(mat4& ...) seen below.
//
struct UBO_VP {
	alignas(16)		mat4 view	= mat4(1.0f);
	alignas(16)		mat4 proj	= mat4(1.0f);
};


//-- Other UBO Variants ------------------------------------------------------

// This UBO passes real-time state used by a Fragment Shader renderer, e.g. a Ray-based
//	algorithm, so it is intended for the Fragment Stage, which is its default value below.
//
struct UBO_rtm {
	alignas(16)		vec4  resolution;
	//alignas(12)	vec3  resolution;	// <-- DID NOT WORK! See NOTE 2 below.
	alignas(4)		float time;
	alignas(16)		vec4  mouse;
};

//----------------------------------------------------------------------------


struct UBO {
	int					byteSize;
	void*				pBytes;
	DestinationStage	destinationStage;

	UBO(UBO_MVP& mvp, DestinationStage dstage = DESTINATION_VERTEX_STAGE)
		:	byteSize(sizeof(UBO_MVP)), pBytes(&mvp), destinationStage(dstage)	{ }

	UBO(mat4& model, DestinationStage dstage = DESTINATION_VERTEX_STAGE)
		:	byteSize(sizeof(mat4)), pBytes(&model), destinationStage(dstage)	{ }
	UBO(UBO_VP& vp, DestinationStage dstage = DESTINATION_VERTEX_STAGE)
		:	byteSize(sizeof(UBO_VP)), pBytes(&vp), destinationStage(dstage)		{ }

	UBO(UBO_rtm& rtm, DestinationStage dstage = DESTINATION_FRAGMENT_STAGE)
		:	byteSize(sizeof(UBO_rtm)), pBytes(&rtm), destinationStage(dstage)	{ }

	UBO() : byteSize(0), pBytes(nullptr), destinationStage(DESTINATION_UNKNOWN)	{ }

	VkShaderStageFlags	getShaderStageFlags() {
		assert(destinationStage != DESTINATION_UNKNOWN);
		return (VkShaderStageFlags) destinationStage;
	}
};


#endif	// UniformBufferLiterals_h


/* DEV NOTE
	Alignment issues CPU-side vs. GPU-side:
	https://vulkan-tutorial.com/Uniform_buffers/Descriptor_pool_and_sets#page_Alignment-requirements
*/

/* NOTE 2
	Using "vec3" seemed to lead to a variable-offset-mismatch on the GPU side!  Specifically,
	graphically at runtime, mouseX appeared to increment in time while mouseY remained 0.
	It seemed that resolution was 16 bytes here but 12 there, so the last float of the
	now-vec4 (zero) went into 'time' on the GPU, and 'time' here went into mouseX there.
	Attempting 'alignas(12)' above as corrective measure is illegal: not a power of two.
*/
