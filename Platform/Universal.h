//
// Universal.h
//	General App Chassis
//
// Common stuff pretty much used everywhere, project/submodule-wide.
//
// Some of this used to be Vulkan-specific, but that was isolated
//	to VulkanModule.  Now it's very generic or C++ ease-of-use related.
//	Log output components were also isolated to Logging.h/.cpp.
//
// Recreated 2/20/20 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef Universal_h
#define Universal_h


#include <string>
using std::string;			// To directly avoid: using namespace std;
using std::to_string;		//	but to also declutter our entire codebase, these are
							//	absolutely not expected to lurk in some other library.
#include <vector>
using std::vector;			// ...and while we're at it,
#include <stdexcept>		//	these additional ones are
using std::exception;		//	well-established as well.
using std::runtime_error;

#include <iostream>
using std::cout; using std::endl; using std::flush;

#include <assert.h>


#define N_ELEMENTS_IN_ARRAY(ELEMENTS)	(sizeof(ELEMENTS) / sizeof(ELEMENTS[0]))


#endif	// Universal_h
