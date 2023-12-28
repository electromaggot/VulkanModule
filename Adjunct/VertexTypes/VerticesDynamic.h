//
// VerticesDynamic.h
//	Vulkan Vertex-based Add-ons
//
// C being strongly typed disallows a practical VertexDynamic; that is, to assign
//	at runtime one of the types from Vertex3DTypes then aggregate via: vector<>
//	This is the way; an aggregator to manage the vertices dynamically.
//
// "size" always refers to "number of bytes"
//
// Created 9/17/23 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef VerticesDynamic_h
#define VerticesDynamic_h

#include "VertexAttribute.h"
#include "Vertex3DTypes.h"
#include "Logging.h"


struct VerticesDynamic {
	const size_t	size1stAlloc =	64 * AttributeByteSizes[POSITION];
	const float		growAllocsBy =	1.5f;	// https://github.com/facebook/folly/blob/main/folly/docs/FBVector.md#memory-handling

	size_t		sizeLastAlloc	= 0;

	size_t	sizeVertices	= 0;		// grows by size of one Vertex per each push_back
	char*	pBytes			= nullptr;


	~VerticesDynamic() {
		clear();
	}

	void clear() {
		if (pBytes)
			free(pBytes);
		sizeLastAlloc = 0;
		sizeVertices = 0;
		pBytes = nullptr;
	}


	void setAttributes(AttributeBits attribute_bits) {
		attribits = attribute_bits;
		sizeVertex = numBytesGivenAttributes(attribits);
	}
	void setAttributes(vector<VertexAttribute> attrs) {
		std::copy(attrs.begin(), attrs.end(), layout);
		sizeVertex = 0;
		for (const auto& attr : attrs)
			sizeVertex += AttributeByteSizes[attr];
	}


	void push_back(Vertex3DNormalTextureColor& vertex) {
		if (sizeVertices + sizeVertex > sizeLastAlloc) {
			if (sizeLastAlloc)
				sizeLastAlloc *= growAllocsBy;
			else
				sizeLastAlloc = size1stAlloc;
			localRealloc(sizeLastAlloc);
			if (failed) return;
		}
		memcpy(pBytes + sizeVertices, &vertex, sizeVertex);
		sizeVertices += sizeVertex;
	}

	void exactResize() {
		localRealloc(sizeVertices);
		if (failed) return;
		sizeLastAlloc = sizeVertices;
	}

	size_t size()		{ return sizeVertices; }

	uint32_t count()	{ return (uint32_t) sizeVertices / (sizeVertex ? sizeVertex : -1); }
															 //  avert divide by zero ^^ offloading error to caller
	AttributeBits attributeBits()	{ return attribits; }

	void* operator [] (const int index) {
		return pBytes + index * sizeVertex;
	}

private:	// not to be assigned directly

	friend class	MeshObject;		// (although)

	size_t	sizeVertex		= 0;

	AttributeBits attribits = 0;
	VertexAttribute layout[N_ELEMENTS_IN_ARRAY(AttributeFormats)];


	// realloc() but with rudimentary error-check/recovery
	//
	void localRealloc(size_t newSize)
	{
		char* pAlloced = (char*) realloc(pBytes, newSize);
		failed = (pAlloced == nullptr);
		if (failed)
			Log(ERROR, "VerticesDynamic: realloc(%ul) failed, model can't grow past %ul, may appear truncated.", newSize, sizeLastAlloc);
		else
			pBytes = pAlloced;
	}
	bool failed = false;
};


#endif	// VerticesDynamic_h
