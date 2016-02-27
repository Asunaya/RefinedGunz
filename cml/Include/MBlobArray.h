// 특정 메모리 블럭의 배열과 그 정보를 하나의 메모리 블럭에 넣기 위한 함수들
#ifndef MBLOBARRAY_H
#define MBLOBARRAY_H

/// nOneBlobSize만큼 nBlobCount갯수만큼 배열한 블럭 만들기
void* MMakeBlobArray(int nOneBlobSize, int nBlobCount);
/// 블럭 지우기
void MEraseBlobArray(void* pBlob);
/// 블럭에서 각 원소 얻어내기
void* MGetBlobArrayElement(void* pBlob, int i);
/// 블럭의 갯수 얻어내기
int MGetBlobArrayCount(void* pBlob);
/// 블걱 전체 사이즈 얻어내기
int MGetBlobArraySize(void* pBlob);
/// Array Block으로 보고 맨 처음 포인터 얻어내기
void* MGetBlobArrayPointer(void* pBlob);

#endif