#pragma once

#include "MUtil.h"

// A blob array is a serialized version of an array of structs,
// suitable for transmitting over the network.
//
// It contains...
// - A 4 byte integer, giving the size of a single element (OneBlobSize).
// - A 4 byte integer, giving the number of elements (BlobCount).
// - BlobCount amount of OneBlobSize-byte large elements.

// Construction and destruction
void* MMakeBlobArray(int nOneBlobSize, int nBlobCount);
void MEraseBlobArray(void* pBlob);

const void* MGetBlobArrayElement(const void* pBlob, int i);
inline void* MGetBlobArrayElement(void* pBlob, int i) {
	return const_cast<void*>(MGetBlobArrayElement(static_cast<const void*>(pBlob), i)); }

int MGetBlobArrayCount(const void* pBlob);
int MGetBlobArraySize(const void* pBlob);

const void* MGetBlobArrayPointer(const void* pBlob);
inline void* MGetBlobArrayPointer(void* pBlob) {
	return const_cast<void*>(MGetBlobArrayPointer(static_cast<const void*>(pBlob))); }

size_t MGetBlobArrayInfoSize();
size_t MGetBlobArrayElementSize(const void* pBlob);

template <typename T>
auto MGetBlobArrayRange(void* pBlob)
{
	auto* ArrayPtr = MGetBlobArrayPointer(pBlob);
	const auto ArraySize = MGetBlobArrayCount(pBlob);
	return Range<T*>{ ArrayPtr, ArrayPtr + ArraySize };
}

// Validates that the blob array's count info values
// exist and match the total blob array size.
bool MValidateBlobArraySize(const void* pBlob, size_t Size);