#include <max.h>

#include "meshml.hpp"
#include "export_main.hpp"

MeshML_ClassDesc meshml_class_desc;

HINSTANCE dll_instance;

BOOL APIENTRY DllMain(HINSTANCE module, DWORD ul_reason_for_call, LPVOID reserved)
{
	dll_instance = module;
	return TRUE;
}

extern "C" int LibNumberClasses()
{
	return 1;
}

extern "C" ClassDesc* LibClassDesc(int i)
{
	switch (i)
	{
	case 0:
		return &meshml_class_desc;

	default:
		return 0;
	}
}

extern "C" const TCHAR* LibDescription()
{
	return TEXT("MeshML Max File Export Plugin");
}

extern "C" ULONG LibVersion()
{
	return VERSION_3DSMAX;
}
