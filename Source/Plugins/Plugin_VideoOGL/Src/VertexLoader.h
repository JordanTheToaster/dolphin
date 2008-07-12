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

#ifndef _VERTEXLOADER_H
#define _VERTEXLOADER_H

#include <vector>

#define SHADER_POSMTX_ATTRIB 1
#define SHADER_NORM1_ATTRIB 6
#define SHADER_NORM2_ATTRIB 7

using namespace std;

////////////////
// CP Structs //
////////////////

// Vertex array numbers
enum
{
    ARRAY_POSITION	= 0,
    ARRAY_NORMAL	= 1,
    ARRAY_COLOR     = 2,
    ARRAY_COLOR2    = 3,
    ARRAY_TEXCOORD0 = 4
};

// Vertex components
enum
{
    NOT_PRESENT = 0,
    DIRECT = 1,
    INDEX8 = 2,
    INDEX16 = 3,
};

#pragma pack(4)

union TVtxDesc
{
    u64 Hex;
    struct 
    {
        // 0: not present
        // 1: present
        unsigned PosMatIdx		: 1;
        unsigned Tex0MatIdx 	: 1;
        unsigned Tex1MatIdx 	: 1;
        unsigned Tex2MatIdx 	: 1;
        unsigned Tex3MatIdx 	: 1;
        unsigned Tex4MatIdx 	: 1;
        unsigned Tex5MatIdx 	: 1;
        unsigned Tex6MatIdx 	: 1;
        unsigned Tex7MatIdx 	: 1;

        // 00: not present 
        // 01: direct 
        // 10: 8 bit index 
        // 11: 16 bit index
        unsigned Position		: 2;
        unsigned Normal			: 2;
        unsigned Color0 		: 2;
        unsigned Color1 		: 2;
        unsigned Tex0Coord 		: 2;
        unsigned Tex1Coord 		: 2;
        unsigned Tex2Coord 		: 2;
        unsigned Tex3Coord 		: 2;
        unsigned Tex4Coord 		: 2;
        unsigned Tex5Coord 		: 2;
        unsigned Tex6Coord 		: 2;
        unsigned Tex7Coord 		: 2;
        unsigned				:31;
    };
    struct {
        u32 Hex0, Hex1;
    };
};



enum
{
    FORMAT_UBYTE		= 0,	// 2 Cmp
    FORMAT_BYTE			= 1,	// 3 Cmp
    FORMAT_USHORT		= 2,
    FORMAT_SHORT		= 3,
    FORMAT_FLOAT		= 4,
};

enum
{
    FORMAT_16B_565		= 0,	// NA
    FORMAT_24B_888		= 1,	
    FORMAT_32B_888x		= 2,
    FORMAT_16B_4444		= 3,
    FORMAT_24B_6666		= 4,
    FORMAT_32B_8888		= 5,
};

union UVAT_group0
{
    u32 Hex;
    struct 
    {
        // 0:8
        unsigned PosElements	: 1;
        unsigned PosFormat		: 3; 
        unsigned PosFrac		: 5; 
        // 9:12
        unsigned NormalElements	: 1; 
        unsigned NormalFormat	: 3; 
        // 13:16
        unsigned Color0Elements : 1;
        unsigned Color0Comp		: 3; 
        // 17:20
        unsigned Color1Elements : 1;
        unsigned Color1Comp		: 3; 
        // 21:29
        unsigned Tex0CoordElements	: 1;
        unsigned Tex0CoordFormat	: 3;
        unsigned Tex0Frac			: 5;
        // 30:31
        unsigned ByteDequant: 1;
        unsigned NormalIndex3: 1;
    };
};

union UVAT_group1
{
    u32 Hex;
    struct 
    {
        // 0:8
        unsigned Tex1CoordElements	: 1;
        unsigned Tex1CoordFormat	: 3;
        unsigned Tex1Frac			: 5;
        // 9:17
        unsigned Tex2CoordElements	: 1;
        unsigned Tex2CoordFormat	: 3;
        unsigned Tex2Frac			: 5;
        // 18:26
        unsigned Tex3CoordElements	: 1;
        unsigned Tex3CoordFormat	: 3;
        unsigned Tex3Frac			: 5;
        // 27:30
        unsigned Tex4CoordElements	: 1;
        unsigned Tex4CoordFormat	: 3;
        // 
        unsigned					: 1;
    };
};

union UVAT_group2
{
    u32 Hex;
    struct 
    {
        // 0:4
        unsigned Tex4Frac			: 5;
        // 5:13
        unsigned Tex5CoordElements	: 1;
        unsigned Tex5CoordFormat	: 3;
        unsigned Tex5Frac			: 5;
        // 14:22
        unsigned Tex6CoordElements	: 1;
        unsigned Tex6CoordFormat	: 3;
        unsigned Tex6Frac			: 5;
        // 23:31
        unsigned Tex7CoordElements	: 1;
        unsigned Tex7CoordFormat	: 3;
        unsigned Tex7Frac			: 5;
    };
};

struct ColorAttr
{
    u8 Elements;
    u8 Comp;
};

struct TexAttr
{
    u8 Elements;
    u8 Format;
    u8 Frac;
};

struct TVtxAttr
{		
    u8 PosElements;
    u8 PosFormat; 
    u8 PosFrac; 
    u8 NormalElements;
    u8 NormalFormat; 
    ColorAttr color[2];
    TexAttr texCoord[8];
    u8 ByteDequant;
    u8 NormalIndex3;
};

#pragma pack ()

#define LOADERDECL __cdecl
typedef void (LOADERDECL *TPipelineFunction)(void*);

/// Use to manage loading and setting vertex buffer data for OpenGL
class VertexLoader
{
public:
    enum
    {
        NRM_ZERO = 0,
        NRM_ONE = 1,
        NRM_THREE = 3
    };

    // m_components
    enum {
        VB_HAS_POSMTXIDX =(1<<1),
        VB_HAS_TEXMTXIDX0=(1<<2),
        VB_HAS_TEXMTXIDX1=(1<<3),
        VB_HAS_TEXMTXIDX2=(1<<4),
        VB_HAS_TEXMTXIDX3=(1<<5),
        VB_HAS_TEXMTXIDX4=(1<<6),
        VB_HAS_TEXMTXIDX5=(1<<7),
        VB_HAS_TEXMTXIDX6=(1<<8),
        VB_HAS_TEXMTXIDX7=(1<<9),
        VB_HAS_TEXMTXIDXALL=(0xff<<2),
        //VB_HAS_POS=0, // Implied, it always has pos! don't bother testing
        VB_HAS_NRM0=(1<<10),
        VB_HAS_NRM1=(1<<11),
        VB_HAS_NRM2=(1<<12),
        VB_HAS_NRMALL=(7<<10),
        
        VB_HAS_COL0=(1<<13),
        VB_HAS_COL1=(1<<14),

        VB_HAS_UV0=(1<<15),
        VB_HAS_UV1=(1<<16),
        VB_HAS_UV2=(1<<17),
        VB_HAS_UV3=(1<<18),
        VB_HAS_UV4=(1<<19),
        VB_HAS_UV5=(1<<20),
        VB_HAS_UV6=(1<<21),
        VB_HAS_UV7=(1<<22),
        VB_HAS_UVALL=(0xff<<15),
        VB_HAS_UVTEXMTXSHIFT=13,
    };

private:	
    TPipelineFunction m_PipelineStates[32];
    int m_numPipelineStates;	
    int m_VertexSize; // number of bytes of a raw vertex
    int m_counter;
    int m_VBVertexStride, m_VBStridePad; // stride of a vertex to send to the GPU

    u32 m_components; // VB_HAS_X

    UVAT_group0 m_group0;
    UVAT_group1 m_group1;
    UVAT_group2 m_group2;

    vector<int> m_vtexmap; // tex index map
    TVtxAttr m_VtxAttr; //Decoded into easy format

    u8* m_compiledCode;

    //common for all loaders
    TVtxDesc m_VtxDesc;

    // seup the pipeline with this vertex fmt
    void SetupColor(int num, int _iMode, int _iFormat, int _iElements);
    void SetupTexCoord(int num, int _iMode, int _iFormat, int _iElements, int _iFrac);

    int m_AttrDirty;

public:
    // constructor
    VertexLoader();
    ~VertexLoader();
    
    // run the pipeline 
    void ProcessFormat();
    void PrepareRun();
    void RunVertices(int primitive, int count);
    void WriteCall(void  (LOADERDECL *func)(void *));
    
    int GetGCVertexSize() const { _assert_( !m_AttrDirty ); return m_VertexSize; }
    int GetVBVertexStride() const { _assert_( !m_AttrDirty); return m_VBVertexStride; }

    int ComputeVertexSize();

     // SetVAT_group
    // ignore PosFrac, texCoord[i].Frac

    void SetVAT_group0(u32 _group0) 
    {
        if ((m_group0.Hex&~0x3e0001f0) != (_group0&~0x3e0001f0)) {
            m_AttrDirty = 2;
        }
        m_group0.Hex = _group0;

        m_VtxAttr.PosElements			= m_group0.PosElements;
        m_VtxAttr.PosFormat				= m_group0.PosFormat;
        m_VtxAttr.PosFrac				= m_group0.PosFrac;
        m_VtxAttr.NormalElements		= m_group0.NormalElements;
        m_VtxAttr.NormalFormat			= m_group0.NormalFormat;
        m_VtxAttr.color[0].Elements		= m_group0.Color0Elements;
        m_VtxAttr.color[0].Comp			= m_group0.Color0Comp;
        m_VtxAttr.color[1].Elements		= m_group0.Color1Elements;
        m_VtxAttr.color[1].Comp			= m_group0.Color1Comp;
        m_VtxAttr.texCoord[0].Elements	= m_group0.Tex0CoordElements;
        m_VtxAttr.texCoord[0].Format	= m_group0.Tex0CoordFormat;
        m_VtxAttr.texCoord[0].Frac		= m_group0.Tex0Frac;
        m_VtxAttr.ByteDequant			= m_group0.ByteDequant;
        m_VtxAttr.NormalIndex3			= m_group0.NormalIndex3;
    };

    void SetVAT_group1(u32 _group1) 
    {
        if ((m_group1.Hex&~0x7c3e1f0) != (_group1&~0x7c3e1f0)) {
            m_AttrDirty = 2;
        }
        m_group1.Hex = _group1;
        

        m_VtxAttr.texCoord[1].Elements	= m_group1.Tex1CoordElements;
        m_VtxAttr.texCoord[1].Format	= m_group1.Tex1CoordFormat;
        m_VtxAttr.texCoord[1].Frac		= m_group1.Tex1Frac;
        m_VtxAttr.texCoord[2].Elements	= m_group1.Tex2CoordElements;
        m_VtxAttr.texCoord[2].Format	= m_group1.Tex2CoordFormat;
        m_VtxAttr.texCoord[2].Frac		= m_group1.Tex2Frac;
        m_VtxAttr.texCoord[3].Elements	= m_group1.Tex3CoordElements;
        m_VtxAttr.texCoord[3].Format	= m_group1.Tex3CoordFormat;
        m_VtxAttr.texCoord[3].Frac      = m_group1.Tex3Frac;
        m_VtxAttr.texCoord[4].Elements	= m_group1.Tex4CoordElements;
        m_VtxAttr.texCoord[4].Format	= m_group1.Tex4CoordFormat;
    };									  
                                          
    void SetVAT_group2(u32 _group2)		  
    {
        if ((m_group2.Hex&~0xf87c3e1f) != (_group2&~0xf87c3e1f)) {
            m_AttrDirty = 2;
        }
        m_group2.Hex = _group2;

        m_VtxAttr.texCoord[4].Frac		= m_group2.Tex4Frac;
        m_VtxAttr.texCoord[5].Elements	= m_group2.Tex5CoordElements;
        m_VtxAttr.texCoord[5].Format	= m_group2.Tex5CoordFormat;
        m_VtxAttr.texCoord[5].Frac		= m_group2.Tex5Frac;
        m_VtxAttr.texCoord[6].Elements	= m_group2.Tex6CoordElements;
        m_VtxAttr.texCoord[6].Format	= m_group2.Tex6CoordFormat;
        m_VtxAttr.texCoord[6].Frac		= m_group2.Tex6Frac;
        m_VtxAttr.texCoord[7].Elements	= m_group2.Tex7CoordElements;
        m_VtxAttr.texCoord[7].Format	= m_group2.Tex7CoordFormat;
        m_VtxAttr.texCoord[7].Frac		= m_group2.Tex7Frac;
    };
};									  

/// Methods to manage and cache the global state of vertex streams and flushing streams
/// Also handles processing the CP registers
class VertexManager
{
    static TVtxDesc s_GlobalVtxDesc;

public:
    enum Collection
    {
        C_NOTHING=0,
        C_TRIANGLES=1,
        C_LINES=2,
        C_POINTS=3
    };

    static bool Init();
    static void Destroy();

    static void ResetBuffer();
    static void ResetComponents();

    static void AddVertices(int primitive, int numvertices);
    static void Flush(); // flushes the current buffer

    static int GetRemainingSize();
    static TVtxDesc &GetVtxDesc() {return s_GlobalVtxDesc; }

    static void LoadCPReg(u32 SubCmd, u32 Value);
    static size_t SaveLoadState(char *ptr, BOOL save);

    static u8* s_pCurBufferPointer;
    static float shiftLookup[32];
};

extern VertexLoader g_VertexLoaders[8];
extern u32 arraybases[16];
extern u32 arraystrides[16];
u8 ReadBuffer8();
u16 ReadBuffer16();
u32 ReadBuffer32();
float ReadBuffer32F();

#endif
