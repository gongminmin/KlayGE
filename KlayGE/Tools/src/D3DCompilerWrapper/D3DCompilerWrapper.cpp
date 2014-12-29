/**
* @file D3DCompierWrapper.cpp
* @author Kanglai Qian, Minmin Gong
*
* @section DESCRIPTION
*
* This source file is part of KlayGE
* For the latest info, see http://www.klayge.org
*
* @section LICENSE
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published
* by the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*
* You may alternatively use this source under the terms of
* the KlayGE Proprietary License (KPL). You can obtained such a license
* from http://www.klayge.org/licensing/.
*/

// Keep this file STL free because of winegcc compatibility.

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <windows.h>
#include <D3Dcompiler.h>

// http://wine-wiki.org/index.php/WineLib#Calling_a_Native_Windows_dll_from_Linux
int main(int argc, char* argv[])
{
	HMODULE mod_d3dcompiler = LoadLibraryEx(TEXT("d3dcompiler_47.dll"), NULL, 0);
	if (!mod_d3dcompiler)
	{
		printf("LoadLibraryEx fail\n");
		return -1;
	}
	pD3DCompile DynamicD3DCompile = reinterpret_cast<pD3DCompile>(GetProcAddress(mod_d3dcompiler, "D3DCompile"));
	if (!DynamicD3DCompile)
	{
		printf("GetProcAddress fail\n");
		return -1;
	}
	if (argc < 9)
	{
#ifdef WIN32
		printf("D3DCompilerWrapper.exe hlsl_file defines_file entry_point target flags1 flags2 dxbc_file error_file");
#else
		printf("wine D3DCompilerWrapper.so hlsl_file defines_file entry_point target flags1 flags2 dxbc_file error_file");
#endif
		return -1;
	}

	char const * hlsl_file = argv[1];
	char const * defines_file = argv[2];
	char const * entry_point = argv[3];
	char const * target = argv[4];
	int flags1 = atoi(argv[5]);
	int flags2 = atoi(argv[6]);
	char const * dxbc_file = argv[7];
	char const * error_file = argv[8];

	FILE* fp = fopen(hlsl_file, "r");
	fseek(fp, 0, SEEK_END);
	long buffer_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char* buffer = new char[buffer_size + 1];
	fread(buffer, sizeof(char), buffer_size, fp);
	fclose(fp);
	buffer[buffer_size] = 0;

	D3D_SHADER_MACRO* macros = new D3D_SHADER_MACRO[128];
	char line_name[1024];
	char line_definition[1024];
	int idx = 0;
	fp = fopen(defines_file, "r");
	while (fgets(line_name, 1024, fp) && fgets(line_definition, 1024, fp))
	{
		char* t1 = new char[strlen(line_name) + 1];
		strcpy(t1, line_name);
		if ('\n' == t1[strlen(t1) - 1])
		{
			t1[strlen(t1) - 1] = '\0';
		}
		char* t2 = new char[strlen(line_definition) + 1];
		strcpy(t2, line_definition);
		if ('\n' == t2[strlen(t2) - 1])
		{
			t2[strlen(t2) - 1] = '\0';
		}
		macros[idx].Name = t1;
		macros[idx].Definition = t2;
		++ idx;
	}
	macros[idx].Name = NULL;
	macros[idx].Definition = NULL;
	fclose(fp);

	ID3DBlob* code = NULL;
	ID3DBlob* err_msg = NULL;
	HRESULT hr = DynamicD3DCompile(buffer, buffer_size, NULL, macros, NULL, entry_point,
		target, flags1, flags2, &code, &err_msg);
	fp = fopen(error_file, "w");
	if (FAILED(hr))
	{
		printf("Compiling error: 0x%x", hr);
	}
	if (err_msg != NULL)
	{
		fwrite(err_msg->GetBufferPointer(), sizeof(char), err_msg->GetBufferSize(), fp);
	}
	fclose(fp);
	fp = fopen(dxbc_file, "wb");
	if (code != NULL)
	{
		fwrite(code->GetBufferPointer(), sizeof(char), code->GetBufferSize(), fp);
	}
	fclose(fp);

	for (int i = 0; i < idx; ++ i)
	{
		delete[] macros[i].Name;
		delete[] macros[i].Definition;
	}
	delete[] macros;
	delete[] buffer;

	FreeLibrary(mod_d3dcompiler);

	return 0;
}
