// Copyright (C) 2003-2008 Dolphin Project.

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
#ifndef _VIDEOINTERFACE_H
#define _VIDEOINTERFACE_H

#include "Common.h"

namespace VideoInterface
{
	enum VIInterruptType
	{
		INT_PRERETRACE		= 0,
		INT_POSTRETRACE		= 1,
		INT_REG_2,
		INT_REG_3,
	};

	// Init
	void Init();	

	// Read
	void HWCALL Read16(u16& _uReturnValue, const u32 _uAddress);
	void HWCALL Read32(u32& _uReturnValue, const u32 _uAddress);
				
	// Write
	void HWCALL Write16(const u16 _uValue, const u32 _uAddress);
	void HWCALL Write32(const u32 _uValue, const u32 _uAddress);	
				
	void GenerateVIInterrupt(VIInterruptType _VIInterrupt);	

	// returns a pointer to the framebuffer
	u8* GetFrameBufferPointer();

	// pre init
	void PreInit(bool _bNTSC);

	// VI Unknown Regs
	extern u8 m_UVIUnknownRegs[0x1000];


    // ??????
    void Update();


	// UpdateInterrupts: check if we have to generate a new VI Interrupt
	void UpdateInterrupts();	


    // ??????
    void UpdateTiming();
};

#endif

