//
// Created by kassie on 11/03/2026.
//

#ifndef KAL_MEMORY_H
#define KAL_MEMORY_H

#include "common.h"

#define GROW_CAPACITY(capacity) \
	((capacity) < 8 ? 8 : (capacity) * 2)

#define INIT_ARRAY(arrayName, arrayPtrName) \
	arrayName->arrayPtrName = NULL; \
	arrayName->count = 0; \
	arrayName->capacity = 0

#define GROW_ARRAY(type, pointer, newCount) \
	(type*)reallocate(pointer, sizeof(type) * (newCount))

#define WRITE_ARRAY(type, arrayName, arrayPtrName, itemName) \
	if (arrayName->capacity < arrayName->count + 1) \
	{ \
		int oldCapacity = arrayName->capacity; \
		arrayName->capacity = GROW_CAPACITY(oldCapacity); \
		arrayName->arrayPtrName = GROW_ARRAY(type, arrayName->arrayPtrName, arrayName->capacity); \
	} \
	arrayName->arrayPtrName[arrayName->count] = itemName; \
	arrayName->count++

void* reallocate(void* ptr, size_t newSize);

#endif //KAL_MEMORY_H