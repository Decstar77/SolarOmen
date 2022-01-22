#pragma once
#include <SolarEngine.h>

#include <vector>

#ifndef DISABLE_COPY
#define DISABLE_COPY(T)                     \
            explicit T(const T&) = delete;  \
            T& operator=(const T&) = delete;
#endif

#ifndef DISABLE_MOVE
#define DISABLE_MOVE(T)                 \
            explicit T(T&&) = delete;   \
            T& operator=(T&&) = delete;
#endif

#ifndef DISABLE_COPY_AND_MOVE
#define DISABLE_COPY_AND_MOVE(T) DISABLE_COPY(T) DISABLE_MOVE(T)
#endif

template<typename T>
std::vector<T> CombineStdVectors(const std::vector<T>& v1, const std::vector<T>& v2)
{
	std::vector<T> v = v1;
	v.insert(v.end(), v2.begin(), v2.end());
	return v;
}

template<typename T, typename... Args>
std::vector<T> CombineStdVectors(const std::vector<T>& v1, const Args &... args)
{
	std::vector<T> v = v1;
	std::vector<T> v2 = CombineStdVectors(args...);
	v.insert(v.end(), v2.begin(), v2.end());
	return v;
}

#include <iostream>
#define LOG(msg) std::cout << msg << std::endl;