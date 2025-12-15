//
// UBOIndexAllocator.h
//	Vulkan Module - Dynamic UBO Index Management
//
// Centralized allocator for managing Dynamic Uniform Buffer object indices.
// Eliminates manual index calculation and prevents allocation conflicts.
//
// Features:
//	- Automatic conflict detection
//	- Debug names for easy identification
//	- Free list for index reuse (performance optimization)
//	- Bounds checking to prevent overflow
//	- Thread-safe allocation (optional, currently single-threaded)
//
// Created 14-Dec-2024 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <queue>
#include <vector>
#include <functional>


class UBOIndexAllocator
{
public:
	// Constructor: maxIndices should match DynamicUniformBuffer capacity.
	explicit UBOIndexAllocator(uint32_t maxIndices);
	~UBOIndexAllocator() = default;

	// Allocate next available index with optional debug name for logging.
	//	Returns: Allocated index, or throws if no capacity
	uint32_t allocate(const std::string& debugName = "");

	// Free an index for reuse, optional - call when object destroyed.
	//	Note: Does not validate if index was actually allocated.
	void free(uint32_t index);

	// Debug utilities
	std::string getDebugName(uint32_t index) const;
	void printAllocations() const;		// Logs all active allocations with names.

	// Statistics and capacity checks
	uint32_t getAllocatedCount() const { return static_cast<uint32_t>(allocations.size()); }
	uint32_t getMaxIndices() const { return maxIndices; }
	bool hasCapacity() const { return nextIndex < maxIndices || !freeList.empty(); }

private:
	uint32_t maxIndices;	// Maximum number of indices available.
	uint32_t nextIndex;		// Next index to allocate, if no free indices available.

	// Active allocations: index -> debug name
	std::unordered_map<uint32_t, std::string> allocations;

	// Free list: indices that have been freed and can be reused,
	//	uses min-heap to always reuse lowest available index.
	std::priority_queue<uint32_t, std::vector<uint32_t>, std::greater<uint32_t>> freeList;
};
