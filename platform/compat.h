#pragma once
#include <SDL.h>
#include <cstdio>
#include <cwchar>
#include <cstdarg>
#include <algorithm>
#include <cstdlib>
using std::min;
using std::max;

typedef bool BOOL;
typedef wchar_t TCHAR;
#define TRUE true
#define FALSE false
inline void OutputDebugString(const wchar_t* s) {
    char text[1024];
    const size_t length = std::wcstombs(text, s, sizeof(text) - 1);
    if (length != static_cast<size_t>(-1)) { text[length] = 0; std::fputs(text, stderr); }
}
inline void OutputDebugStringA(const char* s) { std::fputs(s, stderr); }
template <size_t N, typename... Args>
int wsprintf(wchar_t (&out)[N], const wchar_t* format, Args... args) {
    return std::swprintf(out, N, format, args...);
}
inline void Sleep(unsigned ms) { SDL_Delay(ms); }
