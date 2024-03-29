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

class DrawableSpecifier;	// skirt circular reference including iRenderable.h


struct AddOns
{
	friend struct iRenderable;
	friend struct FixedRenderable;
	friend struct DynamicRenderable;
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

	void createVertexAndOrIndexBuffers(MeshObject& meshObject);
	void destroyVertexAndOrIndexBuffers();

	void createDescribedItems(vector<UBO>& UBO, vector<TextureSpec>& textureSpecs, iPlatform& platform);
	void destroyDescribedItems();

	void Recreate(MeshObject& meshObject);
	void RecreateDescribables();
private:
	vector<DescribEd> reDescribe();

		// getters
public:
	vector<TextureImage*>&	textureImages()	 { return pTextureImages; }
};

#endif	// AddOns_h
