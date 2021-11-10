#pragma once
#include <iostream>
#include <cstdint>
#include <cctype>
#include <memory>

#define REAL_MAX FLT_MAX
#define REAL_MIN -FLT_MAX

#define Kilobytes(val) (val * 1024LL)
#define Megabytes(val) (Kilobytes(val) * 1024LL)
#define Gigabytes(val) (Megabytes(val) * 1024LL)

#define SetABit(x) (1 << x)

#define Assert(val, msg)			\
	if (!(val))						\
	{								\
		std::cout << (msg) << std::endl;		\
		*(int *)NULL = 0;			\
	} 

#define LOG(msg) std::cout << msg << std::endl;
#define LOGTS(msg) std::cout << ToString(msg).GetCStr() << std::endl;
#define LOGTODO(msg) std::cout << "TODO:" << __func__ << " "<< msg << std::endl;

#define EDITOR 1
#define USE_RAW_ASSETS 1

#define VOXEL_IMPORT_SCALE 0.05f
#define MODEL_IMPORT_SCALE 0.5f

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef bool bool32;
typedef float real32;
typedef double real64;

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#define PushStruct(Arena, type) (type *)PushSize_(Arena, sizeof(type))
#define PushArray(Arena, Count, type) (type *)PushSize_(Arena, (Count)*sizeof(type))
#define PushSize(Arena, size) (void *)PushSize_(Arena, size)
#define ZeroStruct(ptr_data) memset(ptr_data, 0, sizeof(*ptr_data))
#define ZeroArray(ptr_data) memset(ptr_data, 0, sizeof(ptr_data));
#define ZeroArrayCount(ptr_data, count) memset(ptr_data, 0, count);

//#include <atomic>
#include <mutex>
//typedef std::atomic<bool> AtomicBool;

#include <string>
#include <sstream>
namespace cm
{
	template<typename T>
	inline void ArrayRemove(T* arr, int32 removeIndex, int32 arraySize)
	{
		Assert(removeIndex >= 0, "Array index is incorrect");
		for (int32 i = removeIndex; i < arraySize - 1; i++)
		{
			arr[i] = arr[i + 1];
		}
	}

	template<typename T>
	inline bool32 ByteCompareBetweenStructs(T* a1, T* a2)
	{
		return memcmp((void*)a1, (void*)a2, sizeof(T)) == 0;
	}

}