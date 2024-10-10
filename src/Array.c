#include "Array.h"

int InitArray(Array* arr) {
	arr->data = (Array_T*)malloc(DEFAULT_CAPACITY * sizeof(Array_T));
	arr->size = 0;
	arr->capacity = DEFAULT_CAPACITY;
	
	return arr->data == NULL;
}

int ClearArray(Array* arr, int resetCapacity) {
	free(arr->data);
	
	arr->size = 0;
	arr->capacity = resetCapacity ? DEFAULT_CAPACITY : arr->capacity;
	arr->data = (Array_T*)malloc(arr->capacity * sizeof(Array_T));
	
	return arr->data == NULL;
}

void FreeArray(Array* arr) {
	free(arr->data);
}

int PushBack(Array* arr, Array_T element) {
	if (arr->data == NULL) {
		return 1;
	}
	
	if (arr->size >= arr->capacity) {
		arr->capacity <<= 1;
		
		Array_T* newData = (Array_T*)malloc(arr->capacity * sizeof(Array_T));
		
		for (int i = 0; i < arr->size; i++) {
			newData[i] = arr->data[i];
		}
		
		free(arr->data);
		
		arr->data = newData;
	}
	
	arr->data[arr->size++] = element;
	
	return 0;
}