#pragma once
#ifdef SOL_BUILD_DLL
#define SOL_API _declspec(dllexport)
#else 
#define SOL_API _declspec(dllimport)
#endif