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

#ifndef __ISOFILE_H_
#define __ISOFILE_H_

#include "Volume.h"

class CISOFile
{
	public:

		CISOFile(const std::string& _rFileName);
		~CISOFile();

		bool IsValid() const {return(m_Valid);}


		const std::string& GetFileName() const {return(m_FileName);}


		const std::string& GetName() const {return(m_Name);}


		const std::string& GetCompany() const {return(m_Company);}


		DiscIO::IVolume::ECountry GetCountry() const {return(m_Country);}


		u64 GetFileSize() const {return(m_FileSize);}


		const wxImage& GetImage() const {return(m_Image);}


	private:

		std::string m_FileName;
		std::string m_Name;
		std::string m_Company;

		u64 m_FileSize;

		DiscIO::IVolume::ECountry m_Country;

		wxImage m_Image;

		bool m_Valid;
};


#endif
