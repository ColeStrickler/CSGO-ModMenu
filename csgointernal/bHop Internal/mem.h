#pragma once
#include <Windows.h>


//https://guidedhacking.com/threads/how-to-hack-any-game-first-internal-hack-dll-tutorial.12142/

namespace mem
{
	// internal and external versions
	//internal
	void Patch(BYTE* dst, BYTE* src, unsigned int size);
	// external
	void PatchEx(BYTE* dst, BYTE* src, unsigned int size, HANDLE hProcess);
	void Nop(BYTE* dst, unsigned int size);
	void NopEx(BYTE* dst, unsigned int size, HANDLE hProcess);
}