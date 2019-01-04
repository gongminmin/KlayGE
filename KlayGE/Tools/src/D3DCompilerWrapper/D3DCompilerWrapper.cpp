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

// Keep this file STL free because of wineg++ compatibility.

#define INITGUID

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#elif defined(__GNUC__)
#define KLAYGE_COMPILER_GCC
#elif defined(__clang__)
#define KLAYGE_COMPILER_CLANG
#endif

#include <stdio.h>
#include <windows.h>
#include <KlayGE/SALWrapper.hpp>
#include <d3d11shader.h>

#ifndef D3D11_SHVER_GET_TYPE
enum D3D11_SHADER_VERSION_TYPE
{
	D3D11_SHVER_PIXEL_SHADER = 0,
	D3D11_SHVER_VERTEX_SHADER = 1,
	D3D11_SHVER_GEOMETRY_SHADER = 2,

	// D3D11 Shaders
	D3D11_SHVER_HULL_SHADER = 3,
	D3D11_SHVER_DOMAIN_SHADER = 4,
	D3D11_SHVER_COMPUTE_SHADER = 5,
};

#define D3D11_SHVER_GET_TYPE(_Version) (((_Version) >> 16) & 0xFFFF)
#endif

DEFINE_GUID(IID_ID3D11ShaderReflection_47,
	0x8d536ca1, 0x0cca, 0x4956, 0xa8, 0x37, 0x78, 0x69, 0x63, 0x75, 0x55, 0x84);

struct D3D11_SIGNATURE_PARAMETER_DESC_47
{
	LPCSTR						SemanticName;
	UINT						SemanticIndex;
	UINT						Register;
	D3D_NAME					SystemValueType;
	D3D_REGISTER_COMPONENT_TYPE	ComponentType;
	BYTE						Mask;
	BYTE						ReadWriteMask;
	UINT						Stream;
	UINT						MinPrecision;
};

class D3DCompiler
{
public:
	D3DCompiler()
		: mod_d3dcompiler_(NULL),
			DynamicD3DCompile_(NULL),
			DynamicD3DReflect_(NULL),
			DynamicD3DStripShader_(NULL)
	{
		mod_d3dcompiler_ = LoadLibraryEx(TEXT("d3dcompiler_47.dll"), NULL, 0);
		if (mod_d3dcompiler_)
		{
#if defined(KLAYGE_COMPILER_GCC) && (__GNUC__ >= 8)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
			DynamicD3DCompile_ = reinterpret_cast<D3DCompileFunc>(GetProcAddress(mod_d3dcompiler_, "D3DCompile"));
			DynamicD3DReflect_ = reinterpret_cast<D3DReflectFunc>(GetProcAddress(mod_d3dcompiler_, "D3DReflect"));
			DynamicD3DStripShader_ = reinterpret_cast<D3DStripShaderFunc>(GetProcAddress(mod_d3dcompiler_, "D3DStripShader"));
#if defined(KLAYGE_COMPILER_GCC) && (__GNUC__ >= 8)
#pragma GCC diagnostic pop
#endif
		}
		else
		{
			MessageBoxW(NULL, L"Can't load d3dcompiler_47.dll", L"Error", MB_OK);
		}
	}

	~D3DCompiler()
	{
		if (mod_d3dcompiler_)
		{
			FreeLibrary(mod_d3dcompiler_);
		}
	}

	HRESULT D3DCompile(LPCVOID pSrcData, SIZE_T SrcDataSize, LPCSTR pSourceName,
		D3D_SHADER_MACRO const * pDefines, ID3DInclude* pInclude, LPCSTR pEntrypoint,
		LPCSTR pTarget, UINT Flags1, UINT Flags2, ID3DBlob** ppCode, ID3DBlob** ppErrorMsgs) const
	{
		return DynamicD3DCompile_(pSrcData, SrcDataSize, pSourceName, pDefines, pInclude, pEntrypoint,
			pTarget, Flags1, Flags2, ppCode, ppErrorMsgs);
	}

	HRESULT D3DReflect(LPCVOID pSrcData, SIZE_T SrcDataSize, REFIID pInterface, void** ppReflector) const
	{
		return DynamicD3DReflect_(pSrcData, SrcDataSize, pInterface, ppReflector);
	}

	HRESULT D3DStripShader(LPCVOID pShaderBytecode, SIZE_T BytecodeLength, UINT uStripFlags, ID3DBlob** ppStrippedBlob) const
	{
		return DynamicD3DStripShader_(pShaderBytecode, BytecodeLength, uStripFlags, ppStrippedBlob);
	}

private:
	typedef HRESULT(WINAPI *D3DCompileFunc)(LPCVOID pSrcData, SIZE_T SrcDataSize, LPCSTR pSourceName,
		D3D_SHADER_MACRO const * pDefines, ID3DInclude* pInclude, LPCSTR pEntrypoint,
		LPCSTR pTarget, UINT Flags1, UINT Flags2, ID3DBlob** ppCode, ID3DBlob** ppErrorMsgs);
	typedef HRESULT(WINAPI *D3DReflectFunc)(LPCVOID pSrcData, SIZE_T SrcDataSize, REFIID pInterface, void** ppReflector);
	typedef HRESULT(WINAPI *D3DStripShaderFunc)(LPCVOID pShaderBytecode, SIZE_T BytecodeLength, UINT uStripFlags, ID3DBlob** ppStrippedBlob);

private:
	HMODULE mod_d3dcompiler_;

	D3DCompileFunc DynamicD3DCompile_;
	D3DReflectFunc DynamicD3DReflect_;
	D3DStripShaderFunc DynamicD3DStripShader_;
};

void PrintHelps()
{
	char const * cmd;
#ifdef WIN32
	cmd = "D3DCompilerWrapper.exe";
#else
	cmd = "wine D3DCompilerWrapper.exe.so";
#endif
	printf("Usage:\n");
	printf("\t%s compile input_file entry_point target flags1 flags2 output_file\n", cmd);
	printf("\t%s reflect input_file output_file\n", cmd);
	printf("\t%s strip input_file flags output_file\n", cmd);
}

void WriteString(char const * str, FILE* fp)
{
	UINT len = static_cast<UINT>(strlen(str));
	fwrite(&len, sizeof(len), 1, fp);
	fwrite(str, 1, len, fp);
}

// http://wine-wiki.org/index.php/WineLib#Calling_a_Native_Windows_dll_from_Linux
int main(int argc, char* argv[])
{
	D3DCompiler d3d_compiler;

	if (argc < 2)
	{
		PrintHelps();
		return -1;
	}

	if (0 == strcmp(argv[1], "compile"))
	{
		if (argc < 8)
		{
			PrintHelps();
			return -1;
		}

		char const * input_file = argv[2];
		char const * entry_point = argv[3];
		char const * target = argv[4];
		int flags1 = atoi(argv[5]);
		int flags2 = atoi(argv[6]);
		char const * output_file = argv[7];

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
		int hr = d3d_compiler.D3DCompile(hlsl, hlsl_size, NULL, macros, NULL, entry_point,
			target, flags1, flags2, &code, &err_msg);
		if (FAILED(hr))
		{
			printf("Compiling error: 0x%x\n", hr);
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
	}
	else if (0 == strcmp(argv[1], "reflect"))
	{
		if (argc < 4)
		{
			PrintHelps();
			return -1;
		}

		char const * input_file = argv[2];
		char const * output_file = argv[3];

		FILE* fp = fopen(input_file, "rb");
		fseek(fp, 0, SEEK_END);
		long bytecode_size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		char* bytecode = new char[bytecode_size];
		fread(bytecode, sizeof(char), bytecode_size, fp);
		fclose(fp);

		ID3D11ShaderReflection* reflection;
		int hr = d3d_compiler.D3DReflect(bytecode, bytecode_size, IID_ID3D11ShaderReflection_47,
			reinterpret_cast<void**>(&reflection));
		if (FAILED(hr))
		{
			printf("Reflect error: 0x%x\n", hr);
		}

		if (reflection != NULL)
		{
			fp = fopen(output_file, "wb");

			D3D11_SHADER_DESC desc;
			reflection->GetDesc(&desc);
			fwrite(&desc.Version, sizeof(desc.Version), 1, fp);
			WriteString(desc.Creator, fp);
			fwrite(&desc.Flags, sizeof(desc.Flags), 1, fp);
			fwrite(&desc.ConstantBuffers, sizeof(desc.ConstantBuffers), 1, fp);
			fwrite(&desc.BoundResources, sizeof(desc.BoundResources), 1, fp);
			fwrite(&desc.InputParameters, sizeof(desc.InputParameters), 1, fp);
			fwrite(&desc.OutputParameters, sizeof(desc.OutputParameters), 1, fp);
			fwrite(&desc.InstructionCount, sizeof(desc.InstructionCount), 1, fp);
			fwrite(&desc.TempRegisterCount, sizeof(desc.TempRegisterCount), 1, fp);
			fwrite(&desc.TempArrayCount, sizeof(desc.TempArrayCount), 1, fp);
			fwrite(&desc.DefCount, sizeof(desc.DefCount), 1, fp);
			fwrite(&desc.DclCount, sizeof(desc.DclCount), 1, fp);
			fwrite(&desc.TextureNormalInstructions, sizeof(desc.TextureNormalInstructions), 1, fp);
			fwrite(&desc.TextureLoadInstructions, sizeof(desc.TextureLoadInstructions), 1, fp);
			fwrite(&desc.TextureCompInstructions, sizeof(desc.TextureCompInstructions), 1, fp);
			fwrite(&desc.TextureBiasInstructions, sizeof(desc.TextureBiasInstructions), 1, fp);
			fwrite(&desc.TextureGradientInstructions, sizeof(desc.TextureGradientInstructions), 1, fp);
			fwrite(&desc.FloatInstructionCount, sizeof(desc.FloatInstructionCount), 1, fp);
			fwrite(&desc.IntInstructionCount, sizeof(desc.IntInstructionCount), 1, fp);
			fwrite(&desc.UintInstructionCount, sizeof(desc.UintInstructionCount), 1, fp);
			fwrite(&desc.StaticFlowControlCount, sizeof(desc.StaticFlowControlCount), 1, fp);
			fwrite(&desc.DynamicFlowControlCount, sizeof(desc.DynamicFlowControlCount), 1, fp);
			fwrite(&desc.MacroInstructionCount, sizeof(desc.MacroInstructionCount), 1, fp);
			fwrite(&desc.ArrayInstructionCount, sizeof(desc.ArrayInstructionCount), 1, fp);
			fwrite(&desc.CutInstructionCount, sizeof(desc.CutInstructionCount), 1, fp);
			fwrite(&desc.EmitInstructionCount, sizeof(desc.EmitInstructionCount), 1, fp);
			fwrite(&desc.GSOutputTopology, sizeof(desc.GSOutputTopology), 1, fp);
			fwrite(&desc.GSMaxOutputVertexCount, sizeof(desc.GSMaxOutputVertexCount), 1, fp);
			fwrite(&desc.InputPrimitive, sizeof(desc.InputPrimitive), 1, fp);
			fwrite(&desc.PatchConstantParameters, sizeof(desc.PatchConstantParameters), 1, fp);
			fwrite(&desc.cGSInstanceCount, sizeof(desc.cGSInstanceCount), 1, fp);
			fwrite(&desc.cControlPoints, sizeof(desc.cControlPoints), 1, fp);
			fwrite(&desc.HSOutputPrimitive, sizeof(desc.HSOutputPrimitive), 1, fp);
			fwrite(&desc.HSPartitioning, sizeof(desc.HSPartitioning), 1, fp);
			fwrite(&desc.TessellatorDomain, sizeof(desc.TessellatorDomain), 1, fp);
			fwrite(&desc.cBarrierInstructions, sizeof(desc.cBarrierInstructions), 1, fp);
			fwrite(&desc.cInterlockedInstructions, sizeof(desc.cInterlockedInstructions), 1, fp);
			fwrite(&desc.cTextureStoreInstructions, sizeof(desc.cTextureStoreInstructions), 1, fp);
			
			for (UINT c = 0; c < desc.ConstantBuffers; ++ c)
			{
				ID3D11ShaderReflectionConstantBuffer* reflection_cb = reflection->GetConstantBufferByIndex(c);

				D3D11_SHADER_BUFFER_DESC d3d_cb_desc;
				reflection_cb->GetDesc(&d3d_cb_desc);
				WriteString(d3d_cb_desc.Name, fp);
				fwrite(&d3d_cb_desc.Type, sizeof(d3d_cb_desc.Type), 1, fp);
				fwrite(&d3d_cb_desc.Variables, sizeof(d3d_cb_desc.Variables), 1, fp);
				fwrite(&d3d_cb_desc.Size, sizeof(d3d_cb_desc.Size), 1, fp);
				fwrite(&d3d_cb_desc.uFlags, sizeof(d3d_cb_desc.uFlags), 1, fp);

				for (UINT v = 0; v < d3d_cb_desc.Variables; ++ v)
				{
					ID3D11ShaderReflectionVariable* reflection_var = reflection_cb->GetVariableByIndex(v);

					D3D11_SHADER_VARIABLE_DESC var_desc;
					reflection_var->GetDesc(&var_desc);
					fwrite(&var_desc, sizeof(var_desc), 1, fp);

					WriteString(var_desc.Name, fp);
					fwrite(&var_desc.StartOffset, sizeof(var_desc.StartOffset), 1, fp);
					fwrite(&var_desc.Size, sizeof(var_desc.Size), 1, fp);
					fwrite(&var_desc.uFlags, sizeof(var_desc.uFlags), 1, fp);
					fwrite(var_desc.DefaultValue, var_desc.Size, 1, fp);
					fwrite(&var_desc.StartTexture, sizeof(var_desc.StartTexture), 1, fp);
					fwrite(&var_desc.TextureSize, sizeof(var_desc.TextureSize), 1, fp);
					fwrite(&var_desc.StartSampler, sizeof(var_desc.StartSampler), 1, fp);
					fwrite(&var_desc.SamplerSize, sizeof(var_desc.SamplerSize), 1, fp);

					D3D11_SHADER_TYPE_DESC type_desc;
					reflection_var->GetType()->GetDesc(&type_desc);
					fwrite(&type_desc.Class, sizeof(type_desc.Class), 1, fp);
					fwrite(&type_desc.Type, sizeof(type_desc.Type), 1, fp);
					fwrite(&type_desc.Rows, sizeof(type_desc.Rows), 1, fp);
					fwrite(&type_desc.Columns, sizeof(type_desc.Columns), 1, fp);
					fwrite(&type_desc.Elements, sizeof(type_desc.Elements), 1, fp);
					fwrite(&type_desc.Members, sizeof(type_desc.Members), 1, fp);
					fwrite(&type_desc.Offset, sizeof(type_desc.Offset), 1, fp);
					WriteString(type_desc.Name, fp);
				}
			}

			for (UINT i = 0; i < desc.BoundResources; ++ i)
			{
				D3D11_SHADER_INPUT_BIND_DESC si_desc;
				reflection->GetResourceBindingDesc(i, &si_desc);
				WriteString(si_desc.Name, fp);
				fwrite(&si_desc.Type, sizeof(si_desc.Type), 1, fp);
				fwrite(&si_desc.BindPoint, sizeof(si_desc.BindPoint), 1, fp);
				fwrite(&si_desc.BindCount, sizeof(si_desc.BindCount), 1, fp);
				fwrite(&si_desc.uFlags, sizeof(si_desc.uFlags), 1, fp);
				fwrite(&si_desc.ReturnType, sizeof(si_desc.ReturnType), 1, fp);
				fwrite(&si_desc.Dimension, sizeof(si_desc.Dimension), 1, fp);
				fwrite(&si_desc.NumSamples, sizeof(si_desc.NumSamples), 1, fp);
			}

			UINT const shader_type = D3D11_SHVER_GET_TYPE(desc.Version);
			if (shader_type == D3D11_SHVER_VERTEX_SHADER)
			{
				D3D11_SIGNATURE_PARAMETER_DESC_47 signature;
				for (UINT i = 0; i < desc.InputParameters; ++ i)
				{
					reflection->GetInputParameterDesc(i, reinterpret_cast<D3D11_SIGNATURE_PARAMETER_DESC*>(&signature));
					WriteString(signature.SemanticName, fp);
					fwrite(&signature.SemanticIndex, sizeof(signature.SemanticIndex), 1, fp);
					fwrite(&signature.Register, sizeof(signature.Register), 1, fp);
					fwrite(&signature.SystemValueType, sizeof(signature.SystemValueType), 1, fp);
					fwrite(&signature.ComponentType, sizeof(signature.ComponentType), 1, fp);
					fwrite(&signature.Mask, sizeof(signature.Mask), 1, fp);
					fwrite(&signature.ReadWriteMask, sizeof(signature.ReadWriteMask), 1, fp);
					fwrite(&signature.Stream, sizeof(signature.Stream), 1, fp);
					fwrite(&signature.MinPrecision, sizeof(signature.MinPrecision), 1, fp);
				}
			}
			else if (shader_type == D3D11_SHVER_COMPUTE_SHADER)
			{
				UINT cs_block_size[3];
				reflection->GetThreadGroupSize(&cs_block_size[0], &cs_block_size[1], &cs_block_size[2]);
				fwrite(cs_block_size, sizeof(cs_block_size), 1, fp);
			}

			reflection->Release();

			fclose(fp);
		}

		delete[] bytecode;
	}
	else if (0 == strcmp(argv[1], "strip"))
	{
		if (argc < 5)
		{
			PrintHelps();
			return -1;
		}

		char const * input_file = argv[2];
		int flags = atoi(argv[3]);
		char const * output_file = argv[4];

		FILE* fp = fopen(input_file, "rb");
		fseek(fp, 0, SEEK_END);
		long bytecode_size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		char* bytecode = new char[bytecode_size];
		fread(bytecode, sizeof(char), bytecode_size, fp);
		fclose(fp);

		ID3DBlob* code = NULL;
		int hr = d3d_compiler.D3DStripShader(bytecode, bytecode_size, flags, &code);
		if (FAILED(hr))
		{
			printf("Strip error: 0x%x\n", hr);
		}

		fp = fopen(output_file, "wb");
		fwrite(code->GetBufferPointer(), sizeof(char), code->GetBufferSize(), fp);
		fclose(fp);

		delete[] bytecode;
	}

	return 0;
}
