#pragma once
#include <Windows.h>
#include <d3d9.h>
#include <d3dx9.h>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#include "dx.h"
#include "trampoline.h"
#include "draw.h"


#define ABS(x) ((x < 0) ? (-x) : (x))
#define TORAD(x) ((x) * 0.01745329252)

