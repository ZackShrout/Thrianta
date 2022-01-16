#pragma once

#include "Defines.h"

/*
Memory layout
u64 capacity = number elements that can be held
u64 length = number of elements currently contained
u64 stride = size of each element in bytes
void* elements
*/

enum
{
    DARRAY_CAPACITY,
    DARRAY_LENGTH,
    DARRAY_STRIDE,
    DARRAY_FIELD_LENGTH
};

TAPI void* _DArrayCreate(u64 length, u64 stride);
TAPI void _DArrayDestroy(void* array);

TAPI u64 _DArrayFieldGet(void* array, u64 field);
TAPI void _DArrayFieldSet(void* array, u64 field, u64 value);

TAPI void* _DArrayResize(void* array);

TAPI void* _DArrayPush(void* array, const void* value_ptr);
TAPI void _DArrayPop(void* array, void* dest);

TAPI void* _DArrayPopAt(void* array, u64 index, void* dest);
TAPI void* _DArrayInsertAt(void* array, u64 index, void* value_ptr);

#define DARRAY_DEFAULT_CAPACITY 1
#define DARRAY_RESIZE_FACTOR 2

#define DArrayCreate(type) \
    _DArrayCreate(DARRAY_DEFAULT_CAPACITY, sizeof(type))

#define DArrayReserve(type, capacity) \
    _DArrayCreate(capacity, sizeof(type))

#define DArrayDestroy(array) _DArrayDestroy(array);

#define DArrayPush(array, value)           \
    {                                      \
        typeof(value) temp = value;        \
        array = _DArrayPush(array, &temp); \
    }
// NOTE: could use __auto_type for temp above, but intellisense
// for VSCode flags it as an unknown type. typeof() seems to
// work just fine, though. Both are GNU extensions.

#define DArrayPop(array, value_ptr) \
    _DArrayPop(array, value_ptr)

#define DArrayInsertAt(array, index, value)           \
    {                                                 \
        typeof(value) temp = value;                   \
        array = _DArrayInsertAt(array, index, &temp); \
    }

#define DArrayPopAt(array, index, value_ptr) \
    _DArrayPopAt(array, index, value_ptr)

#define DArrayClear(array) \
    _DArrayFieldSet(array, DARRAY_LENGTH, 0)

#define DArrayCapacity(array) \
    _DArrayFieldGet(array, DARRAY_CAPACITY)

#define DArrayLength(array) \
    _DArrayFieldGet(array, DARRAY_LENGTH)

#define DArrayStride(array) \
    _DArrayFieldGet(array, DARRAY_STRIDE)

#define DArrayLengthSet(array, value) \
    _DArrayFieldSet(array, DARRAY_LENGTH, value)