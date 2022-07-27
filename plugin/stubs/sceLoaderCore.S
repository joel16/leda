	.set noreorder

#include "pspimport.s"

	IMPORT_START "LoadCoreForKernel",0x00010011
	IMPORT_FUNC  "LoadCoreForKernel",0xA481E30E,sceKernelLinkLibraryEntriesWithModule
	IMPORT_FUNC  "LoadCoreForKernel",0xD8779AC6,sceKernelIcacheClearAll
