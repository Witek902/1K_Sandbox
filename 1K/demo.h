#pragma once

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>

//#define FORCE_INLINE
#define FORCE_INLINE __forceinline

//////////////////////////////////////////////////////////////////////////

#define IMAGE_SIZE 512

//////////////////////////////////////////////////////////////////////////

#define CLIP(X) ( (X) > 255 ? 255 : (X) < 0 ? 0 : X)

// YCbCr -> RGB
#define CONVERT_YCbCr2R(Y, Cb, Cr) CLIP(Y + ((3 * (Cr - 128) - (Cb - 128)) >> 1))
#define CONVERT_YCbCr2G(Y, Cb, Cr) CLIP(Y - ((1 * (Cr - 128) + (Cb - 128)) >> 1))
#define CONVERT_YCbCr2B(Y, Cb, Cr) CLIP(Y + ((3 * (Cb - 128) - (Cr - 128)) >> 1))

//////////////////////////////////////////////////////////////////////////

using uint8 = unsigned char;
using uint32 = unsigned int;
using uint16 = unsigned short;
using int8 = char;
using int32 = int;
using int16 = short;

//////////////////////////////////////////////////////////////////////////

