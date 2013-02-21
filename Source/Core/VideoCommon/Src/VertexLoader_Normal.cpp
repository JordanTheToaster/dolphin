// Copyright (C) 2003 Dolphin Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official SVN repository and contact information can be found at
// http://code.google.com/p/dolphin-emu/

#include "Common.h"
#include "VideoCommon.h"
#include "VertexLoader.h"
#include "VertexLoader_Normal.h"
#include "VertexManagerBase.h"
#include "CPUDetect.h"
#include <cmath>
#include <limits>

#if _M_SSE >= 0x401
#include <smmintrin.h>
#include <emmintrin.h>
#elif _M_SSE >= 0x301 && !(defined __GNUC__ && !defined __SSSE3__)
#include <tmmintrin.h>
#endif

#define LOG_NORM()  // PRIM_LOG("norm: %f %f %f, ", ((float*)VertexManager::s_pCurBufferPointer)[-3], ((float*)VertexManager::s_pCurBufferPointer)[-2], ((float*)VertexManager::s_pCurBufferPointer)[-1]);

VertexLoader_Normal::Set VertexLoader_Normal::m_Table[NUM_NRM_TYPE][NUM_NRM_INDICES][NUM_NRM_ELEMENTS][NUM_NRM_FORMAT];

namespace
{

template <typename T>
float FracAdjust(T val)
{
	//auto const S8FRAC = 1.f / (1u << 6);
	//auto const U8FRAC = 1.f / (1u << 7);
	//auto const S16FRAC = 1.f / (1u << 14);
	//auto const U16FRAC = 1.f / (1u << 15);
	
	return val / float(1u << (sizeof(T) * 8 - std::numeric_limits<T>::is_signed - 1));
}

template <>
float FracAdjust(float val)
{ return val; }

template <typename T, int N>
inline void ReadIndirect(const T* data)
{
	static_assert(3 == N || 9 == N, "N is only sane as 3 or 9!");
	
	auto const dest = reinterpret_cast<float*>(VertexManager::s_pCurBufferPointer);

	for (int i = 0; i != N; ++i)
	{
		dest[i] = FracAdjust(Common::FromBigEndian(data[i]));
		LOG_NORM();
	}
	
	VertexManager::s_pCurBufferPointer += sizeof(float) * N;
}

template <typename T, int N>
void LOADERDECL Normal_Direct()
{
	auto const source = reinterpret_cast<const T*>(DataGetPosition());
	ReadIndirect<T, N * 3>(source);
	DataSkip<N * 3 * sizeof(T)>();
}

template <typename I, typename T, int N, int Offset>
inline void Normal_Index_Offset()
{
	static_assert(!std::numeric_limits<I>::is_signed, "Only unsigned I is sane!");
	
	auto const index = DataRead<I>();
	auto const data = reinterpret_cast<const T*>(cached_arraybases[ARRAY_NORMAL]
		+ (index * arraystrides[ARRAY_NORMAL]) + sizeof(T) * 3 * Offset);
	ReadIndirect<T, N * 3>(data);
}

template <typename I, typename T, int N>
void LOADERDECL Normal_Index()
{
	Normal_Index_Offset<I, T, N, 0>();
}

template <typename I, typename T>
void LOADERDECL Normal_Index_Indices3()
{
	Normal_Index_Offset<I, T, 1, 0>();
	Normal_Index_Offset<I, T, 1, 1>();
	Normal_Index_Offset<I, T, 1, 2>();
}

}

void VertexLoader_Normal::Init(void)
{
	// HACK is for signed instead of unsigned to prevent crashes.
    m_Table[NRM_DIRECT] [NRM_INDICES1][NRM_NBT] [FORMAT_UBYTE] 	= Set(3,  Normal_Direct<u8, 1>); //HACK
    m_Table[NRM_DIRECT] [NRM_INDICES1][NRM_NBT] [FORMAT_BYTE]   = Set(3,  Normal_Direct<s8, 1>);
    m_Table[NRM_DIRECT] [NRM_INDICES1][NRM_NBT] [FORMAT_USHORT]	= Set(6,  Normal_Direct<u16, 1>); //HACK
    m_Table[NRM_DIRECT] [NRM_INDICES1][NRM_NBT] [FORMAT_SHORT] 	= Set(6,  Normal_Direct<s16, 1>);
    m_Table[NRM_DIRECT] [NRM_INDICES1][NRM_NBT] [FORMAT_FLOAT] 	= Set(12, Normal_Direct<float, 1>);
    m_Table[NRM_DIRECT] [NRM_INDICES1][NRM_NBT3][FORMAT_UBYTE] 	= Set(9,  Normal_Direct<u8, 3>); //HACK	
    m_Table[NRM_DIRECT] [NRM_INDICES1][NRM_NBT3][FORMAT_BYTE]  	= Set(9,  Normal_Direct<s8, 3>);
    m_Table[NRM_DIRECT] [NRM_INDICES1][NRM_NBT3][FORMAT_USHORT]	= Set(18, Normal_Direct<u16, 3>); //HACK
    m_Table[NRM_DIRECT] [NRM_INDICES1][NRM_NBT3][FORMAT_SHORT] 	= Set(18, Normal_Direct<s16, 3>);
    m_Table[NRM_DIRECT] [NRM_INDICES1][NRM_NBT3][FORMAT_FLOAT] 	= Set(36, Normal_Direct<float, 3>);
								  			
	m_Table[NRM_DIRECT] [NRM_INDICES3][NRM_NBT] [FORMAT_UBYTE] 	= Set(3,  Normal_Direct<u8, 1>); //HACK
    m_Table[NRM_DIRECT] [NRM_INDICES3][NRM_NBT] [FORMAT_BYTE]  	= Set(3,  Normal_Direct<s8, 1>);
    m_Table[NRM_DIRECT] [NRM_INDICES3][NRM_NBT] [FORMAT_USHORT]	= Set(6,  Normal_Direct<u16, 1>); //HACK
    m_Table[NRM_DIRECT] [NRM_INDICES3][NRM_NBT] [FORMAT_SHORT] 	= Set(6,  Normal_Direct<s16, 1>);
    m_Table[NRM_DIRECT] [NRM_INDICES3][NRM_NBT] [FORMAT_FLOAT] 	= Set(12, Normal_Direct<float, 1>);
    m_Table[NRM_DIRECT] [NRM_INDICES3][NRM_NBT3][FORMAT_UBYTE] 	= Set(9,  Normal_Direct<u8, 3>); //HACK	
    m_Table[NRM_DIRECT] [NRM_INDICES3][NRM_NBT3][FORMAT_BYTE]  	= Set(9,  Normal_Direct<s8, 3>);
    m_Table[NRM_DIRECT] [NRM_INDICES3][NRM_NBT3][FORMAT_USHORT]	= Set(18, Normal_Direct<u16, 3>); //HACK
    m_Table[NRM_DIRECT] [NRM_INDICES3][NRM_NBT3][FORMAT_SHORT] 	= Set(18, Normal_Direct<s16, 3>);
    m_Table[NRM_DIRECT] [NRM_INDICES3][NRM_NBT3][FORMAT_FLOAT] 	= Set(36, Normal_Direct<float, 3>);
								  			
    m_Table[NRM_INDEX8] [NRM_INDICES1][NRM_NBT] [FORMAT_UBYTE] 	= Set(1,  Normal_Index<u8, u8, 1>); //HACK
    m_Table[NRM_INDEX8] [NRM_INDICES1][NRM_NBT] [FORMAT_BYTE]  	= Set(1,  Normal_Index<u8, s8, 1>);
    m_Table[NRM_INDEX8] [NRM_INDICES1][NRM_NBT] [FORMAT_USHORT]	= Set(1,  Normal_Index<u8, u16, 1>); //HACK
    m_Table[NRM_INDEX8] [NRM_INDICES1][NRM_NBT] [FORMAT_SHORT] 	= Set(1,  Normal_Index<u8, s16, 1>);
    m_Table[NRM_INDEX8] [NRM_INDICES1][NRM_NBT] [FORMAT_FLOAT] 	= Set(1,  Normal_Index<u8, float, 1>);
    m_Table[NRM_INDEX8] [NRM_INDICES1][NRM_NBT3][FORMAT_UBYTE] 	= Set(1,  Normal_Index<u8, u8, 3>); //HACK	
    m_Table[NRM_INDEX8] [NRM_INDICES1][NRM_NBT3][FORMAT_BYTE]  	= Set(1,  Normal_Index<u8, s8, 3>);
    m_Table[NRM_INDEX8] [NRM_INDICES1][NRM_NBT3][FORMAT_USHORT]	= Set(1,  Normal_Index<u8, u16, 3>); //HACK
    m_Table[NRM_INDEX8] [NRM_INDICES1][NRM_NBT3][FORMAT_SHORT] 	= Set(1,  Normal_Index<u8, s16, 3>);
    m_Table[NRM_INDEX8] [NRM_INDICES1][NRM_NBT3][FORMAT_FLOAT] 	= Set(1,  Normal_Index<u8, float, 3>);
								  			
	m_Table[NRM_INDEX8] [NRM_INDICES3][NRM_NBT] [FORMAT_UBYTE] 	= Set(1,  Normal_Index<u8, u8, 1>); //HACK
    m_Table[NRM_INDEX8] [NRM_INDICES3][NRM_NBT] [FORMAT_BYTE]  	= Set(1,  Normal_Index<u8, s8, 1>);
    m_Table[NRM_INDEX8] [NRM_INDICES3][NRM_NBT] [FORMAT_USHORT]	= Set(1,  Normal_Index<u8, u16, 1>); //HACK
    m_Table[NRM_INDEX8] [NRM_INDICES3][NRM_NBT] [FORMAT_SHORT] 	= Set(1,  Normal_Index<u8, s16, 1>);
    m_Table[NRM_INDEX8] [NRM_INDICES3][NRM_NBT] [FORMAT_FLOAT] 	= Set(1,  Normal_Index<u8, float, 1>);
    m_Table[NRM_INDEX8] [NRM_INDICES3][NRM_NBT3][FORMAT_UBYTE] 	= Set(3,  Normal_Index_Indices3<u8, u8>); //HACK	
    m_Table[NRM_INDEX8] [NRM_INDICES3][NRM_NBT3][FORMAT_BYTE]  	= Set(3,  Normal_Index_Indices3<u8, s8>);
    m_Table[NRM_INDEX8] [NRM_INDICES3][NRM_NBT3][FORMAT_USHORT]	= Set(3,  Normal_Index_Indices3<u8, u16>); //HACK
    m_Table[NRM_INDEX8] [NRM_INDICES3][NRM_NBT3][FORMAT_SHORT] 	= Set(3,  Normal_Index_Indices3<u8, s16>);
    m_Table[NRM_INDEX8] [NRM_INDICES3][NRM_NBT3][FORMAT_FLOAT] 	= Set(3,  Normal_Index_Indices3<u8, float>);
								  												  			
    m_Table[NRM_INDEX16][NRM_INDICES1][NRM_NBT] [FORMAT_UBYTE] 	= Set(2,  Normal_Index<u16, u8, 1>); //HACK
    m_Table[NRM_INDEX16][NRM_INDICES1][NRM_NBT] [FORMAT_BYTE]  	= Set(2,  Normal_Index<u16, s8, 1>);
    m_Table[NRM_INDEX16][NRM_INDICES1][NRM_NBT] [FORMAT_USHORT]	= Set(2,  Normal_Index<u16, u16, 1>); //HACK
    m_Table[NRM_INDEX16][NRM_INDICES1][NRM_NBT] [FORMAT_SHORT] 	= Set(2,  Normal_Index<u16, s16, 1>);
    m_Table[NRM_INDEX16][NRM_INDICES1][NRM_NBT] [FORMAT_FLOAT] 	= Set(2,  Normal_Index<u16, float, 1>);
    m_Table[NRM_INDEX16][NRM_INDICES1][NRM_NBT3][FORMAT_UBYTE] 	= Set(2,  Normal_Index<u16, u8, 3>); //HACK
    m_Table[NRM_INDEX16][NRM_INDICES1][NRM_NBT3][FORMAT_BYTE]  	= Set(2,  Normal_Index<u16, s8, 3>);
    m_Table[NRM_INDEX16][NRM_INDICES1][NRM_NBT3][FORMAT_USHORT]	= Set(2,  Normal_Index<u16, u16, 3>); //HACK
    m_Table[NRM_INDEX16][NRM_INDICES1][NRM_NBT3][FORMAT_SHORT] 	= Set(2,  Normal_Index<u16, s16, 3>);
    m_Table[NRM_INDEX16][NRM_INDICES1][NRM_NBT3][FORMAT_FLOAT] 	= Set(2,  Normal_Index<u16, float, 3>);
								  			
	m_Table[NRM_INDEX16][NRM_INDICES3][NRM_NBT] [FORMAT_UBYTE] 	= Set(2,  Normal_Index<u16, u8, 1>); //HACK
    m_Table[NRM_INDEX16][NRM_INDICES3][NRM_NBT] [FORMAT_BYTE]  	= Set(2,  Normal_Index<u16, s8, 1>);
    m_Table[NRM_INDEX16][NRM_INDICES3][NRM_NBT] [FORMAT_USHORT]	= Set(2,  Normal_Index<u16, u16, 1>); //HACK
    m_Table[NRM_INDEX16][NRM_INDICES3][NRM_NBT] [FORMAT_SHORT] 	= Set(2,  Normal_Index<u16, s16, 1>);
    m_Table[NRM_INDEX16][NRM_INDICES3][NRM_NBT] [FORMAT_FLOAT] 	= Set(2,  Normal_Index<u16, float, 1>);
    m_Table[NRM_INDEX16][NRM_INDICES3][NRM_NBT3][FORMAT_UBYTE] 	= Set(6,  Normal_Index_Indices3<u16, u8>); //HACK	
    m_Table[NRM_INDEX16][NRM_INDICES3][NRM_NBT3][FORMAT_BYTE]  	= Set(6,  Normal_Index_Indices3<u16, s8>);
    m_Table[NRM_INDEX16][NRM_INDICES3][NRM_NBT3][FORMAT_USHORT]	= Set(6,  Normal_Index_Indices3<u16, u16>); //HACK
    m_Table[NRM_INDEX16][NRM_INDICES3][NRM_NBT3][FORMAT_SHORT] 	= Set(6,  Normal_Index_Indices3<u16, s16>);
    m_Table[NRM_INDEX16][NRM_INDICES3][NRM_NBT3][FORMAT_FLOAT] 	= Set(6,  Normal_Index_Indices3<u16, float>);
}

unsigned int VertexLoader_Normal::GetSize(unsigned int _type,
	unsigned int _format, unsigned int _elements, unsigned int _index3)
{
	return m_Table[_type][_index3][_elements][_format].gc_size;
}

TPipelineFunction VertexLoader_Normal::GetFunction(unsigned int _type,
	unsigned int _format, unsigned int _elements, unsigned int _index3)
{
	TPipelineFunction pFunc = m_Table[_type][_index3][_elements][_format].function;
    return pFunc;
}
