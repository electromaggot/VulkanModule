//
// AddOns.h
//	VulkanModule AddOns
//
// E.g. "AddOns" may be...		Dependency		...although optional because:
//		---------------			----------		--------------------
//		Vertex Buffer 			MeshObject*		(vertex shaders may encapsulate their own vertices)
//		Index Buffer				"			(also vertex shader implementation-specific)
//	Uniform Buffer Objects		   UBO...		(technically not needed for a super-simple demo)
//		  Textures			 TextureSpec[]...	(ditto)
//						 ...and: Descriptors,
//						   (via) DescribEd[]
// * - Note that MeshObject itself is non-optional (and is not managed here), but
//		this "vertex specifier" may indeed specify that no Vertex (or Index) Buffer is needed.
//
// 3/24/20 Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef AddOns_h
#define AddOns_h

#include "VulkanSetup.h"

#include "Descriptors.h"
#include "MeshObject.h"
#include "UniformBufferLiterals.h"
#include "UniformBuffer.h"
#include "TextureImage.h"
#include "Customizer.h"

class DrawableSpecifier;	// skirt circular reference including iRenderable.h


struct AddOns
{
	friend struct iRenderable;
	friend struct Renderable;
	friend struct SecondaryRenderable;
	friend class  Renderables;


	AddOns(DrawableSpecifier& drawable, VulkanSetup& setup, iPlatform& platform);
	~AddOns();

		// MEMBERS
protected:
	vector<DescribEd>	described;

	PrimitiveBuffer*	pVertexBuffer	= nullptr;
	PrimitiveBuffer*	pIndexBuffer	= nullptr;

	vector<UniformBuffer*>	pUniformBuffers;
	vector<TextureImage*>	pTextureImages;

	vector<UBO>			ubos;			// Store local copies of these,
	vector<TextureSpec>	texspecs;		//	otherwise they go away.

	VulkanSetup&		vulkan;			// These are retained mainly
	iPlatform&			platform;		//	for Recreate.

		// METHODS

	// Dynamic geometry staging.  updateVertexData()/updateIndexData() can be called at any point in the frame -
	//	including before the swapchain image is acquired, when the frame is not yet known - so new data lands HERE
	//	and is copied into each frame's own buffer the first time that frame draws.  Writing all the buffers up
	//	front would clobber frames still in flight; see the DEV NOTE at the end of 'PrimitiveBuffer.cpp'.
	vector<uint8_t>	stagedVertexData;
	vector<uint8_t>	stagedIndexData;
	uint32_t		framesNeedingVertexUpload = 0;	// bitmask, one bit per swapchain image
	uint32_t		framesNeedingIndexUpload  = 0;

	void stageVertexData(void* pData, VkDeviceSize size);
	void stageIndexData(void* pData, VkDeviceSize size);
public:
	// Bring frame iFrame's copies up to date, if this renderable's geometry changed since it last
	//	drew.  Called from the draw path, which is the first point that knows the frame.
	void uploadStagedGeometry(uint32_t iFrame);
protected:

	void createVertexAndOrIndexBuffers(MeshObject& meshObject, Customizer customize = NONE);
	void createDescribedItems(vector<UBO>& UBO, vector<TextureSpec>& textureSpecs,
							  vector<VkDescriptorImageInfo>& runtimeTextures,
							  vector<vector<VkDescriptorImageInfo>>& perFrameRuntimeTextures,
							  iPlatform& platform);
	void destroyVertexAndOrIndexBuffers();

	void createDescribedItems(vector<UBO>& UBO, vector<TextureSpec>& textureSpecs,
							  vector<VkDescriptorImageInfo>& runtimeTextures, iPlatform& platform);
	void destroyDescribedItems();

	// Customizer must be the SAME value the buffers were first created with.  It selects host-visible
	//	versus device-local (so omitting it silently rebuilds DYNAMIC geometry as device-local, after
	//	which every mapped update fails).  iRenderable::Recreate passes its own `customize`.
	void Recreate(MeshObject& meshObject, Customizer customize);
	void RecreateDescribables();
private:
	vector<DescribEd> reDescribe();

		// getters
public:
	vector<TextureImage*>&	textureImages()	 { return pTextureImages; }
};

#endif	// AddOns_h
