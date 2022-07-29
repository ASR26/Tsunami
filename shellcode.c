#include <stdio.h>
#include <windows.h>

int main()
{
	LoadLibrary("msvcrt.dll");
	_asm
	{
		push ebp
		mov ebp,esp
		xor edi,edi
		push edi
		sub esp,04h
		mov byte ptr [ebp-08h],63h
		mov byte ptr [ebp-07h],61h
        mov byte ptr [ebp-06h],6Ch
        mov byte ptr [ebp-05h],63h
        mov byte ptr [ebp-04h],2Eh
        mov byte ptr [ebp-03h],65h
        mov byte ptr [ebp-02h],78h
		mov byte ptr [ebp-01h],65h
		lea eax,[ebp-08h]
		push eax
		mov ebx,0x77bf8044
		call ebx
	}
}
