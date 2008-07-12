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

#include "Common.h"

#include "WII_IPC_HLE_Device_DI.h"

#include "../HW/CPU.h"
#include "../HW/Memmap.h"
#include "../Core.h"

#include "../VolumeHandler.h"

#include "VolumeCreator.h"
#include "Filesystem.h"

// __________________________________________________________________________________________________
//
CWII_IPC_HLE_Device_di::CWII_IPC_HLE_Device_di(u32 _DeviceID, const std::string& _rDeviceName )
    : IWII_IPC_HLE_Device(_DeviceID, _rDeviceName)
    , m_pVolume(NULL)
    , m_pFileSystem(NULL)
{
    
    m_pVolume = DiscIO::CreateVolumeFromFilename(Core::GetStartupParameter().m_strFilename);
    if (m_pVolume)
        m_pFileSystem = DiscIO::CreateFileSystem(*m_pVolume);
}

// __________________________________________________________________________________________________
//
CWII_IPC_HLE_Device_di::~CWII_IPC_HLE_Device_di()
{
    delete m_pFileSystem;
    delete m_pVolume;
}

// __________________________________________________________________________________________________
//
bool 
CWII_IPC_HLE_Device_di::Open(u32 _CommandAddress)
{
    Memory::Write_U32(GetDeviceID(), _CommandAddress+4);
    return true;
}

// __________________________________________________________________________________________________
//
bool CWII_IPC_HLE_Device_di::IOCtl(u32 _CommandAddress) 
{
    LOG(WII_IPC_HLE, "*******************************");
    LOG(WII_IPC_HLE, "CWII_IPC_HLE_Device_di::IOCtl");
    LOG(WII_IPC_HLE, "*******************************");

    // DumpCommands(_CommandAddress);

	// u32 Cmd =  Memory::Read_U32(_CommandAddress + 0xC);
	// TODO: Use Cmd for something?

	u32 BufferIn =  Memory::Read_U32(_CommandAddress + 0x10);
	u32 BufferInSize =  Memory::Read_U32(_CommandAddress + 0x14);
    u32 BufferOut = Memory::Read_U32(_CommandAddress + 0x18);
    u32 BufferOutSize = Memory::Read_U32(_CommandAddress + 0x1C);

    LOG(WII_IPC_HLE, "%s - BufferIn(0x%08x, 0x%x) BufferOut(0x%08x, 0x%x)", GetDeviceName().c_str(), BufferIn, BufferInSize, BufferOut, BufferOutSize);

	u32 ReturnValue = ExecuteCommand(BufferIn, BufferInSize, BufferOut, BufferOutSize);	
    Memory::Write_U32(ReturnValue, _CommandAddress + 0x4);

/*    DumpCommands(_CommandAddress);

    LOG(WII_IPC_HLE, "InBuffer");
    DumpCommands(BufferIn, BufferInSize);

//    LOG(WII_IPC_HLE, "OutBuffer");
  //  DumpCommands(BufferOut, BufferOutSize); */

    return true;
}

// __________________________________________________________________________________________________
//
bool CWII_IPC_HLE_Device_di::IOCtlV(u32 _CommandAddress) 
{  
    PanicAlert("CWII_IPC_HLE_Device_di::IOCtlV() unknown");

    DumpCommands(_CommandAddress);

    return true;
}

// __________________________________________________________________________________________________
//
u32 CWII_IPC_HLE_Device_di::ExecuteCommand(u32 _BufferIn, u32 _BufferInSize, u32 _BufferOut, u32 _BufferOutSize)
{    
    u32 Command = Memory::Read_U32(_BufferIn) >> 24;
	switch (Command)
	{
	// DVDLowInquiry
	case 0x12:
		{
			Memory::Memset(_BufferOut, 0, _BufferOutSize);
			LOG(WII_IPC_HLE, "%s executes DVDLowInquiry (Buffer 0x%08x, 0x%x)", GetDeviceName().c_str(), _BufferOut, _BufferOutSize);

            return 0x1;			
		}
		break;
	
    // DVDLowRead
    case 0x71:
        {                                 
            u32 Size = Memory::Read_U32(_BufferIn+0x04);
            u64 DVDAddress = (u64)Memory::Read_U32(_BufferIn+0x08) << 2;

            const char* pFilename = m_pFileSystem->GetFileName(DVDAddress);
            if (pFilename != NULL)
            {
                LOG(WII_IPC_HLE, "    DVDLowRead: %s (0x%x) - (DVDAddr: 0x%x, Size: 0x%x)", pFilename, m_pFileSystem->GetFileSize(pFilename), DVDAddress, Size);
            }
            else
            {
                LOG(WII_IPC_HLE, "    DVDLowRead: file unkw - (DVDAddr: 0x%x, Size: 0x%x)", GetDeviceName().c_str(), DVDAddress, Size);
            }

            if (Size > _BufferOutSize)
            {
                PanicAlert("You try to read more data from the DVD than fit inside the out buffer. Clamp.");
                Size = _BufferOutSize;
            }

            if (VolumeHandler::ReadToPtr(Memory::GetPointer(_BufferOut), DVDAddress, Size) != true)
            {
                PanicAlert("Cant read from DVD_Plugin - DVD-Interface: Fatal Error");
            }	

            return 0x1;			
        }
        break;

    // DVDLowGetCoverReg - called by "Legend of Spyro"
    case 0x7a:
        {   
            Memory::Memset(_BufferOut, 0, _BufferOutSize);
        }
        break;

	// DVDLowClearCoverInterrupt
	case 0x86:
		{
			Memory::Memset(_BufferOut, 0, _BufferOutSize);
			LOG(WII_IPC_HLE, "%s executes DVDLowClearCoverInterrupt (Buffer 0x%08x, 0x%x)", GetDeviceName().c_str(), _BufferOut, _BufferOutSize);
		}
		break;

    // DVDLowSeek
    case 0xab:
        {}
        break;

	// DVDLowStopMotor
	case 0xe3:
		{
			Memory::Memset(_BufferOut, 0, _BufferOutSize);
			LOG(WII_IPC_HLE, "%s executes DVDLowStopMotor (Buffer 0x%08x, 0x%x)", GetDeviceName().c_str(), _BufferOut, _BufferOutSize);
            PanicAlert("Weird. Nobody should call DVDLowStopMotor.");
		}
		break;

	default:
		LOG(WII_IPC_HLE, "%s executes unknown cmd 0x%08x (Buffer 0x%08x, 0x%x)", GetDeviceName().c_str(), Command, _BufferOut, _BufferOutSize);

        PanicAlert("%s executes unknown cmd 0x%08x", GetDeviceName().c_str(), Command);
		break;
	}

    // i dunno but prolly 1 is okay all the time :)
    return 1;
}
