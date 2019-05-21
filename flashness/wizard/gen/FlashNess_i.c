

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


 /* File created by MIDL compiler version 7.00.0555 */
/* at Tue May 21 22:30:41 2019
 */
/* Compiler settings for FlashNess.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 7.00.0555 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, IID_IComponentRegistrar,0xa817e7a2,0x43fa,0x11d0,0x9e,0x44,0x00,0xaa,0x00,0xb6,0x77,0x0a);


MIDL_DEFINE_GUID(IID, IID_IFlashNess,0x13953020,0xA524,0x47F3,0x9C,0x07,0xD3,0x84,0xE0,0x38,0x0E,0xE3);


MIDL_DEFINE_GUID(IID, LIBID_FlashNessLib,0x77696C48,0xE4FB,0x48DB,0x86,0x6A,0x54,0x60,0x54,0x49,0x1C,0x2A);


MIDL_DEFINE_GUID(CLSID, CLSID_CompReg,0x3C1338E5,0x80F7,0x4D95,0xB0,0x0F,0x54,0xBC,0x8A,0x3F,0xC6,0x4C);


MIDL_DEFINE_GUID(IID, DIID__IFlashNessEvents,0x5D6EAB78,0x98C7,0x4591,0x93,0xCF,0x6B,0x16,0xBD,0xB7,0xF1,0x65);


MIDL_DEFINE_GUID(CLSID, CLSID_FlashNess,0x6DD1301A,0xDCB4,0x43E4,0x94,0x47,0xB1,0xC7,0xEA,0x0A,0xBA,0xCB);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



