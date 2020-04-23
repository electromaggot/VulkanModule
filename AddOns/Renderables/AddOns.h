//
// AddOns.h
//	VulkanModule AddOns
//
// E.g. "AddOns" may be...		Dependency		...although optional because:
//		---------------			----------		--------------------
//		Vertex Buffer 		VertexBasedObject*	(vertex shaders may encapsulate their own vertices)
//		Index Buffer				"			(also vertex shader implementation-specific)
//	Uniform Buffer Objects		   UBO...		(technically not needed for a super-simple demo)
//		  Textures			 TextureSpec[]...	(ditto)
//						 ...and: Descriptors,
//						   (via) DescribEd[]
// * - Note that VertexBasedObject itself is non-optional (and is not managed here), but
//		this "vertex specifier" may indeed specify that no Vertex (or Index) Buffer is needed.
//
// Created 3/26/20 by Tadd
//	© 2020 Megaphone Studios
//
#ifndef AddOns_h
#define AddOns_h

#include "VulkanSetup.h"

#include "Descriptors.h"
#include "VertexBasedObject.h"
#include "UniformBufferLiterals.h"
#include "UniformBuffer.h"
#include "TextureImage.h"

struct Renderable;		// skirt circular reference including iRenderable.h


struct AddOns
{
	friend class iRenderable;
	friend class FixedRenderable;
	friend class Renderables;


	AddOns(Renderable& renderable, VulkanSetup& setup, iPlatform& platform);
	~AddOns();

		// MEMBERS
protected:
	vector<DescribEd>	described;

	PrimitiveBuffer*	pVertexBuffer	= nullptr;
	PrimitiveBuffer*	pIndexBuffer	= nullptr;

	UniformBuffer*		  pUniformBuffer;
	vector<TextureImage*> pTextureImages;

	UBO					ubo;			// Store local copies of these,
	vector<TextureSpec>	texspecs;		//	otherwise they go away.

	VulkanSetup&		vulkan;			// These are retained mainly
	iPlatform&			platform;		//	for Recreate.

		// METHODS

	void createVertexAndOrIndexBuffers(VertexBasedObject& vertexObject);
	void destroyVertexAndOrIndexBuffers();

	void createDescribedItems(UBO* pUBO, TextureSpec textureSpecs[], iPlatform& platform);	// IMPORTANT:
												  // ^^^^^^^^^^^^^^ This "array" is really just a pointer, and is expected to either be null (nullptr) or point
												  //				to an array of TextureSpec structures TERMINATED by one that is null or having: .fileName == nullptr

	void Recreate(VertexBasedObject& vertexObject);
	void RecreateDescribables();
private:
	vector<DescribEd> reDescribe();
};

#endif	// AddOns_h
