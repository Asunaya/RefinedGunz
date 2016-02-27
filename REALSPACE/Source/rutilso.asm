.486p
.387


public _DotProductASM
public _TransformVertexASM

Data segment byte public '.rel' USE32
_retfloat	dword ?
Data ends

_CODE segment para public USE32

_DotProductASM proc near	; ecx=vector edx=vector eax=return(float)

	fld		dword ptr [ecx]
	fmul	dword ptr [edx]
	fld		dword ptr [ecx+4]
	fmul	dword ptr [edx+4]
	fld		dword ptr [ecx+8]
	fmul	dword ptr [edx+8]
	fadd
	fadd
	fstp	dword ptr [eax]
	ret
_DotProductASM endp

_TransformVertexASM proc near	; ecx=vertex edx=matrix
	push ebp
	mov ebp,esp

	mov edx,[ebp+12]
	mov ecx,[ebp+8]

	mov		_retfloat,1
	fild	_retfloat

	fld		dword ptr [ecx]
	fmul	dword ptr [edx+12]
	fld		dword ptr [ecx+4]
	fmul	dword ptr [edx+12+16]
	fld		dword ptr [ecx+8]
	fmul	dword ptr [edx+12+32]
	fadd
	fadd
	fadd	dword ptr [edx+12+48]
	fdiv
	fst		dword ptr [ecx+36] ; save pw

	fld		dword ptr [ecx]
	fmul	dword ptr [edx]
	fld		dword ptr [ecx+4]
	fmul	dword ptr [edx+16]
	fld		dword ptr [ecx+8]
	fmul	dword ptr [edx+32]
	fadd
	fadd
	fadd	dword ptr [edx+48]
	fmul	st,st(1)
	fstp	dword ptr [ecx+24] ; save sx
	
	fld		dword ptr [ecx]
	fmul	dword ptr [edx+4]
	fld		dword ptr [ecx+4]
	fmul	dword ptr [edx+4+16]
	fld		dword ptr [ecx+8]
	fmul	dword ptr [edx+4+32]
	fadd
	fadd
	fadd	dword ptr [edx+4+48]
	fmul	st,st(1)
	fstp	dword ptr [ecx+28] ; save sy

	fld		dword ptr [ecx]
	fmul	dword ptr [edx+8]
	fld		dword ptr [ecx+4]
	fmul	dword ptr [edx+8+16]
	fld		dword ptr [ecx+8]
	fmul	dword ptr [edx+8+32]
	fadd
	fadd
	fadd	dword ptr [edx+8+48]
	fmul	st,st(1)
	fstp	dword ptr [ecx+32] ; save sz

	fstp	st
	pop ebp
	ret
_TransformVertexASM endp
_CODE ends

end