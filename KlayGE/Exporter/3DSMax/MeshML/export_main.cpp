// export_main.cpp
// KlayGE 3DSMax导出接口类 实现文件
// Ver 2.5.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
//
// 2.5.0
// 初次建立 (2005.5.1)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <max.h>

#include "meshml.hpp"
#include "export_main.hpp"

KlayGE::meshml_class_desc mcd;

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
		return &mcd;

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
