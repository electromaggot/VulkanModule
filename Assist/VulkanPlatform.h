//
// VulkanPlatform.h
//	Vulkan Module Setup
//
// Convenience typedefs, externs, some oft-used implementation and
//	abbreviation, used throughout Vulkan setup, trying to make it a bit
//	easier to use.  Also to help uniformly tie-in the platform interface.
//	For instance, 
	//
	//TJ_TODO: I'm using this file in a more widespread fashion
	//	than just for Vulkan.  Consider renaming/relocating it
	//	to something more general.
	//
	//	...also...
	//COMMENT re: 'using std::'s can be something like:
	//	This is controversial, but
	//	...throughout this code...
	//	the key is really that we're STANDARDIZING on implementations
	//	like std::string for ALL OUR STRINGS, and that
	//	all the 'std::'s clumsily add a lot of noise to our source,
	//	(versus cleaner languages like Swift or C#), when the risk
	//	of some_other_lib::string "sneaking in" is practically
	//	non-existent (or existing only to you pedants out there).
	//
//
// StringArray was an attempt to avoid bloat of e.g. std::vector<std::string>
//	simply to ease C's inability to pass a sized constant string array (since
//	it reduces passed-in array references to simple pointers, losing a
//	preinitialized array's size information).
//
// Created 3/2/19 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef VulkanPlatform_h
#define VulkanPlatform_h

#include "vulkan/vulkan.h"

#include "Logging.h"


extern VkResult call;		// return result of function call  ...for anyone to use

							// Use this for vkFunction calls passing: const VkAllocationCallbacks* pAllocator
extern const VkAllocationCallbacks* const nullALLOC;		// (and give it an explicit unmistakable TYPE too)
							//	to make that seldom-used parameter stand out from actually-significant nullptrs.

typedef const char*		StrPtr;			// Abstract away (somewhat) pointer to string used by Vulkan.
typedef uint32_t		ArrayCount;		// Abstract-out a Vulkan-typical array size indicator.
typedef unsigned int	Index;			// Abstract general means of indexing into array.


extern string ErrStr(VkResult);

extern const char* VkFormatString(VkFormat);


extern void DumpStringVector(StrPtr label, vector<StrPtr> strray);


struct StringArray
{
	ArrayCount	count;
	StrPtr*		pStrings;
};
/*struct StringArray // works well when the strings already exist on the heap (won't free them)
{
	//uint32_t	count;
	vector<const char*>	strings;
	//inline void setCount()	{ count = (sizeof(this) - sizeof(uint32_t)) / sizeof(const char*); };
	uint32_t count() { return (uint32_t) strings.size(); };
	const char** pStrings() { return strings.data(); };
	void append(const char* str) { strings.emplace_back(str); };
};*/


// Finally, specific to Windows/Visual Studio: IntelliSense may complain about some of this
//	project's code that interacts with Vulkan and may be uniquely supported by Clang.  Namely:
//	- Deliberate use of VLAs (variable length arrays) for easy short-lived stack-based array
//	  allocations.    Per IntelliSense:  [E0028]  "expression must have a constant value"
//	- Designated Initializers for clearer (and prettier) data structure pre-initializations,
//	  which Vulkan requires a lot of.    IntelliSense:  [E2878]  "unexpected designator"
// And it's not just those "errors" (not warnings) appearing in IntelliSense's "Error List"
//	but the "red squiggles" appearing in the code and red dot in the scroll bar...
//	while Clang supports those features and such code builds & runs just fine.
// The following preprocessor directives silence those...
//				(and thanks to EDG for publishing this:  http://www.edg.com/docs/edg_cpp.pdf)
//
#ifdef __INTELLISENSE__			// <-- either this clause...
//#pragma clang diagnostic ignored "-Wunknown-pragmas"	// ...or this line, your choice.
  #pragma diag_suppress 28
  #pragma diag_suppress 2878
								// and while we're at it, a false-negative not in our
  #pragma diag_suppress	1847	//	code, but in GLM: "attributes are not allowed here"
#endif


#endif // VulkanPlatform_h
