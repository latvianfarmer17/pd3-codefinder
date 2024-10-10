#ifndef ARRAY_H
#define ARRAY_H

#include <malloc.h>

#define DEFAULT_CAPACITY 32

typedef long long unsigned int Array_T;

typedef struct _Array {
	Array_T* data;
	int size;
	int capacity;
} Array;

int InitArray(Array* arr);
int ClearArray(Array* arr, int resetCapacity);
void FreeArray(Array* arr);
int PushBack(Array* arr, Array_T element);

#endif