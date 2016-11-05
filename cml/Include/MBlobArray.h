#pragma once

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
