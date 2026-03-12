//
// Created by kassie on 11/03/2026.
//

#ifndef KAL_MEMORY_H
#define KAL_MEMORY_H

#include "common.h"

#define GROW_CAPACITY(capacity) \
	((capacity) < 8 ? 8 : (capacity) * 2)

#define INIT_ARRAY() \
	array->tokens = NULL; \
	array->count = 0; \
	array->capacity = 0

#define GROW_ARRAY(type, pointer, newCount) \
	(type*)reallocate(pointer, sizeof(type) * (newCount))

#define WRITE_ARRAY(type, arrayPtrName, itemName) \
	if (array->capacity < array->count + 1) \
	{ \
		int oldCapacity = array->capacity; \
		array->capacity = GROW_CAPACITY(oldCapacity); \
		array->arrayPtrName = GROW_ARRAY(type, array->arrayPtrName, array->capacity); \
	} \
	array->arrayPtrName[array->count] = itemName; \
	array->count++

void* reallocate(void* ptr, size_t newSize);

#endif //KAL_MEMORY_H