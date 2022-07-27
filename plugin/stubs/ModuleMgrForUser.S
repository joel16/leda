	.set noreorder

#include "pspimport.s"

	IMPORT_START "ModuleMgrForUser",0x40010011
	IMPORT_FUNC  "ModuleMgrForUser",0xD8B73127, sceKernelGetModuleIdByAddress
