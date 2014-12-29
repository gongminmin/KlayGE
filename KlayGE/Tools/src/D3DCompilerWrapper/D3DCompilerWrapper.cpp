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
	if (argc < 7)
	{
#ifdef WIN32
		printf("D3DCompilerWrapper.exe input_file entry_point target flags1 flags2 output_file");
#else
		printf("wine D3DCompilerWrapper.so input_file defines_file entry_point target flags1 flags2 output_file");
#endif
		return -1;
	}

	char const * input_file = argv[1];
	char const * entry_point = argv[2];
	char const * target = argv[3];
	int flags1 = atoi(argv[4]);
	int flags2 = atoi(argv[5]);
	char const * output_file = argv[6];

	FILE* fp = fopen(input_file, "rb");
	int hlsl_size;
	fread(&hlsl_size, sizeof(hlsl_size), 1, fp);
	char* hlsl = new char[hlsl_size + 1];
	fread(hlsl, sizeof(char), hlsl_size, fp);
	hlsl[hlsl_size] = 0;

	int num_macros;
	fread(&num_macros, sizeof(num_macros), 1, fp);
	D3D_SHADER_MACRO* macros = new D3D_SHADER_MACRO[num_macros + 1];
	char line_name[1024];
	char line_definition[1024];
	int idx = 0;
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
	HRESULT hr = DynamicD3DCompile(hlsl, hlsl_size, NULL, macros, NULL, entry_point,
		target, flags1, flags2, &code, &err_msg);
	if (FAILED(hr))
	{
		printf("Compiling error: 0x%x", hr);
	}
	fp = fopen(output_file, "wb");
	fwrite(&hr, sizeof(hr), 1, fp);
	if (code != NULL)
	{
		int const size = static_cast<int>(code->GetBufferSize());
		fwrite(&size, sizeof(size), 1, fp);
		fwrite(code->GetBufferPointer(), sizeof(char), size, fp);
	}
	else
	{
		int const size = 0;
		fwrite(&size, sizeof(size), 1, fp);
	}
	if (err_msg != NULL)
	{
		int const size = static_cast<int>(err_msg->GetBufferSize());
		fwrite(&size, sizeof(size), 1, fp);
		fwrite(err_msg->GetBufferPointer(), sizeof(char), err_msg->GetBufferSize(), fp);
	}
	else
	{
		int const size = 0;
		fwrite(&size, sizeof(size), 1, fp);
	}
	fclose(fp);

	for (int i = 0; i < num_macros; ++ i)
	{
		delete[] macros[i].Name;
		delete[] macros[i].Definition;
	}
	delete[] macros;
	delete[] hlsl;

	FreeLibrary(mod_d3dcompiler);

	return 0;
}
