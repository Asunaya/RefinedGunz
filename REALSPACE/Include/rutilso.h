#ifndef RUTILSO_H
#define RUTILSO_H

#include "rsutils.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void DotProductASM(rvector&,rvector&,float&);
extern void TransformVertexASM(rvertex&,rmatrix44&);

#ifdef __cplusplus
}
#endif

#endif