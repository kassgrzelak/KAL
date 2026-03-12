//
// Created by kassie on 11/03/2026.
//

#include "memory.h"

void* reallocate(void* ptr, const size_t newSize)
{
	if (newSize == 0)
	{
		free(ptr);
		return NULL;
	}

	void* result = realloc(ptr, newSize);
	if (result == NULL)
		exit(1);
	return result;
}
