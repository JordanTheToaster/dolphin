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
#ifndef _X64ANALYZER_H
#define _X64ANALYZER_H

#include "Common.h"

struct InstructionInfo
{
	int operandSize; //8, 16, 32, 64
	int instructionSize;
	int regOperandReg;
	int otherReg;
	int scaledReg;
	bool zeroExtend;
	bool hasImmediate;
	u64 immediate;
	s32 displacement;
};

struct ModRM
{
	int mod, reg, rm;
	ModRM(u8 modRM, u8 rex)
	{
		mod = modRM >> 6;
		reg = ((modRM >> 3) & 7) | ((rex & 4)?8:0);
		rm = modRM & 7;
	}
};


enum AccessType{
	OP_ACCESS_READ = 0,
	OP_ACCESS_WRITE = 1
};

bool DisassembleMov(const unsigned char *codePtr, InstructionInfo &info, int accessType);

#endif
