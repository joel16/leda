#include <pspdisplay_kernel.h>
#include <pspinit.h>
#include <pspkdebug.h>
#include <pspkernel.h>
#include <psploadexec_kernel.h>
#include <pspmodulemgr.h>
#include <pspsyscon.h>

#include "kubridge.h"

PSP_MODULE_INFO("Legacy_Software_Loader", 0x1006, 1, 0);

// Structs and Macros (Mostly from uOFW)

/** Current number category size for libraries. */
#define LIBRARY_VERSION_NUMBER_CATEGORY_SIZE    (2)

/** 
 * This structure represents a function stub belonging to same privilege-level linked libraries, 
 * i.e. a kernel resident library linked with a kernel stub library. 
 */
typedef struct {
    /** The call to the imported function via a MIPS ASM Jump instruction. */
    u32 call;
    /** The delay slot belonging to the call, typically a NOP instruction. */
    u32 delaySlot;
} DirectCall;

/** 
 * This structure represents a function stub belonging to different privilege-level linked libraries, 
 * i.e. a kernel resident library linked with a user stub library. 
 */
typedef struct {
    /** The return instruction from the stub. Typically a JR $ra command. */
    u32 returnAddr;
    /** The system call exception used to call the imported function. */
    u32 syscall;
} Syscall;

/**
 * This structure represents an imported function stub.
 */
typedef union {
    /** User/User or Kernel/Kernel function stub. */
    DirectCall dc;
    /** Kernel/User function stub. */
    Syscall sc;
} SceStub;

/**
 * This structure represents an imported variable stub.
 */
typedef struct {
    u32 *addr;
    /** The NID identifying the imported variable. */
    u32 nid;
} SceVariableStub;

/**
 * This structure represents the imports, provided by a resident library, that a given module is using. 
 * A module can have multiple stub libraries.
 */
typedef struct {
    /** The name of the library. */
	const char *libName;  //0
    /** 
     * The version of the library. It consists of a 'major' and 'minor' field. The version of a stub 
     * library shouldn't be higher than the version(s) of the corresponding resident library/libraries. 
     * Linking won't be performed in such a case.
     */
	u8 version[LIBRARY_VERSION_NUMBER_CATEGORY_SIZE]; //4
    /** The library's attributes. Can be set to either SCE_LIB_NO_SPECIAL_ATTR or SCE_LIB_WEAK_IMPORT. */
	u16 attribute; //6
    /**
     * The length of this entry table in 32-Bit words. Set this to either "STUB_LIBRARY_ENTRY_TABLE_OLD_LEN" 
     * or "STUB_LIBRARY_ENTRY_TABLE_NEW_LEN". Use this member when  you want to iterate through a 
     * list of entry tables (size = len * 4).
     */ 
	u8 len; //8
    /** The number of imported variables by the stub library. */
	u8 vStubCount; //9
    /** The number of imported functions by the stub library. */
	u16 stubCount; //10
    /** Pointer to an array of NIDs containing the NIDs of the imported functions and variables. */
	u32 *nidTable; //12
    /** Pointer to an array of imported function stubs. */
	SceStub *stubTable; //16
    /** Pointer to an array of imported variable stubs. */
	SceVariableStub *vStubTable; // 20
    /** Unknown. */
    u16 unk24; //24
} SceStubLibraryEntryTable;

// Globals
SceOff g_pos = 0;       // 0x00003DD0
SceOff g_pos2 = 0;      // 0x00003DCC
s32 g_module_id = 0;    // 0x00003DE4
void *g_address = NULL; // 0x00003E10
u32 g_address_size = 0; // 0x00003DE8

// Function prototypes
s32 sctrlHENRegisterHomebrewLoader(s32 (* handler)(const char *path, s32 flags, SceKernelLMOption *option));
s32 sceKernelLinkLibraryEntriesWithModule(SceModule *mod, SceStubLibraryEntryTable *libStubTable, u32 size);
u32 sctrlHENFindFunction(char *modname, char *libname, u32 nid);
extern u32 sceKernelQuerySystemCall(void *func);

s32 sub_000000F0(s32 *arg) {
    s32 ret = 1;
    
    if (*arg == 0x3E00008) {
        ret = 0;
        
        if (arg[1] != 0) {
            return arg[1] != 0x3CC;
        }
    }
    
    return ret;
}

void loc_00000164(void) {
    kuKernelLoadModule(NULL, 0, NULL);
    kuKernelLoadModuleWithApitype2(0, NULL, 0, NULL);
    kuKernelInitApitype();
    kuKernelInitFileName(NULL);
    kuKernelBootFrom();
    kuKernelInitKeyConfig();
    kuKernelGetUserLevel();
    kuKernelSetDdrMemoryProtection(NULL, 0, 0);
    kuKernelGetModel();
    return;
}

SceOff sub_000001FC(SceUID fd, SceOff offset, s32 whence) {
    g_pos = sceIoLseek(fd, offset, whence);
    
    if (g_pos2 != 0) {
        if (offset != 0 && whence == PSP_SEEK_END) {
            return g_pos2;
        }
    }
    
    return g_pos;
}

s32 sub_00000968(SceCtrlData *pad_data, s32 count) {
    s32 k1 = pspSdkSetK1(0);
    s32 ret = sceCtrlPeekBufferPositive(pad_data, count);
    pspSdkSetK1(k1);
    return ret;
}

void sub_000009C4(void) {
    s32 k1 = pspSdkSetK1(0);
    sceKernelIcacheInvalidateAll();
    pspSdkSetK1(k1);
}

void sub_000009FC(s32 level, s32 unk) {
    s32 k1 = pspSdkSetK1(0);
    sceDisplaySetBrightness(level, unk);
    pspSdkSetK1(k1);
}

s32 sub_00000A54(s32 SceLED, s32 state) {
    s32 k1 = pspSdkSetK1 (0);
    s32 ret = sceSysconCtrlLED(SceLED, state);
    pspSdkSetK1(k1);
    return ret;
}

s32 sub_00000AB8(char *modname, char *libname, u32 nid) {
    s32 ret = 0;
    
    u32 *func = (void *)sctrlHENFindFunction(modname, libname, nid);
    
    if (func != 0) {
        ret = sceKernelQuerySystemCall(func);

        if (ret < 0) {
            ret = 0;
        }
    }
    
    return ret;
}

/**
 * Subroutine at address 0x00000CCC
 */
void sub_00000CCC(void) {
    sceKernelDcacheWritebackAll();
    sceKernelIcacheClearAll();
}

s32 sub_00002ACC(s32 arg, void *address, u32 size) {
    // LoadCoreForKernel_C0913394 was used without any args in the original leda plugin
    s32 ret = sceKernelLinkLibraryEntriesWithModule(NULL, NULL, 0);
    
    if (g_address == NULL && ret < 0) {
        g_module_id = sceKernelGetModuleIdByAddress(address);
        g_address = address;
        g_address_size = size;
    }

    // sub_000022D4();
    return ret;
}

// TODO
s32 sub_00003304(const char *path, s32 flags, SceKernelLMOption *option) {
    return 0;
}

s32 loc_00000AB0(struct SceKernelLoadExecVSHParam *param) {
    return sceKernelExitVSHKernel(NULL);
}


s32 module_start(SceSize args, void *argp) {
    s32 api_type = sceKernelInitApitype();
    
    if (api_type != PSP_INIT_APITYPE_MS2) {
        return 1;
    }
    
    Kprintf("LEDA - Legacy Software Loader, version 0.1, by Dark_AleX\n");
    sctrlHENRegisterHomebrewLoader(sub_00003304);
    return 0;
}

s32 module_stop(void) {
	return 0;
}
