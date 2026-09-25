
; ===========================================================================
; getsurf.asm
; 
; DOOM-Style Assembly Fallbacks for Ignition (1997)
; 
; These routines contain the exact 1997 assembly instructions extracted 
; from MAINDOS_32BIT.EXE. They are used to guarantee a 100% byte-match 
; during the recompilation process. 
;
; The functionally equivalent pure C versions are preserved in getsurf.c 
; (wrapped in #if 0) for use in modern, portable source ports.
; ===========================================================================

.386
.model flat
.code
PUBLIC Surface_TestTrianglePositiveDZ_
Surface_TestTrianglePositiveDZ_ PROC
    push ebp
    mov ebp, esi
    cmp ecx, 0
    je loc_20c7f
    mov esi, dword ptr [ebp]
    push edi
loc_20c24:
    mov edi, dword ptr [esi + 4]
    cmp ebx, edi
    jl loc_20c75
    movsx eax, word ptr [esi + 10h]
    add eax, edi
    xor edi, -1
    cmp eax, ebx
    jl loc_20c75
    lea eax, [ebx + edi + 1]
    mov edi, eax
    imul eax, dword ptr [esi + 8]
    sar eax, 10h
    add eax, dword ptr [esi]
    cmp eax, edx
    jg loc_20c75
    imul edi, dword ptr [esi + 0ch]
    sar edi, 10h
    add edi, dword ptr [esi]
    cmp edi, edx
    jl loc_20c75
    pop edi
    mov eax, dword ptr [esi + 14h]
    mov dword ptr [edi], eax
    xor eax, eax
    mov ax, word ptr [esi + 12h]
    add edi, 8
    shl eax, 2
    mov esi, dword ptr [esi + 14h]
    push edi
    add eax, dword ptr [esi + 4]
    nop 
    mov dword ptr [edi - 4], eax
loc_20c75:
    mov esi, dword ptr [ebp + 4]
    add ebp, 4
    dec ecx
    jne loc_20c24
    pop edi
loc_20c7f:
    pop ebp
    ret 
Surface_TestTrianglePositiveDZ_ ENDP

PUBLIC Surface_TestTriangleNegativeDZ_
Surface_TestTriangleNegativeDZ_ PROC
    push ebp
    mov ebp, esi
    cmp ecx, 0
    je loc_20ce8
    mov esi, dword ptr [ebp]
    push edi
loc_20c8d:
    mov edi, dword ptr [esi + 4]
    cmp ebx, edi
    jg loc_20cde
    movsx eax, word ptr [esi + 10h]
    add eax, edi
    xor edi, -1
    cmp eax, ebx
    jg loc_20cde
    lea eax, [ebx + edi + 1]
    mov edi, eax
    imul eax, dword ptr [esi + 8]
    sar eax, 10h
    add eax, dword ptr [esi]
    cmp eax, edx
    jg loc_20cde
    imul edi, dword ptr [esi + 0ch]
    sar edi, 10h
    add edi, dword ptr [esi]
    cmp edi, edx
    jl loc_20cde
    pop edi
    mov eax, dword ptr [esi + 14h]
    mov dword ptr [edi], eax
    xor eax, eax
    mov ax, word ptr [esi + 12h]
    add edi, 8
    shl eax, 2
    mov esi, dword ptr [esi + 14h]
    push edi
    add eax, dword ptr [esi + 4]
    nop 
    mov dword ptr [edi - 4], eax
loc_20cde:
    mov esi, dword ptr [ebp + 4]
    add ebp, 4
    dec ecx
    jne loc_20c8d
    pop edi
loc_20ce8:
    pop ebp
    ret 
Surface_TestTriangleNegativeDZ_ ENDP

PUBLIC Surface_LoadSRF_
Surface_LoadSRF_ PROC
    push ebx
    push ecx
    push esi
    push edi
    push ebp
    sub esp, 0ch
    mov esi, dword ptr ds:[0ebaach]
    mov edi, dword ptr ds:[0ebaa0h]
    mov ebp, edx
    call 60f9ch
    mov edx, dword ptr [eax]
    mov dword ptr ds:[0ebab4h], edx
    mov edx, dword ptr [eax + 4]
    mov dword ptr ds:[0ebabch], edx
    mov edx, dword ptr [eax + 8]
    mov dword ptr ds:[0eba94h], edx
    mov edx, dword ptr [eax + 0ch]
    mov dword ptr ds:[0eba90h], edx
    mov edx, dword ptr [eax + 10h]
    mov dword ptr ds:[0eba9ch], edx
    mov edx, dword ptr [eax + 14h]
    mov dword ptr ds:[0bfe80h], eax
    mov dword ptr ds:[0eba98h], edx
    mov edx, dword ptr [eax + 18h]
    add eax, 24h
    mov dword ptr [esp + 8], edx
    mov edx, dword ptr [eax - 8]
    mov dword ptr ds:[0ebac0h], eax
    mov dword ptr [esp + 4], edx
    mov edx, dword ptr [eax - 4]
    mov eax, dword ptr ds:[0eba98h]
    mov dword ptr [esp], edx
    mov edx, dword ptr ds:[0eba9ch]
    imul edx, eax
    lea eax, [edx*4]
    sub eax, edx
    mov esi, dword ptr ds:[0ebac0h]
    shl eax, 2
    mov edx, dword ptr [esp + 8]
    add esi, eax
    lea eax, [edx*4]
    sub eax, edx
    mov edx, dword ptr [esp + 4]
    shl eax, 3
    shl edx, 2
    lea edi, [esi + eax]
    lea eax, [edi + edx]
    xor ebx, ebx
    mov dword ptr ds:[0ebab0h], eax
    xor edx, edx
    jmp loc_1ffb2
loc_1ff9b:
    mov eax, dword ptr ds:[0ebac0h]
    add ebx, 0ch
    mov ecx, dword ptr ds:[0ebab0h]
    add dword ptr [ebx + eax - 0ch], ecx
    inc edx
    add dword ptr [ebx + eax - 8], edi
loc_1ffb2:
    mov eax, dword ptr ds:[0eba9ch]
    imul eax, dword ptr ds:[0eba98h]
    cmp edx, eax
    jl loc_1ff9b
    cmp dword ptr [esp + 8], 0
    jle loc_1ffee
    mov edx, dword ptr [esp + 8]
    lea ecx, [edx*4]
    sub ecx, edx
    xor eax, eax
    shl ecx, 3
    mov edx, esi
loc_1ffdd:
    mov ebx, dword ptr [edx + eax + 14h]
    add eax, 18h
    add ebx, ebp
    mov dword ptr [edx + eax - 4], ebx
    cmp eax, ecx
    jl loc_1ffdd
loc_1ffee:
    mov ecx, dword ptr [esp]
    add ecx, dword ptr [esp + 4]
    shl ecx, 2
    xor eax, eax
    test ecx, ecx
    jle loc_20010
    mov edx, edi
loc_20000:
    mov ebp, dword ptr [edx + eax]
    add eax, 4
    add ebp, esi
    mov dword ptr [edx + eax - 4], ebp
    cmp eax, ecx
    jl loc_20000
loc_20010:
    mov eax, 1
    mov dword ptr ds:[0ebaa0h], edi
    mov dword ptr ds:[0ebaach], esi
    add esp, 0ch
    pop ebp
    pop edi
    pop esi
    pop ecx
    pop ebx
    ret 
Surface_LoadSRF_ ENDP

PUBLIC Surface_Raycast_
Surface_Raycast_ PROC
    push ecx
    push esi
    push edi
    push ebp
    sub esp, 18h
    mov ebp, dword ptr ds:[0ebad0h]
    mov dword ptr [esp + 10h], eax
    mov dword ptr [esp + 0ch], edx
    mov dword ptr [esp + 14h], ebx
    mov eax, ebx
    mov edx, ebx
    mov ebx, dword ptr ds:[0eba94h]
    sar edx, 1fh
    idiv ebx
    mov ecx, dword ptr ds:[0eba9ch]
    imul ecx, eax
    mov eax, dword ptr [esp + 10h]
    mov edx, eax
    mov esi, dword ptr ds:[0eba90h]
    sar edx, 1fh
    idiv esi
    add ecx, eax
    lea edx, [ecx*4]
    sub edx, ecx
    mov eax, dword ptr ds:[0ebac0h]
    shl edx, 2
    mov edi, 0eb900h
    add eax, edx
    mov ebx, dword ptr [esp + 14h]
    mov edx, dword ptr [esp + 10h]
    mov ecx, dword ptr [eax + 8]
    mov esi, dword ptr [eax]
    sar ecx, 10h
    mov dword ptr ds:[0ebad4h], eax
    call 20c18h
    mov eax, dword ptr ds:[0ebad4h]
    mov edx, dword ptr [esp + 10h]
    mov ebx, dword ptr [esp + 14h]
    mov ecx, dword ptr [eax + 6]
    mov esi, dword ptr [eax + 4]
    sar ecx, 10h
    mov dword ptr ds:[0ebacch], edi
    call 20c81h
    mov dword ptr ds:[0ebacch], edi
    cmp edi, 0eb900h
    jne loc_208d4
    mov ebx, -1
    mov eax, 0ebad8h
    mov ebp, dword ptr ds:[0ebad0h]
    mov dword ptr ds:[0ebad8h], ebx
    jmp loc_20bab
loc_208d4:
    mov eax, 0eb900h
    mov ecx, 4fh
    mov ebp, dword ptr ds:[0ebad0h]
    mov dword ptr ds:[0ebac8h], eax
    cmp edi, 0eb908h
    jbe loc_20990
    mov esi, 3b9aca00h
    mov dword ptr ds:[0ebac4h], eax
    cmp edi, eax
    jbe loc_20990
loc_20907:
    mov ebx, dword ptr [esp + 14h]
    mov edx, dword ptr [esp + 10h]
    mov eax, dword ptr ds:[0ebac4h]
    mov dword ptr ds:[0ebad0h], ebp
    call 20bbch
    mov ebx, dword ptr [esp + 0ch]
    mov ebp, eax
    sub ebp, ebx
    test ebp, ebp
    jge loc_2092f
    neg ebp
    jmp loc_20932
loc_2092f:
    imul ebp, ebp
loc_20932:
    mov eax, dword ptr ds:[0ebac4h]
    mov eax, dword ptr [eax + 4]
    mov eax, dword ptr [eax]
    sar eax, 10h
    cmp ebp, esi
    jge loc_2094d
    cmp eax, 28h
    jl loc_20965
    cmp eax, 4fh
    jg loc_20965
loc_2094d:
    cmp ebp, esi
    jl loc_2095b
    cmp eax, 28h
    jl loc_2095b
    cmp eax, 4fh
    jle loc_20973
loc_2095b:
    cmp ecx, 28h
    jl loc_20973
    cmp ecx, 4fh
    jg loc_20973
loc_20965:
    mov ecx, eax
    mov eax, dword ptr ds:[0ebac4h]
    mov esi, ebp
    mov dword ptr ds:[0ebac8h], eax
loc_20973:
    mov edi, dword ptr ds:[0ebac4h]
    add edi, 8
    mov edx, dword ptr ds:[0ebacch]
    mov dword ptr ds:[0ebac4h], edi
    cmp edi, edx
    jb loc_20907
loc_20990:
    mov esi, dword ptr ds:[0ebac8h]
    mov ecx, dword ptr [esi]
    mov esi, dword ptr [esi + 4]
    mov eax, dword ptr [esi]
    sar eax, 10h
    mov dword ptr ds:[0ebb18h], eax
    mov eax, dword ptr [ecx + 0ch]
    mov dword ptr ds:[0ebb1ch], eax
    mov eax, dword ptr [ecx + 10h]
    mov dword ptr ds:[0ebb20h], eax
    mov eax, dword ptr [ecx + 14h]
    mov edi, dword ptr [esi + 4]
    mov dword ptr ds:[0ebb24h], eax
    lea eax, [edi*4]
    sub eax, edi
    shl eax, 2
    mov edi, dword ptr [ecx + 4]
    add eax, edi
    mov edx, dword ptr [eax + 8]
    mov dword ptr ds:[0ebb28h], edx
    mov edx, dword ptr [eax + 0ch]
    add eax, 8
    mov dword ptr ds:[0ebb2ch], edx
    mov eax, dword ptr [eax + 8]
    mov edx, dword ptr [esi + 8]
    mov dword ptr ds:[0ebb30h], eax
    lea eax, [edx*4]
    sub eax, edx
    shl eax, 2
    add eax, edi
    mov edx, dword ptr [eax + 8]
    mov dword ptr ds:[0ebb34h], edx
    mov edx, dword ptr [eax + 0ch]
    add eax, 8
    mov dword ptr ds:[0ebb38h], edx
    mov eax, dword ptr [eax + 8]
    mov edx, dword ptr [esi + 0ch]
    mov dword ptr ds:[0ebb3ch], eax
    lea eax, [edx*4]
    sub eax, edx
    shl eax, 2
    add eax, edi
    mov edx, dword ptr [eax + 8]
    add eax, 8
    mov dword ptr ds:[0ebb40h], edx
    mov edx, dword ptr [eax + 4]
    mov eax, dword ptr [eax + 8]
    mov dword ptr ds:[0ebb48h], eax
    mov eax, dword ptr [ecx]
    mov dword ptr ds:[0ebb4ch], eax
    mov eax, dword ptr [ecx + 1eh]
    mov ebx, dword ptr ds:[0ebb34h]
    sar eax, 10h
    mov esi, dword ptr ds:[0ebb38h]
    mov dword ptr ds:[0ebb50h], eax
    mov eax, dword ptr [ecx + 1ch]
    mov edi, dword ptr ds:[0ebb28h]
    sar eax, 10h
    mov ecx, dword ptr ds:[0ebb34h]
    mov dword ptr ds:[0ebb54h], eax
    sub edi, ecx
    mov ecx, dword ptr ds:[0ebb30h]
    mov eax, dword ptr ds:[0ebb3ch]
    mov dword ptr ds:[0ebb44h], edx
    sub ecx, eax
    mov eax, dword ptr ds:[0ebb40h]
    mov edx, dword ptr ds:[0ebb2ch]
    sub eax, ebx
    mov ebx, dword ptr ds:[0ebb44h]
    sub edx, esi
    sub ebx, esi
    mov esi, dword ptr ds:[0ebb48h]
    mov dword ptr [esp], ebx
    sub esi, dword ptr ds:[0ebb3ch]
    mov ebx, dword ptr ds:[0ebb18h]
    mov dword ptr ds:[0ebad8h], ebx
    mov ebx, edx
    imul ebx, esi
    imul esi, edi
    mov dword ptr [esp + 4], ebx
    mov ebx, dword ptr [esp]
    imul ebx, ecx
    imul ecx, eax
    imul eax, edx
    sub ecx, esi
    mov dword ptr ds:[0ebae0h], ecx
    imul edi, dword ptr [esp]
    mov esi, dword ptr ds:[0ebb1ch]
    sub edi, eax
    mov eax, dword ptr ds:[0ebb28h]
    mov dword ptr ds:[0ebae4h], edi
    add eax, esi
    mov edi, dword ptr ds:[0ebb20h]
    mov dword ptr ds:[0ebae8h], eax
    mov eax, dword ptr ds:[0ebb2ch]
    sub eax, edi
    mov edx, dword ptr ds:[0ebb24h]
    mov dword ptr ds:[0ebaech], eax
    mov eax, dword ptr ds:[0ebb30h]
    add eax, edx
    mov dword ptr [esp + 8], ebx
    mov dword ptr ds:[0ebaf0h], eax
    mov eax, dword ptr ds:[0ebb34h]
    mov ebx, dword ptr [esp + 4]
    add eax, esi
    sub ebx, dword ptr [esp + 8]
    mov dword ptr ds:[0ebaf4h], eax
    mov eax, dword ptr ds:[0ebb38h]
    mov dword ptr ds:[0ebadch], ebx
    sub eax, edi
    mov ebx, esi
    mov dword ptr ds:[0ebaf8h], eax
    mov eax, dword ptr ds:[0ebb3ch]
    mov ecx, edi
    add eax, edx
    mov edi, ebx
    mov dword ptr ds:[0ebafch], eax
    mov eax, dword ptr ds:[0ebb40h]
    mov esi, edx
    add eax, edi
    mov edx, ecx
    mov dword ptr ds:[0ebb00h], eax
    mov eax, dword ptr ds:[0ebb44h]
    sub eax, edx
    mov ebx, esi
    mov dword ptr ds:[0ebb04h], eax
    mov eax, dword ptr ds:[0ebb48h]
    add eax, ebx
    mov dword ptr ds:[0ebb08h], eax
    mov eax, dword ptr ds:[0ebb4ch]
    mov dword ptr ds:[0ebb0ch], eax
    mov eax, dword ptr ds:[0ebb50h]
    mov dword ptr ds:[0ebb10h], eax
    mov eax, dword ptr ds:[0ebb54h]
    mov dword ptr ds:[0ebb14h], eax
    mov eax, 0ebad8h
loc_20bab:
    mov dword ptr ds:[0ebad0h], ebp
    add esp, 18h
    pop ebp
    pop edi
    pop esi
    pop ecx
    ret 
Surface_Raycast_ ENDP

PUBLIC Surface_GetTriangleHeight_
Surface_GetTriangleHeight_ PROC
    push ecx
    push esi
    push edi
    mov ebx, dword ptr [eax + 4]
    mov ecx, dword ptr [ebx + 4]
    mov esi, dword ptr [eax]
    lea eax, [ecx*4]
    mov edx, dword ptr [esi + 4]
    sub eax, ecx
    mov ecx, dword ptr [ebx + 8]
    mov edi, dword ptr [edx + eax*4 + 0ch]
    lea eax, [ecx*4]
    sub eax, ecx
    mov ebx, dword ptr [ebx + 0ch]
    mov ecx, dword ptr [edx + eax*4 + 0ch]
    lea eax, [ebx*4]
    add edx, 8
    sub eax, ebx
    neg edi
    neg ecx
    mov edx, dword ptr [edx + eax*4 + 4]
    add edi, ecx
    neg edx
    add edx, edi
    mov ebx, 3
    mov eax, edx
    sar edx, 1fh
    idiv ebx
    add eax, dword ptr [esi + 10h]
    pop edi
    pop esi
    pop ecx
    ret 
Surface_GetTriangleHeight_ ENDP

end
