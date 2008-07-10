// D3D9ShaderObject.cpp
// KlayGE D3D9 shader对象类 实现文件
// Ver 3.7.0
// 版权所有(C) 龚敏敏, 2006-2008
// Homepage: http://klayge.sourceforge.net
//
// 3.7.0
// 改为直接传入RenderEffect (2008.7.4)
//
// 3.5.0
// 初次建立 (2006.11.2)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/COMPtr.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Sampler.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEffect.hpp>

#include <string>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <boost/assert.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>

#include <KlayGE/D3D9/D3D9RenderEngine.hpp>
#include <KlayGE/D3D9/D3D9Mapping.hpp>
#include <KlayGE/D3D9/D3D9Texture.hpp>
#include <KlayGE/D3D9/D3D9ShaderObject.hpp>

namespace KlayGE
{
	D3D9ShaderObject::D3D9ShaderObject()
	{
		is_shader_validate_.assign(true);
	}

	void D3D9ShaderObject::SetShader(RenderEffect& effect, ShaderType type, boost::shared_ptr<std::vector<shader_desc> > const & shader_descs,
			boost::shared_ptr<std::string> const & shader_text)
	{
		is_shader_validate_[type] = true;

		D3D9RenderEngine const & render_eng = *checked_cast<D3D9RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D9DevicePtr const & d3d_device = render_eng.D3DDevice();

		std::string shader_profile = (*shader_descs)[type].profile;
		switch (type)
		{
		case ST_VertexShader:
			if ("auto" == shader_profile)
			{
				shader_profile = D3DXGetVertexShaderProfile(d3d_device.get());

				if ((2 == render_eng.DeviceCaps().max_shader_model) && ("vs_3_0" == shader_profile))
				{
					// Fix for Intel DAMN on-board GPUs
					shader_profile = "vs_2_0";
				}
			}
			break;

		case ST_PixelShader:
			if ("auto" == shader_profile)
			{
				shader_profile = D3DXGetPixelShaderProfile(d3d_device.get());
			}
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}

		ID3DXConstantTable* constant_table = NULL;

		ID3DXBuffer* code;
		ID3DXBuffer* err_msg;
		D3DXCompileShader(shader_text->c_str(), static_cast<UINT>(shader_text->size()), NULL, NULL,
			(*shader_descs)[type].func_name.c_str(), shader_profile.c_str(),
			0, &code, &err_msg, &constant_table);
		if (err_msg != NULL)
		{
#ifdef D3DXSHADER_USE_LEGACY_D3DX9_31_DLL
			ID3DXConstantTable* constant_table_legacy;
			ID3DXBuffer* code_legacy;
			ID3DXBuffer* err_msg_legacy;
			D3DXCompileShader(shader_text->c_str(), static_cast<UINT>(shader_text->size()), NULL, NULL,
				(*shader_descs)[type].func_name.c_str(), shader_profile.c_str(),
				D3DXSHADER_USE_LEGACY_D3DX9_31_DLL, &code_legacy, &err_msg_legacy, &constant_table_legacy);
			if (err_msg_legacy != NULL)
			{
#ifdef KLAYGE_DEBUG
				std::cerr << *shader_text << std::endl;
				std::cerr << static_cast<char*>(err_msg_legacy->GetBufferPointer()) << std::endl;
#endif

				if (code_legacy)
				{
					code_legacy->Release();
				}
				if (constant_table_legacy)
				{
					constant_table_legacy->Release();
				}
				if (err_msg_legacy)
				{
					err_msg_legacy->Release();
				}

				if (code)
				{
					code->Release();
				}
				if (constant_table)
				{
					constant_table->Release();
				}
				if (err_msg)
				{
					err_msg->Release();
				}
			}
			else
			{
				if (code)
				{
					code->Release();
				}
				if (constant_table)
				{
					constant_table->Release();
				}
				if (err_msg)
				{
					err_msg->Release();
				}

				code = code_legacy;
				constant_table = constant_table_legacy;
			}
#else
#ifdef KLAYGE_DEBUG
			std::cerr << *shader_text << std::endl;
			std::cerr << static_cast<char*>(err_msg->GetBufferPointer()) << std::endl;
#endif

			err_msg->Release();
#endif
		}

		if (NULL == code)
		{
			is_shader_validate_[type] = false;
		}
		else
		{
			switch (type)
			{
			case ST_VertexShader:
				IDirect3DVertexShader9* vs;
				if (FAILED(d3d_device->CreateVertexShader(static_cast<DWORD*>(code->GetBufferPointer()), &vs)))
				{
					is_shader_validate_[type] = false;
				}
				vertex_shader_ = MakeCOMPtr(vs);
				break;

			case ST_PixelShader:
				IDirect3DPixelShader9* ps;
				if (FAILED(d3d_device->CreatePixelShader(static_cast<DWORD*>(code->GetBufferPointer()), &ps)))
				{
					is_shader_validate_[type] = false;
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
			}

			if (bool_end > bool_begin)
			{
				bool_registers_[type].resize(bool_end - bool_begin);
				bool_start_[type] = bool_begin;
			}
			if (int_end > int_begin)
			{
				int_registers_[type].resize(int_end - int_begin);
				int_start_[type] = int_begin;
			}
			if (float_end > float_begin)
			{
				float_registers_[type].resize(float_end - float_begin);
				float_start_[type] = float_begin;
			}

			for (UINT c = 0; c < ct_desc.Constants; ++ c)
			{
				D3DXHANDLE handle = constant_table->GetConstant(NULL, c);
				D3DXCONSTANT_DESC constant_desc;
				UINT count;
				constant_table->GetConstantDesc(handle, &constant_desc, &count);

				D3D9ShaderParameterHandle p_handle;
				p_handle.shader_type = static_cast<uint8_t>(type);
				p_handle.register_set = static_cast<uint8_t>(constant_desc.RegisterSet);
				switch (p_handle.register_set)
				{
				case D3DXRS_BOOL:
					p_handle.register_index = static_cast<uint16_t>(constant_desc.RegisterIndex - bool_start_[type]);
					break;

				case D3DXRS_INT4:
					p_handle.register_index = static_cast<uint16_t>(constant_desc.RegisterIndex - int_start_[type]);
					break;

				case D3DXRS_FLOAT4:
					p_handle.register_index = static_cast<uint16_t>(constant_desc.RegisterIndex - float_start_[type]);
					break;

				default:
					p_handle.register_index = static_cast<uint16_t>(constant_desc.RegisterIndex);
					break;
				}
				p_handle.register_count = static_cast<uint16_t>(constant_desc.RegisterCount);
				p_handle.rows = static_cast<uint8_t>(constant_desc.Rows);
				p_handle.columns = static_cast<uint8_t>(constant_desc.Columns);

				RenderEffectParameterPtr const & p = effect.ParameterByName(constant_desc.Name);
				if (p != RenderEffectParameter::NullObject())
				{
					param_binds_[type].push_back(this->GetBindFunc(p_handle, p));
				}
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

		is_validate_ = true;
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			is_validate_ &= is_shader_validate_[i];
		}
	}

	ShaderObjectPtr D3D9ShaderObject::Clone(RenderEffect& effect)
	{
		D3D9ShaderObjectPtr ret(new D3D9ShaderObject);
		ret->is_validate_ = is_validate_;
		ret->is_shader_validate_ = is_shader_validate_;
		ret->vertex_shader_ = vertex_shader_;
		ret->pixel_shader_ = pixel_shader_;
		ret->bool_start_ = bool_start_;
		ret->int_start_ = int_start_;
		ret->float_start_ = float_start_;
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			ret->bool_registers_[i].resize(bool_registers_[i].size());
			ret->int_registers_[i].resize(int_registers_[i].size());
			ret->float_registers_[i].resize(float_registers_[i].size());
			ret->samplers_[i].resize(samplers_[i].size());

			ret->param_binds_[i].reserve(param_binds_[i].size());
			BOOST_FOREACH(BOOST_TYPEOF(param_binds_[i])::const_reference pb, param_binds_[i])
			{
				ret->param_binds_[i].push_back(ret->GetBindFunc(pb.p_handle, effect.ParameterByName(*(pb.param->Name()))));
			}
		}

		return ret;
	}

	void D3D9ShaderObject::SetBoolToBool(D3D9ShaderParameterHandle const & p_handle, RenderEffectParameterPtr const & param)
	{
		bool v;
		param->Value(v);

		bool_registers_[p_handle.shader_type][p_handle.register_index].x() = v;
	}

	void D3D9ShaderObject::SetBoolToInt(D3D9ShaderParameterHandle const & p_handle, RenderEffectParameterPtr const & param)
	{
		bool v;
		param->Value(v);

		int_registers_[p_handle.shader_type][p_handle.register_index].x() = v;
	}

	void D3D9ShaderObject::SetBoolToFloat(D3D9ShaderParameterHandle const & p_handle, RenderEffectParameterPtr const & param)
	{
		bool v;
		param->Value(v);

		float_registers_[p_handle.shader_type][p_handle.register_index].x() = v;
	}

	void D3D9ShaderObject::SetIntToBool(D3D9ShaderParameterHandle const & p_handle, RenderEffectParameterPtr const & param)
	{
		int v;
		param->Value(v);

		bool_registers_[p_handle.shader_type][p_handle.register_index].x() = v;
	}

	void D3D9ShaderObject::SetIntToInt(D3D9ShaderParameterHandle const & p_handle, RenderEffectParameterPtr const & param)
	{
		int v;
		param->Value(v);

		int_registers_[p_handle.shader_type][p_handle.register_index].x() = v;
	}

	void D3D9ShaderObject::SetIntToFloat(D3D9ShaderParameterHandle const & p_handle, RenderEffectParameterPtr const & param)
	{
		int v;
		param->Value(v);

		float_registers_[p_handle.shader_type][p_handle.register_index].x() = static_cast<float>(v);
	}

	void D3D9ShaderObject::SetFloat(D3D9ShaderParameterHandle const & p_handle, RenderEffectParameterPtr const & param)
	{
		BOOST_ASSERT(D3DXRS_FLOAT4 == p_handle.register_set);

		float v;
		param->Value(v);

		float_registers_[p_handle.shader_type][p_handle.register_index].x() = v;
	}

	void D3D9ShaderObject::SetFloat2(D3D9ShaderParameterHandle const & p_handle, RenderEffectParameterPtr const & param)
	{
		BOOST_ASSERT(D3DXRS_FLOAT4 == p_handle.register_set);

		float2 v;
		param->Value(v);

		float4& v4 = float_registers_[p_handle.shader_type][p_handle.register_index];
		v4.x() = v.x();
		v4.y() = v.y();
	}

	void D3D9ShaderObject::SetFloat3(D3D9ShaderParameterHandle const & p_handle, RenderEffectParameterPtr const & param)
	{
		BOOST_ASSERT(D3DXRS_FLOAT4 == p_handle.register_set);

		float3 v;
		param->Value(v);

		float4& v4 = float_registers_[p_handle.shader_type][p_handle.register_index];
		v4.x() = v.x();
		v4.y() = v.y();
		v4.z() = v.z();
	}

	void D3D9ShaderObject::SetFloat4(D3D9ShaderParameterHandle const & p_handle, RenderEffectParameterPtr const & param)
	{
		BOOST_ASSERT(D3DXRS_FLOAT4 == p_handle.register_set);

		float4 v;
		param->Value(v);

		float_registers_[p_handle.shader_type][p_handle.register_index] = v;
	}

	void D3D9ShaderObject::SetFloat4x4(D3D9ShaderParameterHandle const & p_handle, RenderEffectParameterPtr const & param)
	{
		BOOST_ASSERT(D3DXRS_FLOAT4 == p_handle.register_set);

		float4x4 v;
		param->Value(v);

		v = MathLib::transpose(v);
		memcpy(&float_registers_[p_handle.shader_type][p_handle.register_index], &v[0], p_handle.register_count * sizeof(float4));
	}

	void D3D9ShaderObject::SetBoolArrayToBool(D3D9ShaderParameterHandle const & p_handle, RenderEffectParameterPtr const & param)
	{
		std::vector<bool> v;
		param->Value(v);

		for (size_t i = 0; i < v.size(); ++ i)
		{
			bool_registers_[p_handle.shader_type][p_handle.register_index + i].x() = v[i];
		}
	}

	void D3D9ShaderObject::SetBoolArrayToInt(D3D9ShaderParameterHandle const & p_handle, RenderEffectParameterPtr const & param)
	{
		std::vector<bool> v;
		param->Value(v);

		for (size_t i = 0; i < v.size(); ++ i)
		{
			int_registers_[p_handle.shader_type][p_handle.register_index + i].x() = v[i];
		}
	}

	void D3D9ShaderObject::SetBoolArrayToFloat(D3D9ShaderParameterHandle const & p_handle, RenderEffectParameterPtr const & param)
	{
		std::vector<bool> v;
		param->Value(v);

		for (size_t i = 0; i < v.size(); ++ i)
		{
			float_registers_[p_handle.shader_type][p_handle.register_index + i].x() = v[i];
		}
	}

	void D3D9ShaderObject::SetIntArrayToBool(D3D9ShaderParameterHandle const & p_handle, RenderEffectParameterPtr const & param)
	{
		std::vector<int> v;
		param->Value(v);

		for (size_t i = 0; i < v.size(); ++ i)
		{
			bool_registers_[p_handle.shader_type][p_handle.register_index + i].x() = v[i];
		}
	}

	void D3D9ShaderObject::SetIntArrayToInt(D3D9ShaderParameterHandle const & p_handle, RenderEffectParameterPtr const & param)
	{
		std::vector<int> v;
		param->Value(v);

		for (size_t i = 0; i < v.size(); ++ i)
		{
			int_registers_[p_handle.shader_type][p_handle.register_index + i].x() = v[i];
		}
	}

	void D3D9ShaderObject::SetIntArrayToFloat(D3D9ShaderParameterHandle const & p_handle, RenderEffectParameterPtr const & param)
	{
		std::vector<int> v;
		param->Value(v);

		for (size_t i = 0; i < v.size(); ++ i)
		{
			float_registers_[p_handle.shader_type][p_handle.register_index + i].x() = static_cast<float>(v[i]);
		}
	}

	void D3D9ShaderObject::SetFloatArray(D3D9ShaderParameterHandle const & p_handle, RenderEffectParameterPtr const & param)
	{
		BOOST_ASSERT(D3DXRS_FLOAT4 == p_handle.register_set);

		std::vector<float> v;
		param->Value(v);

		for (size_t i = 0; i < v.size(); ++ i)
		{
			float_registers_[p_handle.shader_type][p_handle.register_index + i].x() = v[i];
		}
	}

	void D3D9ShaderObject::SetFloat4Array(D3D9ShaderParameterHandle const & p_handle, RenderEffectParameterPtr const & param)
	{
		std::vector<float4> v;
		param->Value(v);
							
		if (!v.empty())
		{
			memcpy(&float_registers_[p_handle.shader_type][p_handle.register_index], &v[0],
						std::min(p_handle.register_count, static_cast<uint16_t>(v.size())) * sizeof(float4));
		}
	}

	void D3D9ShaderObject::SetFloat4x4Array(D3D9ShaderParameterHandle const & p_handle, RenderEffectParameterPtr const & param)
	{
		std::vector<float4x4> v;
		param->Value(v);
							

		uint32_t start = p_handle.register_index;
		BOOST_FOREACH(BOOST_TYPEOF(v)::reference mat, v)
		{
			mat = MathLib::transpose(mat);
			memcpy(&float_registers_[p_handle.shader_type][start], &mat[0], p_handle.rows * sizeof(float4));
			start += p_handle.rows;
		}
	}

	void D3D9ShaderObject::SetSampler(D3D9ShaderParameterHandle const & p_handle, RenderEffectParameterPtr const & param)
	{
		BOOST_ASSERT(p_handle.register_index < samplers_[p_handle.shader_type].size());

		SamplerPtr v;
		param->Value(v);

		samplers_[p_handle.shader_type][p_handle.register_index] = v;
	}

	D3D9ShaderObject::parameter_bind_t D3D9ShaderObject::GetBindFunc(D3D9ShaderParameterHandle const & p_handle, RenderEffectParameterPtr const & param)
	{
		parameter_bind_t ret;
		ret.param = param;
		ret.p_handle = p_handle;

		switch (param->type())
		{
		case REDT_bool:
			if (param->ArraySize() != 0)
			{
				switch (p_handle.register_set)
				{
				case D3DXRS_BOOL:
					ret.func = boost::bind(&D3D9ShaderObject::SetBoolArrayToBool, this, _1, _2);
					break;

				case D3DXRS_INT4:
					ret.func = boost::bind(&D3D9ShaderObject::SetBoolArrayToInt, this, _1, _2);
					break;

				case D3DXRS_FLOAT4:
					ret.func = boost::bind(&D3D9ShaderObject::SetBoolArrayToFloat, this, _1, _2);
					break;

				default:
					BOOST_ASSERT(false);
					break;
				}
			}
			else
			{
				switch (p_handle.register_set)
				{
				case D3DXRS_BOOL:
					ret.func = boost::bind(&D3D9ShaderObject::SetBoolToBool, this, _1, _2);
					break;

				case D3DXRS_INT4:
					ret.func = boost::bind(&D3D9ShaderObject::SetBoolToInt, this, _1, _2);
					break;

				case D3DXRS_FLOAT4:
					ret.func = boost::bind(&D3D9ShaderObject::SetBoolToFloat, this, _1, _2);
					break;

				default:
					BOOST_ASSERT(false);
					break;
				}
			}
			break;

		case REDT_dword:
		case REDT_int:
			if (param->ArraySize() != 0)
			{
				switch (p_handle.register_set)
				{
				case D3DXRS_BOOL:
					ret.func = boost::bind(&D3D9ShaderObject::SetIntArrayToBool, this, _1, _2);
					break;

				case D3DXRS_INT4:
					ret.func = boost::bind(&D3D9ShaderObject::SetIntArrayToInt, this, _1, _2);
					break;

				case D3DXRS_FLOAT4:
					ret.func = boost::bind(&D3D9ShaderObject::SetIntArrayToFloat, this, _1, _2);
					break;

				default:
					BOOST_ASSERT(false);
					break;
				}
			}
			else
			{
				switch (p_handle.register_set)
				{
				case D3DXRS_BOOL:
					ret.func = boost::bind(&D3D9ShaderObject::SetIntToBool, this, _1, _2);
					break;

				case D3DXRS_INT4:
					ret.func = boost::bind(&D3D9ShaderObject::SetIntToInt, this, _1, _2);
					break;

				case D3DXRS_FLOAT4:
					ret.func = boost::bind(&D3D9ShaderObject::SetIntToFloat, this, _1, _2);
					break;

				default:
					BOOST_ASSERT(false);
					break;
				}
			}
			break;

		case REDT_float:
			if (param->ArraySize() != 0)
			{
				ret.func = boost::bind(&D3D9ShaderObject::SetFloatArray, this, _1, _2);
			}
			else
			{
				ret.func = boost::bind(&D3D9ShaderObject::SetFloat, this, _1, _2);
			}
			break;

		case REDT_float2:
			ret.func = boost::bind(&D3D9ShaderObject::SetFloat2, this, _1, _2);
			break;

		case REDT_float3:
			ret.func = boost::bind(&D3D9ShaderObject::SetFloat3, this, _1, _2);
			break;

		case REDT_float4:
			if (param->ArraySize() != 0)
			{
				ret.func = boost::bind(&D3D9ShaderObject::SetFloat4Array, this, _1, _2);
			}
			else
			{
				ret.func = boost::bind(&D3D9ShaderObject::SetFloat4, this, _1, _2);
			}
			break;

		case REDT_float4x4:
			if (param->ArraySize() != 0)
			{
				ret.func = boost::bind(&D3D9ShaderObject::SetFloat4x4Array, this, _1, _2);
			}
			else
			{
				ret.func = boost::bind(&D3D9ShaderObject::SetFloat4x4, this, _1, _2);
			}
			break;

		case REDT_sampler1D:
		case REDT_sampler2D:
		case REDT_sampler3D:
		case REDT_samplerCUBE:
			ret.func = boost::bind(&D3D9ShaderObject::SetSampler, this, _1, _2);
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}

		return ret;
	}

	void D3D9ShaderObject::Active()
	{
		D3D9RenderEngine& re = *checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		re.SetVertexShader(vertex_shader_.get());
		re.SetPixelShader(pixel_shader_.get());

		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			ShaderType type = static_cast<ShaderType>(i);

			BOOST_FOREACH(BOOST_TYPEOF(param_binds_[i])::reference pb, param_binds_[i])
			{
				pb.func(pb.p_handle, pb.param);
			}

			if (!bool_registers_[type].empty())
			{
				if (ST_VertexShader == type)
				{
					re.SetVertexShaderConstantB(bool_start_[type], &bool_registers_[type][0].x(),
						static_cast<uint32_t>(bool_registers_[type].size()));
				}
				else
				{
					re.SetPixelShaderConstantB(bool_start_[type], &bool_registers_[type][0].x(),
						static_cast<uint32_t>(bool_registers_[type].size()));
				}
			}
			if (!int_registers_[type].empty())
			{
				if (ST_VertexShader == type)
				{
					re.SetVertexShaderConstantI(int_start_[type], &int_registers_[type][0].x(),
						static_cast<uint32_t>(int_registers_[type].size()));
				}
				else
				{
					re.SetPixelShaderConstantI(int_start_[type], &int_registers_[type][0].x(),
						static_cast<uint32_t>(int_registers_[type].size()));
				}
			}
			if (!float_registers_[type].empty())
			{
				if (ST_VertexShader == type)
				{
					re.SetVertexShaderConstantF(float_start_[type], &float_registers_[type][0].x(),
						static_cast<uint32_t>(float_registers_[type].size()));
				}
				else
				{
					re.SetPixelShaderConstantF(float_start_[type], &float_registers_[type][0].x(),
						static_cast<uint32_t>(float_registers_[type].size()));
				}
			}

			for (size_t j = 0, j_end = samplers_[type].size(); j < j_end; ++ j)
			{
				uint32_t stage = static_cast<uint32_t>(j);
				if (ST_VertexShader == type)
				{
					stage += D3DVERTEXTEXTURESAMPLER0;
				}

				SamplerPtr const & sampler = samplers_[type][j];
				if (!sampler || !sampler->texture)
				{
					re.SetTexture(stage, NULL);
				}
				else
				{
					D3D9Texture const & d3d9Tex = *checked_pointer_cast<D3D9Texture>(sampler->texture);
					re.SetTexture(stage, d3d9Tex.D3DBaseTexture().get());
					re.SetSamplerState(stage, D3DSAMP_SRGBTEXTURE, IsSRGB(sampler->texture->Format()));

					re.SetSamplerState(stage, D3DSAMP_BORDERCOLOR, D3D9Mapping::MappingToUInt32Color(sampler->border_clr));

					re.SetSamplerState(stage, D3DSAMP_ADDRESSU, D3D9Mapping::Mapping(sampler->addr_mode_u));
					re.SetSamplerState(stage, D3DSAMP_ADDRESSV, D3D9Mapping::Mapping(sampler->addr_mode_v));
					re.SetSamplerState(stage, D3DSAMP_ADDRESSW, D3D9Mapping::Mapping(sampler->addr_mode_w));

					switch (sampler->filter)
					{
					case Sampler::TFO_Point:
						re.SetSamplerState(stage, D3DSAMP_MINFILTER, D3DTEXF_POINT);
						re.SetSamplerState(stage, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
						re.SetSamplerState(stage, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
						break;

					case Sampler::TFO_Bilinear:
						re.SetSamplerState(stage, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
						re.SetSamplerState(stage, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
						re.SetSamplerState(stage, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
						break;

					case Sampler::TFO_Trilinear:
						re.SetSamplerState(stage, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
						re.SetSamplerState(stage, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
						re.SetSamplerState(stage, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
						break;

					case Sampler::TFO_Anisotropic:
						re.SetSamplerState(stage, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
						re.SetSamplerState(stage, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
						re.SetSamplerState(stage, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
						break;

					default:
						BOOST_ASSERT(false);
						re.SetSamplerState(stage, D3DSAMP_MINFILTER, D3DTEXF_POINT);
						re.SetSamplerState(stage, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
						re.SetSamplerState(stage, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
						break;
					}

					re.SetSamplerState(stage, D3DSAMP_MAXANISOTROPY, sampler->anisotropy);
					re.SetSamplerState(stage, D3DSAMP_MAXMIPLEVEL, sampler->max_mip_level);
					re.SetSamplerState(stage, D3DSAMP_MIPMAPLODBIAS, float_to_uint32(sampler->mip_map_lod_bias));
				}
			}
		}
	}
}
