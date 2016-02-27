.686
.XMM

public _PIII_Mult00_4x4_4x4

_TEXT SEGMENT byte public '.rel' USE32
_PIII_Mult00_4x4_4x4 proc

        push ebp        ; Save EBP
        mov ebp, esp    ; Move ESP into EBP so we can refer
                        ;   to arguments on the stack

	mov edx, dword ptr [esp+8] ; src1
	mov eax, dword ptr [esp+10h] ; dst
	mov ecx, dword ptr [esp+0ch] ; src2
	movss xmm0, dword ptr [edx]
	movaps xmm1, OWORD ptr [ecx]
	shufps xmm0, xmm0, 0
	movss xmm2, dword ptr [edx+4]
	mulps xmm0, xmm1
	shufps xmm2, xmm2, 0
	movaps xmm3, OWORD ptr [ecx+10h]
	movss xmm7, dword ptr [edx+8]
	mulps xmm2, xmm3
	shufps xmm7, xmm7, 0
	addps xmm0, xmm2
	movaps xmm4, OWORD ptr [ecx+20h]
	movss xmm2, dword ptr [edx+0Ch]
	mulps xmm7, xmm4
	shufps xmm2, xmm2, 0
	addps xmm0, xmm7
	movaps xmm5, OWORD ptr [ecx+30h]
	movss xmm6, dword ptr [edx+10h]
	mulps xmm2, xmm5
	movss xmm7, dword ptr [edx+14h]
	shufps xmm6, xmm6, 0
	addps xmm0, xmm2
	shufps xmm7, xmm7, 0
	movlps qword ptr [eax], xmm0
	movhps qword ptr [eax+8], xmm0
	mulps xmm7, xmm3
	movss xmm0, dword ptr [edx+18h]
	mulps xmm6, xmm1
	shufps xmm0, xmm0, 0
	addps xmm6, xmm7
	mulps xmm0, xmm4
	movss xmm2, dword ptr [edx+24h]
	addps xmm6, xmm0
	movss xmm0, dword ptr [edx+1Ch]
	movss xmm7, dword ptr [edx+20h]
	shufps xmm0, xmm0, 0
	shufps xmm7, xmm7, 0
	mulps xmm0, xmm5
	mulps xmm7, xmm1
	addps xmm6, xmm0
	shufps xmm2, xmm2, 0
	movlps qword ptr [eax+10h], xmm6
	movhps qword ptr [eax+18h], xmm6
	mulps xmm2, xmm3
	movss xmm6, dword ptr [edx+28h]
	addps xmm7, xmm2
	shufps xmm6, xmm6, 0
	movss xmm2, dword ptr [edx+2Ch]
	mulps xmm6, xmm4
	shufps xmm2, xmm2, 0
	addps xmm7, xmm6
	mulps xmm2, xmm5
	movss xmm0, dword ptr [edx+34h]
	addps xmm7, xmm2
	shufps xmm0, xmm0, 0
	movlps qword ptr [eax+20h], xmm7
	movss xmm2, dword ptr [edx+30h]
	movhps qword ptr [eax+28h], xmm7
	mulps xmm0, xmm3
	shufps xmm2, xmm2, 0
	movss xmm6, dword ptr [edx+38h]
	mulps xmm2, xmm1
	shufps xmm6, xmm6, 0
	addps xmm2, xmm0
	mulps xmm6, xmm4
	movss xmm7, dword ptr [edx+3Ch]
	shufps xmm7, xmm7, 0
	addps xmm2, xmm6
	mulps xmm7, xmm5
	addps xmm2, xmm7
	movaps OWORD ptr [eax+30h], xmm2

    pop ebp         ; Restore EBP
   	ret
_PIII_Mult00_4x4_4x4 endp
_TEXT ends

end