#pragma once
#include <Windows.h>
#include "mem.h"

void Patch(BYTE* dst, BYTE* src, unsigned int size);

bool Hook(char* src, char* dst, int len);

char* TrampHook(char* src, char* dst, unsigned int len);