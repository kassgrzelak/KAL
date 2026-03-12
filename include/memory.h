//
// Created by kassie on 11/03/2026.
//

#ifndef KAL_MEMORY_H
#define KAL_MEMORY_H

#include "common.h"

#define GROW_CAPACITY(capacity) \
	((capacity) < 8 ? 8 : (capacity) * 2)

#define GROW_ARRAY(type, pointer, oldCount, newCount) \
	(type*)reallocate(pointer, sizeof(type) * (oldCount), sizeof(type) * (newCount))

#define FREE_ARRAY(type, pointer, oldCount) \
	reallocate(pointer, sizeof(type) * (oldCount), 0)

void* reallocate(void* ptr, size_t newSize);

#endif //KAL_MEMORY_H