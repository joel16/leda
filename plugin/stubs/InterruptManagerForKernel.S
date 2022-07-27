    .set noreorder

#include "pspimport.s"
    IMPORT_START "InterruptManagerForKernel",0x00010000
    IMPORT_FUNC  "InterruptManagerForKernel",0xF153B371,sceKernelQuerySystemCall
