// D3D9ShaderObject.cpp
// KlayGE D3D9 shader对象类 实现文件
// Ver 3.5.0
// 版权所有(C) 龚敏敏, 2003-2006
// Homepage: http://klayge.sourceforge.net
//
// 3.5.0
// 初次建立 (2006.11.2)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/COMPtr.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Sampler.hpp>

#include <boost/assert.hpp>
#include <string>
#include <algorithm>
#include <sstream>
#include <iostream>

#include <KlayGE/D3D9/D3D9RenderEngine.hpp>
#include <KlayGE/D3D9/D3D9Mapping.hpp>
#include <KlayGE/D3D9/D3D9ShaderObject.hpp>

namespace KlayGE
{
	void D3D9ShaderObject::SetShader(ShaderType type, std::string const & profile, std::string const & name, std::string const & text)
	{
		is_validate_ = true;

		D3D9RenderEngine& render_eng(*checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		ID3D9DevicePtr d3d_device = render_eng.D3DDevice();

		std::string shader_profile = profile;
		switch (type)
		{
		case ST_VertexShader:
			if ("auto" == profile)
			{
				shader_profile = D3DXGetVertexShaderProfile(d3d_device.get());
			}
			break;

		case ST_PixelShader:
			if ("auto" == profile)
			{
				shader_profile = D3DXGetPixelShaderProfile(d3d_device.get());
			}
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}

		ID3DXConstantTable* constant_table;

		ID3DXBuffer* code;
		ID3DXBuffer* err_msg;
		D3DXCompileShader(text.c_str(), static_cast<UINT>(text.size()), NULL, NULL,
			name.c_str(), shader_profile.c_str(),
			0, &code, &err_msg, &constant_table);
		if (err_msg != NULL)
		{
#ifdef KLAYGE_DEBUG
			std::cerr << text << std::endl;
			std::cerr << static_cast<char*>(err_msg->GetBufferPointer()) << std::endl;
#endif
			err_msg->Release();
		}

		if (NULL == code)
		{
			is_validate_ = false;
		}
		else
		{
			switch (type)
			{
			case ST_VertexShader:
				IDirect3DVertexShader9* vs;
				if (FAILED(d3d_device->CreateVertexShader(static_cast<DWORD*>(code->GetBufferPointer()), &vs)))
				{
					is_validate_ = false;
				}
				vertex_shader_ = MakeCOMPtr(vs);
				break;

			case ST_PixelShader:
				IDirect3DPixelShader9* ps;
				if (FAILED(d3d_device->CreatePixelShader(static_cast<DWORD*>(code->GetBufferPointer()), &ps)))
				{
					is_validate_ = false;
				}
				pixel_shader_ = MakeCOMPtr(ps);
				break;

			default:
				BOOST_ASSERT(false);
				break;
			}

			code->Release();
		}

		if (constant_table)
		{
			uint32_t bool_begin = 0xFFFFFFFF, bool_end = 0;
			uint32_t int_begin = 0xFFFFFFFF, int_end = 0;
			uint32_t float_begin = 0xFFFFFFFF, float_end = 0;

			D3DXCONSTANTTABLE_DESC ct_desc;
			constant_table->GetDesc(&ct_desc);
			for (UINT c = 0; c < ct_desc.Constants; ++ c)
			{
				D3DXHANDLE handle = constant_table->GetConstant(NULL, c);
				D3DXCONSTANT_DESC constant_desc;
				UINT count;
				constant_table->GetConstantDesc(handle, &constant_desc, &count);

				switch (constant_desc.RegisterSet)
				{
				case D3DXRS_BOOL:
					bool_begin = std::min<uint32_t>(bool_begin, constant_desc.RegisterIndex);
					bool_end = std::max<uint32_t>(bool_end, constant_desc.RegisterIndex + constant_desc.RegisterCount);
					break;

				case D3DXRS_INT4:
					int_begin = std::min<uint32_t>(int_begin, constant_desc.RegisterIndex);
					int_end = std::max<uint32_t>(int_end, constant_desc.RegisterIndex + constant_desc.RegisterCount);
					break;

				case D3DXRS_FLOAT4:
					float_begin = std::min<uint32_t>(float_begin, constant_desc.RegisterIndex);
					float_end = std::max<uint32_t>(float_end, constant_desc.RegisterIndex + constant_desc.RegisterCount);
					break;

				default:
					break;
				}

				D3D9ShaderParameterHandle p_handle;
				p_handle.shader_type = static_cast<uint8_t>(type);
				p_handle.register_set = static_cast<uint8_t>(constant_desc.RegisterSet);
				p_handle.register_index = static_cast<uint16_t>(constant_desc.RegisterIndex);
				p_handle.register_count = static_cast<uint16_t>(constant_desc.RegisterCount);
				p_handle.rows = static_cast<uint8_t>(constant_desc.Rows);
				p_handle.columns = static_cast<uint8_t>(constant_desc.Columns);

				param_descs_[type].insert(std::make_pair(constant_desc.Name, p_handle));
			}

			if (bool_end > bool_begin)
			{
				bool_registers_[type].resize((bool_end - bool_begin) * 4);
				bool_start_[type] = bool_begin;
			}
			if (int_end > int_begin)
			{
				int_registers_[type].resize((int_end - int_begin) * 4);
				int_start_[type] = int_begin;
			}
			if (float_end > float_begin)
			{
				float_registers_[type].resize((float_end - float_begin) * 4);
				float_start_[type] = float_begin;
			}

			constant_table->Release();
		}

		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		switch (type)
		{
		case ST_VertexShader:
			samplers_[type].resize(re.DeviceCaps().max_vertex_texture_units);
			break;

		case ST_PixelShader:
			samplers_[type].resize(re.DeviceCaps().max_texture_units);
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}
	}

	bool D3D9ShaderObject::HasParameter(ShaderType type, std::string const & name) const
	{
		return param_descs_[type].find(name) != param_descs_[type].end();
	}

	void D3D9ShaderObject::SetParameter(std::string const & name, bool value)
	{
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			ShaderType type = static_cast<ShaderType>(i);

			parameter_descs_t::iterator iter = param_descs_[type].find(name);
			if (iter != param_descs_[type].end())
			{
				D3D9ShaderParameterHandle const & p_handle = iter->second;

				switch (p_handle.register_set)
				{
				case D3DXRS_BOOL:
					bool_registers_[p_handle.shader_type][(p_handle.register_index - bool_start_[p_handle.shader_type]) * 4] = value;
					break;

				case D3DXRS_INT4:
					int_registers_[p_handle.shader_type][(p_handle.register_index - int_start_[p_handle.shader_type]) * 4] = value;
					break;

				case D3DXRS_FLOAT4:
					float_registers_[p_handle.shader_type][(p_handle.register_index - float_start_[p_handle.shader_type]) * 4] = value;
					break;

				default:
					BOOST_ASSERT(false);
					break;
				}
			}
		}
	}

	void D3D9ShaderObject::SetParameter(std::string const & name, int value)
	{
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			ShaderType type = static_cast<ShaderType>(i);
			
			parameter_descs_t::iterator iter = param_descs_[type].find(name);
			if (iter != param_descs_[type].end())
			{
				D3D9ShaderParameterHandle const & p_handle = iter->second;

				switch (p_handle.register_set)
				{
				case D3DXRS_BOOL:
					bool_registers_[p_handle.shader_type][(p_handle.register_index - bool_start_[p_handle.shader_type]) * 4] = value;
					break;

				case D3DXRS_INT4:
					int_registers_[p_handle.shader_type][(p_handle.register_index - int_start_[p_handle.shader_type]) * 4] = value;
					break;

				case D3DXRS_FLOAT4:
					float_registers_[p_handle.shader_type][(p_handle.register_index - float_start_[p_handle.shader_type]) * 4] = static_cast<float>(value);
					break;

				default:
					BOOST_ASSERT(false);
					break;
				}
			}
		}
	}

	void D3D9ShaderObject::SetParameter(std::string const & name, float value)
	{
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			ShaderType type = static_cast<ShaderType>(i);
			
			parameter_descs_t::iterator iter = param_descs_[type].find(name);
			if (iter != param_descs_[type].end())
			{
				D3D9ShaderParameterHandle const & p_handle = iter->second;
				BOOST_ASSERT(D3DXRS_FLOAT4 == p_handle.register_set);

				float_registers_[p_handle.shader_type][(p_handle.register_index - float_start_[p_handle.shader_type]) * 4] = value;
			}
		}
	}

	void D3D9ShaderObject::SetParameter(std::string const & name, float4 const & value)
	{
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			ShaderType type = static_cast<ShaderType>(i);
			
			parameter_descs_t::iterator iter = param_descs_[type].find(name);
			if (iter != param_descs_[type].end())
			{
				D3D9ShaderParameterHandle const & p_handle = iter->second;
				BOOST_ASSERT(D3DXRS_FLOAT4 == p_handle.register_set);

				memcpy(&float_registers_[p_handle.shader_type][(p_handle.register_index - float_start_[p_handle.shader_type]) * 4], &value[0], sizeof(value));
			}
		}
	}

	void D3D9ShaderObject::SetParameter(std::string const & name, float4x4 const & value)
	{
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			ShaderType type = static_cast<ShaderType>(i);
			
			parameter_descs_t::iterator iter = param_descs_[type].find(name);
			if (iter != param_descs_[type].end())
			{
				D3D9ShaderParameterHandle const & p_handle = iter->second;
				BOOST_ASSERT(D3DXRS_FLOAT4 == p_handle.register_set);

				float4x4 tmp = MathLib::transpose(value);
				memcpy(&float_registers_[p_handle.shader_type][(p_handle.register_index - float_start_[p_handle.shader_type]) * 4], &tmp[0], p_handle.register_count * sizeof(float4));
			}
		}
	}

	void D3D9ShaderObject::SetParameter(std::string const & name, std::vector<bool> const & value)
	{
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			ShaderType type = static_cast<ShaderType>(i);
			
			parameter_descs_t::iterator iter = param_descs_[type].find(name);
			if (iter != param_descs_[type].end())
			{
				D3D9ShaderParameterHandle const & p_handle = iter->second;

				switch (p_handle.register_set)
				{
				case D3DXRS_BOOL:
					for (size_t i = 0; i < value.size(); ++ i)
					{
						bool_registers_[p_handle.shader_type][(p_handle.register_index - bool_start_[p_handle.shader_type] + i) * 4] = value[i];
					}
					break;

				case D3DXRS_INT4:
					for (size_t i = 0; i < value.size(); ++ i)
					{
						int_registers_[p_handle.shader_type][(p_handle.register_index - int_start_[p_handle.shader_type] + i) * 4] = value[i];
					}
					break;

				case D3DXRS_FLOAT4:
					for (size_t i = 0; i < value.size(); ++ i)
					{
						float_registers_[p_handle.shader_type][(p_handle.register_index - float_start_[p_handle.shader_type] + i) * 4] = value[i];
					}
					break;

				default:
					BOOST_ASSERT(false);
					break;
				}
			}
		}
	}

	void D3D9ShaderObject::SetParameter(std::string const & name, std::vector<int> const & value)
	{
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			ShaderType type = static_cast<ShaderType>(i);
			
			parameter_descs_t::iterator iter = param_descs_[type].find(name);
			if (iter != param_descs_[type].end())
			{
				D3D9ShaderParameterHandle const & p_handle = iter->second;

				switch (p_handle.register_set)
				{
				case D3DXRS_BOOL:
					for (size_t i = 0; i < value.size(); ++ i)
					{
						bool_registers_[p_handle.shader_type][(p_handle.register_index - bool_start_[p_handle.shader_type] + i) * 4] = value[i];
					}
					break;

				case D3DXRS_INT4:
					for (size_t i = 0; i < value.size(); ++ i)
					{
						int_registers_[p_handle.shader_type][(p_handle.register_index - int_start_[p_handle.shader_type] + i) * 4] = value[i];
					}
					break;

				case D3DXRS_FLOAT4:
					for (size_t i = 0; i < value.size(); ++ i)
					{
						float_registers_[p_handle.shader_type][(p_handle.register_index - float_start_[p_handle.shader_type] + i) * 4] = static_cast<float>(value[i]);
					}
					break;

				default:
					BOOST_ASSERT(false);
					break;
				}
			}
		}
	}

	void D3D9ShaderObject::SetParameter(std::string const & name, std::vector<float> const & value)
	{
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			ShaderType type = static_cast<ShaderType>(i);
			
			parameter_descs_t::iterator iter = param_descs_[type].find(name);
			if (iter != param_descs_[type].end())
			{
				D3D9ShaderParameterHandle const & p_handle = iter->second;
				BOOST_ASSERT(D3DXRS_FLOAT4 == p_handle.register_set);

				for (size_t i = 0; i < value.size(); ++ i)
				{
					float_registers_[p_handle.shader_type][(p_handle.register_index - float_start_[p_handle.shader_type] + i) * 4] = value[i];
				}
			}
		}
	}

	void D3D9ShaderObject::SetParameter(std::string const & name, std::vector<float4> const & value)
	{
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			ShaderType type = static_cast<ShaderType>(i);
			
			parameter_descs_t::iterator iter = param_descs_[type].find(name);
			if (iter != param_descs_[type].end())
			{
				D3D9ShaderParameterHandle const & p_handle = iter->second;

				memcpy(&float_registers_[p_handle.shader_type][(p_handle.register_index - float_start_[p_handle.shader_type]) * 4], &value[0],
					std::min(p_handle.register_count, static_cast<uint16_t>(value.size())) * sizeof(float4));
			}
		}
	}

	void D3D9ShaderObject::SetParameter(std::string const & name, std::vector<float4x4> const & value)
	{
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			ShaderType type = static_cast<ShaderType>(i);
			
			parameter_descs_t::iterator iter = param_descs_[type].find(name);
			if (iter != param_descs_[type].end())
			{
				D3D9ShaderParameterHandle const & p_handle = iter->second;

				uint32_t start = p_handle.register_index;
				std::vector<float4x4> tmp(value);
				for (std::vector<float4x4>::iterator iter = tmp.begin(); iter != tmp.end(); ++ iter)
				{
					*iter = MathLib::transpose(*iter);
					memcpy(&float_registers_[p_handle.shader_type][(start - float_start_[p_handle.shader_type]) * 4], &(*iter)[0], p_handle.rows * sizeof(float4));
					start += p_handle.rows;
				}
			}
		}
	}

	void D3D9ShaderObject::SetParameter(std::string const & name, SamplerPtr const & value)
	{
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			ShaderType type = static_cast<ShaderType>(i);
			
			parameter_descs_t::iterator iter = param_descs_[type].find(name);
			if (iter != param_descs_[type].end())
			{
				D3D9ShaderParameterHandle const & p_handle = iter->second;

				BOOST_ASSERT(p_handle.register_index < samplers_[p_handle.shader_type].size());
				samplers_[p_handle.shader_type][p_handle.register_index] = value;
			}
		}
	}
}
