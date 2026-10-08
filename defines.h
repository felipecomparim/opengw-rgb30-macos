#ifndef DEFINES_H
#define DEFINES_H

#pragma once

#ifdef _WIN32
#include "targetver.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include <GL/gl.h>
#include <GL/glu.h>
#else
#include "platform/compat.h"
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#endif
#include <stdlib.h>
#include <memory.h>

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

/*
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

#ifndef GL_GENERATE_MIPMAP
#define GL_GENERATE_MIPMAP 0x8191
#endif

#ifndef GL_RENDERBUFFER
#define GL_RENDERBUFFER 0x8D41
#endif
*/

#define PI (3.14159)
#define RADIAN (PI*2)


#endif // DEFINES_H
