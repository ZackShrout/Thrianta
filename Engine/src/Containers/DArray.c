#include "Containers/DArray.h"
#include "Core/TMemory.h"
#include "Core/Logger.h"

void* _DArrayCreate(u64 length, u64 stride)
{
    u64 header_size = DARRAY_FIELD_LENGTH * sizeof(u64);
    u64 array_size = length * stride;
    u64* new_array = TAllocate(header_size + array_size, MEMORY_TAG_DARRAY);
    TSetMemory(new_array, 0, header_size + array_size);
    new_array[DARRAY_CAPACITY] = length;
    new_array[DARRAY_LENGTH] = 0;
    new_array[DARRAY_STRIDE] = stride;
    return (void*)(new_array + DARRAY_FIELD_LENGTH);
}

void _DArrayDestroy(void* array)
{
    u64* header = (u64*)array - DARRAY_FIELD_LENGTH;
    u64 header_size = DARRAY_FIELD_LENGTH * sizeof(u64);
    u64 total_size = header_size + header[DARRAY_CAPACITY] * header[DARRAY_STRIDE];
    TFree(header, total_size, MEMORY_TAG_DARRAY);
}

u64 _DArrayFieldGet(void* array, u64 field)
{
    u64* header = (u64*)array - DARRAY_FIELD_LENGTH;
    return header[field];
}

void _DArrayFieldSet(void* array, u64 field, u64 value)
{
    u64* header = (u64*)array - DARRAY_FIELD_LENGTH;
    header[field] = value;
}

void* _DArrayResize(void* array)
{
    u64 length = DArrayLength(array);
    u64 stride = DArrayStride(array);
    void* temp = _DArrayCreate(
        (DARRAY_RESIZE_FACTOR * DArrayCapacity(array)),
        stride);
    TCopyMemory(temp, array, length * stride);

    _DArrayFieldSet(temp, DARRAY_LENGTH, length);
    _DArrayDestroy(array);
    return temp;
}

void* _DArrayPush(void* array, const void* value_ptr)
{
    u64 length = DArrayLength(array);
    u64 stride = DArrayStride(array);
    if (length >= DArrayCapacity(array))
    {
        array = _DArrayResize(array);
    }

    u64 addr = (u64)array;
    addr += (length * stride);
    TCopyMemory((void*)addr, value_ptr, stride);
    _DArrayFieldSet(array, DARRAY_LENGTH, length + 1);
    return array;
}

void _DArrayPop(void* array, void* dest)
{
    u64 length = DArrayLength(array);
    u64 stride = DArrayStride(array);
    u64 addr = (u64)array;
    addr += ((length - 1) * stride);
    TCopyMemory(dest, (void*)addr, stride);
    _DArrayFieldSet(array, DARRAY_LENGTH, length - 1);
}

void* _DArrayPopAt(void* array, u64 index, void* dest)
{
    u64 length = DArrayLength(array);
    u64 stride = DArrayStride(array);
    if (index >= length)
    {
        TERROR("Index outside the bounds of this array! Length: %i, index: %index", length, index);
        return array;
    }

    u64 addr = (u64)array;
    TCopyMemory(dest, (void*)(addr + (index * stride)), stride);

    // If not on the last element, snip out the entry and copy the rest inward.
    if (index != length - 1)
    {
        TCopyMemory(
            (void*)(addr + (index * stride)),
            (void*)(addr + ((index + 1) * stride)),
            stride * (length - index));
    }

    _DArrayFieldSet(array, DARRAY_LENGTH, length - 1);
    return array;
}

void* _DArrayInsertAt(void* array, u64 index, void* value_ptr)
{
    u64 length = DArrayLength(array);
    u64 stride = DArrayStride(array);
    if (index >= length)
    {
        TERROR("Index outside the bounds of this array! Length: %i, index: %index", length, index);
        return array;
    }
    
    if (length >= DArrayCapacity(array))
    {
        array = _DArrayResize(array);
    }

    u64 addr = (u64)array;

    // If not on the last element, copy the rest outward.
    if (index != length - 1)
    {
        TCopyMemory(
            (void*)(addr + ((index + 1) * stride)),
            (void*)(addr + (index * stride)),
            stride * (length - index));
    }

    // Set the value at the index
    TCopyMemory((void*)(addr + (index * stride)), value_ptr, stride);

    _DArrayFieldSet(array, DARRAY_LENGTH, length + 1);
    return array;
}