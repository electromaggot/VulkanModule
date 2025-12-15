//
// UBOIndexAllocator.cpp
//	Vulkan Module - Dynamic UBO Index Management
//
// See header file comment for overview.
//
// Created 14-Dec-2024 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#include "UBOIndexAllocator.h"
#include "Logging.h"
#include <stdexcept>
#include <sstream>


UBOIndexAllocator::UBOIndexAllocator(uint32_t maxIndices)
	:	maxIndices(maxIndices)
	  , nextIndex(0)
{
	Log(NOTE, "UBOIndexAllocator created with capacity for %u indices", maxIndices);
}

uint32_t UBOIndexAllocator::allocate(const std::string& debugName)
{
	uint32_t index;

	if (! freeList.empty()) {			// Try to reuse a freed index first, lowest available.
		index = freeList.top();
		freeList.pop();
		Log(LOW, "🔄 UBO index %u allocated (reused): %s", index, debugName.c_str());
	}
	else {								// Otherwise, allocate next sequential index.
		if (nextIndex >= maxIndices) {
			Log(ERROR, "UBO index allocation failed: capacity exceeded (%u/%u)", nextIndex, maxIndices);
			throw std::runtime_error("UBOIndexAllocator: No more indices available (capacity: " +
									 std::to_string(maxIndices) + ")");
		}
		index = nextIndex++;
		Log(LOW, "➕ UBO index %u allocated (new): %s", index, debugName.c_str());
	}

	allocations[index] = debugName;		// Store allocation with debug name.

	return index;
}

void UBOIndexAllocator::free(uint32_t index)
{
	auto it = allocations.find(index);	// Find allocation...
	if (it == allocations.end()) {
		Log(WARN, "UBO index %u freed but was not tracked (double-free or never allocated?)", index);
		return;
	}
	std::string debugName = it->second;
	allocations.erase(it);
	freeList.push(index);

	Log(LOW, "➖ UBO index %u freed: %s", index, debugName.c_str());
}

std::string UBOIndexAllocator::getDebugName(uint32_t index) const
{
	auto it = allocations.find(index);
	if (it != allocations.end())
		return it->second;
	return "<unallocated>";
}

void UBOIndexAllocator::printAllocations() const
{
	Log(NOTE, "📋 UBO Index Allocations (%u/%u used):", getAllocatedCount(), maxIndices);

	if (allocations.empty()) {
		Log(NOTE, "   (no active allocations)");
		return;
	}

	std::vector<std::pair<uint32_t, std::string>> sorted(allocations.begin(), allocations.end());
	std::sort(sorted.begin(), sorted.end());	// Sort by index for readable output.

	for (const auto& [index, debugName] : sorted)
		Log(NOTE, "   [%3u] %s", index, debugName.c_str());

	if (!freeList.empty()) {			// Show free list if any indices have been freed.
		// Copy free list to vector for display...  priority_queue doesn't support iteration.
		std::priority_queue<uint32_t, std::vector<uint32_t>, std::greater<uint32_t>> tempQueue = freeList;
		std::vector<uint32_t> freeIndices;
		while (!tempQueue.empty()) {
			freeIndices.push_back(tempQueue.top());
			tempQueue.pop();
		}

		std::ostringstream oss;
		oss << "   Free list: [";
		for (size_t i = 0; i < freeIndices.size(); ++i) {
			if (i > 0) oss << ", ";
			oss << freeIndices[i];
		}
		oss << "]";
		Log(NOTE, "%s", oss.str().c_str());
	}
}
