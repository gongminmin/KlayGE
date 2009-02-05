// D3D10ShaderObject.cpp
// KlayGE D3D10 shader对象类 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2008
// Homepage: http://klayge.sourceforge.net
//
// 3.8.0
// 初次建立 (2008.9.21)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/COMPtr.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEffect.hpp>

#include <string>
#include <map>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <boost/assert.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>

#include <KlayGE/D3D10/D3D10MinGWDefs.hpp>
#include <d3d10.h>
#include <d3dx10.h>

#include <KlayGE/D3D10/D3D10RenderEngine.hpp>
#include <KlayGE/D3D10/D3D10RenderStateObject.hpp>
#include <KlayGE/D3D10/D3D10Mapping.hpp>
#include <KlayGE/D3D10/D3D10Texture.hpp>
#include <KlayGE/D3D10/D3D10ShaderObject.hpp>

namespace
{
	using namespace KlayGE;

	template <typename SrcType, typename DstType>
	class SetD3D10ShaderParameter
	{
	public:
		SetD3D10ShaderParameter(uint8_t* target, RenderEffectParameterPtr const & param, char* dirty)
			: target_(reinterpret_cast<DstType*>(target)), param_(param), dirty_(dirty)
		{
		}

		void operator()()
		{
			SrcType v;
			param_->Value(v);

			if (*target_ != static_cast<DstType>(v))
			{
				*target_ = static_cast<DstType>(v);
				*dirty_ = true;
			}
		}

	private:
		DstType* target_;
		RenderEffectParameterPtr param_;
		char* dirty_;
	};

	template <>
	class SetD3D10ShaderParameter<float2, float>
	{
	public:
		SetD3D10ShaderParameter(uint8_t* target, RenderEffectParameterPtr const & param, char* dirty)
			: target_(reinterpret_cast<float2*>(target)), param_(param), dirty_(dirty)
		{
		}

		void operator()()
		{
			float2 v;
			param_->Value(v);

			if (*target_ != v)
			{
				*target_ = v;
				*dirty_ = true;
			}
		}

	private:
		float2* target_;
		RenderEffectParameterPtr param_;
		char* dirty_;
	};

	template <>
	class SetD3D10ShaderParameter<float3, float>
	{
	public:
		SetD3D10ShaderParameter(uint8_t* target, RenderEffectParameterPtr const & param, char* dirty)
			: target_(reinterpret_cast<float3*>(target)), param_(param), dirty_(dirty)
		{
		}

		void operator()()
		{
			float3 v;
			param_->Value(v);

			if (*target_ != v)
			{
				*target_ = v;
				*dirty_ = true;
			}
		}

	private:
		float3* target_;
		RenderEffectParameterPtr param_;
		char* dirty_;
	};

	template <>
	class SetD3D10ShaderParameter<float4, float>
	{
	public:
		SetD3D10ShaderParameter(uint8_t* target, RenderEffectParameterPtr const & param, char* dirty)
			: target_(reinterpret_cast<float4*>(target)), param_(param), dirty_(dirty)
		{
		}

		void operator()()
		{
			float4 v;
			param_->Value(v);

			if (*target_ != v)
			{
				*target_ = v;
				*dirty_ = true;
			}
		}

	private:
		float4* target_;
		RenderEffectParameterPtr param_;
		char* dirty_;
	};

	template <>
	class SetD3D10ShaderParameter<float4x4, float>
	{
	public:
		SetD3D10ShaderParameter(uint8_t* target, uint32_t rows, RenderEffectParameterPtr const & param, char* dirty)
			: target_(reinterpret_cast<float4*>(target)), size_(rows * sizeof(float4)), param_(param), dirty_(dirty)
		{
		}

		void operator()()
		{
			float4x4 v;
			param_->Value(v);

			v = MathLib::transpose(v);
			memcpy(target_, &v[0], size_);
			*dirty_ = true;
		}

	private:
		float4* target_;
		size_t size_;
		RenderEffectParameterPtr param_;
		char* dirty_;
	};

	template <typename SrcType, typename DstType>
	class SetD3D10ShaderParameter<SrcType*, DstType>
	{
	public:
		SetD3D10ShaderParameter(uint8_t* target, uint32_t elements, RenderEffectParameterPtr const & param, char* dirty)
			: target_(reinterpret_cast<DstType*>(target)), elements_(elements), param_(param), dirty_(dirty)
		{
		}

		void operator()()
		{
			std::vector<SrcType> v;
			param_->Value(v);

			for (size_t i = 0; i < v.size(); ++ i)
			{
				target_[i * 4] = static_cast<DstType>(v[i]);
			}
			*dirty_ = true;
		}

	private:
		DstType* target_;
		uint32_t elements_;
		RenderEffectParameterPtr param_;
		char* dirty_;
	};

	template <>
	class SetD3D10ShaderParameter<float4*, float>
	{
	public:
		SetD3D10ShaderParameter(uint8_t* target, uint32_t elements, RenderEffectParameterPtr const & param, char* dirty)
			: target_(reinterpret_cast<float4*>(target)), size_(elements * sizeof(float4)), param_(param), dirty_(dirty)
		{
		}

		void operator()()
		{
			std::vector<float4> v;
			param_->Value(v);

			if (!v.empty())
			{
				memcpy(target_, &v[0], std::min(size_, v.size() * sizeof(float4)));
			}
			*dirty_ = true;
		}

	private:
		float4* target_;
		size_t size_;
		RenderEffectParameterPtr param_;
		char* dirty_;
	};

	template <>
	class SetD3D10ShaderParameter<float4x4*, float>
	{
	public:
		SetD3D10ShaderParameter(uint8_t* target, size_t rows, RenderEffectParameterPtr const & param, char* dirty)
			: target_(reinterpret_cast<float4*>(target)), rows_(rows), param_(param), dirty_(dirty)
		{
		}

		void operator()()
		{
			std::vector<float4x4> v;
			param_->Value(v);


			size_t start = 0;
			BOOST_FOREACH(BOOST_TYPEOF(v)::reference mat, v)
			{
				mat = MathLib::transpose(mat);
				memcpy(&target_[start], &mat[0], rows_ * sizeof(float4));
				start += rows_;
			}
			*dirty_ = true;
		}

	private:
		float4* target_;
		size_t rows_;
		RenderEffectParameterPtr param_;
		char* dirty_;
	};

	template <>
	class SetD3D10ShaderParameter<TexturePtr, TexturePtr>
	{
	public:
		SetD3D10ShaderParameter(TexturePtr& texture, RenderEffectParameterPtr const & param)
			: texture_(&texture), param_(param)
		{
		}

		void operator()()
		{
			param_->Value(*texture_);
		}

	private:
		TexturePtr* texture_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetD3D10ShaderParameter<SamplerStateObjectPtr, SamplerStateObjectPtr>
	{
	public:
		SetD3D10ShaderParameter(SamplerStateObjectPtr& sampler, RenderEffectParameterPtr const & param)
			: sampler_(&sampler), param_(param)
		{
		}

		void operator()()
		{
			param_->Value(*sampler_);
		}

	private:
		SamplerStateObjectPtr* sampler_;
		RenderEffectParameterPtr param_;
	};
}

namespace KlayGE
{
	D3D10ShaderObject::D3D10ShaderObject()
	{
		is_shader_validate_.assign(true);
	}

	std::string D3D10ShaderObject::GenShaderText(RenderEffect const & effect) const
	{
		std::stringstream ss;

		BOOST_AUTO(cbuffers, effect.CBuffers());
		BOOST_FOREACH(BOOST_TYPEOF(cbuffers)::const_reference cbuff, cbuffers)
		{
			ss << "cbuffer " << cbuff.first << std::endl;
			ss << "{" << std::endl;

			BOOST_FOREACH(BOOST_TYPEOF(cbuff.second)::const_reference param_index, cbuff.second)
			{
				RenderEffectParameter& param = *effect.ParameterByIndex(param_index);
				switch (param.type())
				{
				case REDT_texture1D:
				case REDT_texture2D:
				case REDT_texture3D:
				case REDT_textureCUBE:
				case REDT_sampler:
					break;

				default:
					ss << effect.TypeName(param.type()) << " " << *param.Name();
					if (param.ArraySize() != 0)
					{
						ss << "[" << param.ArraySize() << "]";
					}
					ss << ";" << std::endl;
					break;
				}
			}

			ss << "};" << std::endl;
		}

		for (uint32_t i = 0; i < effect.NumParameters(); ++ i)
		{
			RenderEffectParameter& param = *effect.ParameterByIndex(i);

			switch (param.type())
			{
			case REDT_texture1D:
				ss << "Texture1D " << *param.Name() << ";" << std::endl;
				break;

			case REDT_texture2D:
				ss << "Texture2D " << *param.Name() << ";" << std::endl;
				break;

			case REDT_texture3D:
				ss << "Texture3D " << *param.Name() << ";" << std::endl;
				break;

			case REDT_textureCUBE:
				ss << "TextureCube " << *param.Name() << ";" << std::endl;
				break;

			case REDT_sampler:
				ss << "sampler " << *param.Name() << ";" << std::endl;
				break;

			default:
				break;
			}
		}

		for (uint32_t i = 0; i < effect.NumShaders(); ++ i)
		{
			ss << effect.ShaderByIndex(i).str() << std::endl;
		}

		return ss.str();
	}

	void D3D10ShaderObject::SetShader(RenderEffect& effect, ShaderType type, boost::shared_ptr<std::vector<shader_desc> > const & shader_descs,
			boost::shared_ptr<std::string> const & shader_text)
	{
		is_shader_validate_[type] = true;

		D3D10RenderEngine const & render_eng = *checked_cast<D3D10RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D10DevicePtr const & d3d_device = render_eng.D3DDevice();

		std::string shader_profile = (*shader_descs)[type].profile;
		switch (type)
		{
		case ST_VertexShader:
			if ("auto" == shader_profile)
			{
				shader_profile = render_eng.D3D10GetVertexShaderProfile(d3d_device.get());
			}
			break;

		case ST_PixelShader:
			if ("auto" == shader_profile)
			{
				shader_profile = render_eng.D3D10GetPixelShaderProfile(d3d_device.get());
			}
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}

		ID3D10Blob* code;
		ID3D10Blob* err_msg;
		D3D10_SHADER_MACRO macros[] = { { "CONSTANT_BUFFER", "1" }, { "KLAYGE_D3D10", "1" }, { NULL, NULL } };
		D3DX10CompileFromMemory(shader_text->c_str(), static_cast<UINT>(shader_text->size()), NULL, macros,
			NULL, (*shader_descs)[type].func_name.c_str(), shader_profile.c_str(),
			0, 0, NULL, &code, &err_msg, NULL);
		if (err_msg != NULL)
		{
#ifdef KLAYGE_DEBUG
			std::istringstream iss(*shader_text);
			std::string s;
			int line = 1;
			while (iss)
			{
				std::getline(iss, s);
				std::cerr << line << " " << s << std::endl;
				++ line;
			}
			std::cerr << static_cast<char*>(err_msg->GetBufferPointer()) << std::endl;
#endif
			
			err_msg->Release();
		}

		ID3D10BlobPtr code_blob;
		if (NULL == code)
		{
			is_shader_validate_[type] = false;
		}
		else
		{
			code_blob = MakeCOMPtr(code);
			switch (type)
			{
			case ST_VertexShader:
				ID3D10VertexShader* vs;
				if (FAILED(d3d_device->CreateVertexShader(code_blob->GetBufferPointer(), code_blob->GetBufferSize(), &vs)))
				{
					is_shader_validate_[type] = false;
				}
				vertex_shader_ = MakeCOMPtr(vs);
				vs_code_ = code_blob;
				break;

			case ST_PixelShader:
				ID3D10PixelShader* ps;
				if (FAILED(d3d_device->CreatePixelShader(code_blob->GetBufferPointer(), code_blob->GetBufferSize(), &ps)))
				{
					is_shader_validate_[type] = false;
				}
				pixel_shader_ = MakeCOMPtr(ps);
				break;

			default:
				BOOST_ASSERT(false);
				break;
			}

			ID3D10ShaderReflection* reflection;
			render_eng.D3D10ReflectShader(code_blob->GetBufferPointer(), code_blob->GetBufferSize(), &reflection);
			if (reflection != NULL)
			{
				D3D10_SHADER_DESC desc;
				reflection->GetDesc(&desc);

				dirty_[type].resize(desc.ConstantBuffers);
				d3d_cbufs_[type].resize(desc.ConstantBuffers);
				cbufs_[type].resize(desc.ConstantBuffers);
				for (UINT c = 0; c < desc.ConstantBuffers; ++ c)
				{
					ID3D10ShaderReflectionConstantBuffer* reflection_cb = reflection->GetConstantBufferByIndex(c);

					D3D10_SHADER_BUFFER_DESC cb_desc;
					reflection_cb->GetDesc(&cb_desc);
					cbufs_[type][c].resize(cb_desc.Size);

					for (UINT v = 0; v < cb_desc.Variables; ++ v)
					{
						ID3D10ShaderReflectionVariable* reflection_var = reflection_cb->GetVariableByIndex(v);

						D3D10_SHADER_VARIABLE_DESC var_desc;
						reflection_var->GetDesc(&var_desc);
						if (var_desc.uFlags & D3D10_SVF_USED)
						{
							D3D10_SHADER_TYPE_DESC type_desc;
							reflection_var->GetType()->GetDesc(&type_desc);

							D3D10ShaderParameterHandle p_handle;
							p_handle.shader_type = static_cast<uint8_t>(type);
							p_handle.param_class = type_desc.Class;
							p_handle.param_type = type_desc.Type;
							p_handle.cbuff = c;
							p_handle.offset = var_desc.StartOffset;
							p_handle.elements = type_desc.Elements;
							p_handle.rows = static_cast<uint8_t>(type_desc.Rows);
							p_handle.columns = static_cast<uint8_t>(type_desc.Columns);

							RenderEffectParameterPtr const & p = effect.ParameterByName(var_desc.Name);
							if (p)
							{
								param_binds_[type].push_back(this->GetBindFunc(p_handle, p));
							}
						}
					}

					D3D10_BUFFER_DESC desc;
					desc.ByteWidth = cb_desc.Size;
					desc.Usage = D3D10_USAGE_DEFAULT;
					desc.BindFlags = D3D10_BIND_CONSTANT_BUFFER;
					desc.CPUAccessFlags = 0;
					desc.MiscFlags = 0;
					ID3D10Buffer* tmp_buf;
					TIF(d3d_device->CreateBuffer(&desc, NULL, &tmp_buf));
					d3d_cbufs_[type][c] = MakeCOMPtr(tmp_buf);
				}

				int num_textures = -1;
				int num_samplers = -1;
				for (uint32_t i = 0; i < desc.BoundResources; ++ i)
				{
					D3D10_SHADER_INPUT_BIND_DESC si_desc;
					reflection->GetResourceBindingDesc(i, &si_desc);

					switch (si_desc.Type)
					{
					case D3D10_SIT_TEXTURE:
						num_textures = std::max(num_textures, static_cast<int>(si_desc.BindPoint));
						break;

					case D3D10_SIT_SAMPLER:
						num_samplers = std::max(num_samplers, static_cast<int>(si_desc.BindPoint));
						break;
					}
				}

				textures_[type].resize(num_textures + 1);
				samplers_[type].resize(num_samplers + 1);

				for (uint32_t i = 0; i < desc.BoundResources; ++ i)
				{
					D3D10_SHADER_INPUT_BIND_DESC si_desc;
					reflection->GetResourceBindingDesc(i, &si_desc);

					if ((D3D10_SIT_TEXTURE == si_desc.Type) || (D3D10_SIT_SAMPLER == si_desc.Type))
					{
						RenderEffectParameterPtr const & p = effect.ParameterByName(si_desc.Name);
						if (p)
						{
							D3D10ShaderParameterHandle p_handle;
							p_handle.shader_type = static_cast<uint8_t>(type);
							p_handle.param_class = D3D10_SVC_OBJECT;
							p_handle.param_type = (D3D10_SIT_TEXTURE == si_desc.Type) ? D3D10_SVT_TEXTURE : D3D10_SVT_SAMPLER;
							p_handle.offset = si_desc.BindPoint;
							p_handle.elements = 1;
							p_handle.rows = 0;
							p_handle.columns = 1;

							param_binds_[type].push_back(this->GetBindFunc(p_handle, p));
						}
					}
				}

				reflection->Release();
			}
		}

		is_validate_ = true;
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			is_validate_ &= is_shader_validate_[i];
		}
	}

	ShaderObjectPtr D3D10ShaderObject::Clone(RenderEffect& effect)
	{
		D3D10ShaderObjectPtr ret = MakeSharedPtr<D3D10ShaderObject>();
		ret->is_validate_ = is_validate_;
		ret->is_shader_validate_ = is_shader_validate_;
		ret->vertex_shader_ = vertex_shader_;
		ret->pixel_shader_ = pixel_shader_;
		ret->vs_code_ = vs_code_;
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			ret->textures_[i].resize(textures_[i].size());
			ret->samplers_[i].resize(samplers_[i].size());

			ret->cbufs_[i] = cbufs_[i];
			ret->dirty_[i] = dirty_[i];
			ret->d3d_cbufs_[i] = d3d_cbufs_[i];

			ret->param_binds_[i].reserve(param_binds_[i].size());
			BOOST_FOREACH(BOOST_TYPEOF(param_binds_[i])::const_reference pb, param_binds_[i])
			{
				ret->param_binds_[i].push_back(ret->GetBindFunc(pb.p_handle, effect.ParameterByName(*(pb.param->Name()))));
			}
		}

		return ret;
	}

	D3D10ShaderObject::parameter_bind_t D3D10ShaderObject::GetBindFunc(D3D10ShaderParameterHandle const & p_handle, RenderEffectParameterPtr const & param)
	{
		parameter_bind_t ret;
		ret.param = param;
		ret.p_handle = p_handle;

		switch (param->type())
		{
		case REDT_bool:
			if (param->ArraySize() != 0)
			{
				switch (p_handle.param_type)
				{
				case D3D10_SVT_BOOL:
					ret.func = SetD3D10ShaderParameter<bool*, BOOL>(&cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.elements, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
					break;

				case D3D10_SVT_INT:
					ret.func = SetD3D10ShaderParameter<bool*, int>(&cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.elements, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
					break;

				case D3D10_SVT_FLOAT:
					ret.func = SetD3D10ShaderParameter<bool*, float>(&cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.elements, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
					break;

				default:
					BOOST_ASSERT(false);
					break;
				}
			}
			else
			{
				switch (p_handle.param_type)
				{
				case D3D10_SVT_BOOL:
					ret.func = SetD3D10ShaderParameter<bool, BOOL>(&cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
					break;

				case D3D10_SVT_INT:
					ret.func = SetD3D10ShaderParameter<bool, int>(&cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
					break;

				case D3D10_SVT_FLOAT:
					ret.func = SetD3D10ShaderParameter<bool, float>(&cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
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
				switch (p_handle.param_type)
				{
				case D3D10_SVT_BOOL:
					ret.func = SetD3D10ShaderParameter<int*, BOOL>(&cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.elements, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
					break;

				case D3D10_SVT_INT:
					ret.func = SetD3D10ShaderParameter<int*, int>(&cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.elements, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
					break;

				case D3D10_SVT_FLOAT:
					ret.func = SetD3D10ShaderParameter<int*, float>(&cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.elements, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
					break;

				default:
					BOOST_ASSERT(false);
					break;
				}
			}
			else
			{
				switch (p_handle.param_type)
				{
				case D3D10_SVT_BOOL:
					ret.func = SetD3D10ShaderParameter<int, BOOL>(&cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
					break;

				case D3D10_SVT_INT:
					ret.func = SetD3D10ShaderParameter<int, int>(&cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
					break;

				case D3D10_SVT_FLOAT:
					ret.func = SetD3D10ShaderParameter<int, float>(&cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
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
				ret.func = SetD3D10ShaderParameter<float*, float>(&cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.elements, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
			}
			else
			{
				ret.func = SetD3D10ShaderParameter<float, float>(&cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
			}
			break;

		case REDT_float2:
			ret.func = SetD3D10ShaderParameter<float2, float>(&cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
			break;

		case REDT_float3:
			ret.func = SetD3D10ShaderParameter<float3, float>(&cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
			break;

		case REDT_float4:
			if (param->ArraySize() != 0)
			{
				ret.func = SetD3D10ShaderParameter<float4*, float>(&cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.elements, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
			}
			else
			{
				ret.func = SetD3D10ShaderParameter<float4, float>(&cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
			}
			break;

		case REDT_float4x4:
			if (param->ArraySize() != 0)
			{
				ret.func = SetD3D10ShaderParameter<float4x4*, float>(&cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.rows, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
			}
			else
			{
				ret.func = SetD3D10ShaderParameter<float4x4, float>(&cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.rows, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
			}
			break;

		case REDT_texture1D:
		case REDT_texture2D:
		case REDT_texture3D:
		case REDT_textureCUBE:
			ret.func = SetD3D10ShaderParameter<TexturePtr, TexturePtr>(textures_[p_handle.shader_type][p_handle.offset], param);
			break;

		case REDT_sampler:
			ret.func = SetD3D10ShaderParameter<SamplerStateObjectPtr, SamplerStateObjectPtr>(samplers_[p_handle.shader_type][p_handle.offset], param);
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}

		return ret;
	}

	void D3D10ShaderObject::Bind()
	{
		D3D10RenderEngine& re = *checked_cast<D3D10RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D10DevicePtr const & d3d_device = re.D3DDevice();

		re.VSSetShader(vertex_shader_);
		re.PSSetShader(pixel_shader_);

		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			BOOST_FOREACH(BOOST_TYPEOF(param_binds_[i])::reference pb, param_binds_[i])
			{
				pb.func();
			}
		}

		std::vector<ID3D10SamplerState*> sss;
		std::vector<ID3D10ShaderResourceView*> srs;

		BOOST_TYPEOF(textures_)::const_reference vs_textures = textures_[ST_VertexShader];
		srs.resize(vs_textures.size());
		for (size_t i = 0; i < vs_textures.size(); ++ i)
		{
			if (vs_textures[i])
			{
				srs[i] = checked_pointer_cast<D3D10Texture>(vs_textures[i])->D3DShaderResourceView().get();
			}
			else
			{
				srs[i] = NULL;
			}
		}
		if (!srs.empty())
		{
			d3d_device->VSSetShaderResources(0, static_cast<UINT>(srs.size()), &srs[0]);
		}

		BOOST_TYPEOF(samplers_)::const_reference vs_samplers = samplers_[ST_VertexShader];
		sss.resize(vs_samplers.size());
		for (size_t i = 0; i < vs_samplers.size(); ++ i)
		{
			if (vs_samplers[i])
			{
				sss[i] = checked_pointer_cast<D3D10SamplerStateObject>(vs_samplers[i])->D3DSamplerState().get();
			}
			else
			{
				sss[i] = NULL;
			}
		}
		if (!sss.empty())
		{
			d3d_device->VSSetSamplers(0, static_cast<UINT>(sss.size()), &sss[0]);
		}

		BOOST_TYPEOF(textures_)::const_reference ps_textures = textures_[ST_PixelShader];
		srs.resize(ps_textures.size());
		for (size_t i = 0; i < ps_textures.size(); ++ i)
		{
			if (ps_textures[i])
			{
				srs[i] = checked_pointer_cast<D3D10Texture>(ps_textures[i])->D3DShaderResourceView().get();
			}
			else
			{
				srs[i] = NULL;
			}
		}
		if (!srs.empty())
		{
			d3d_device->PSSetShaderResources(0, static_cast<UINT>(srs.size()), &srs[0]);
		}

		BOOST_TYPEOF(samplers_)::const_reference ps_samplers = samplers_[ST_PixelShader];
		sss.resize(ps_samplers.size());
		for (size_t i = 0; i < ps_samplers.size(); ++ i)
		{
			if (ps_samplers[i])
			{
				sss[i] = checked_pointer_cast<D3D10SamplerStateObject>(ps_samplers[i])->D3DSamplerState().get();
			}
			else
			{
				sss[i] = NULL;
			}
		}
		if (!sss.empty())
		{
			d3d_device->PSSetSamplers(0, static_cast<UINT>(sss.size()), &sss[0]);
		}

		for (size_t i = 0; i < d3d_cbufs_.size(); ++ i)
		{
			for (size_t j = 0; j < d3d_cbufs_[i].size(); ++ j)
			{
				if (dirty_[i][j])
				{
					d3d_device->UpdateSubresource(d3d_cbufs_[i][j].get(), D3D10CalcSubresource(0, 0, 1), NULL, &cbufs_[i][j][0],
						static_cast<UINT>(cbufs_[i][j].size()), static_cast<UINT>(cbufs_[i][j].size()));
				}

				dirty_[i][j] = false;
			}
		}

		if (!d3d_cbufs_[ST_VertexShader].empty())
		{
			std::vector<ID3D10Buffer*> cb(d3d_cbufs_[ST_VertexShader].size());
			for (size_t i = 0; i < cb.size(); ++ i)
			{
				cb[i] = d3d_cbufs_[ST_VertexShader][i].get();
			}
			d3d_device->VSSetConstantBuffers(0, static_cast<UINT>(cb.size()), &cb[0]);
		}
		if (!d3d_cbufs_[ST_PixelShader].empty())
		{
			std::vector<ID3D10Buffer*> cb(d3d_cbufs_[ST_PixelShader].size());
			for (size_t i = 0; i < cb.size(); ++ i)
			{
				cb[i] = d3d_cbufs_[ST_PixelShader][i].get();
			}
			d3d_device->PSSetConstantBuffers(0, static_cast<UINT>(cb.size()), &cb[0]);
		}
	}

	void D3D10ShaderObject::Unbind()
	{
		D3D10RenderEngine& re = *checked_cast<D3D10RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D10DevicePtr const & d3d_device = re.D3DDevice();

		std::vector<ID3D10ShaderResourceView*> srs(textures_[ST_VertexShader].size(), NULL);
		if (!srs.empty())
		{
			d3d_device->VSSetShaderResources(0, static_cast<UINT>(srs.size()), &srs[0]);
		}
		srs.resize(textures_[ST_PixelShader].size(), NULL);
		if (!srs.empty())
		{
			d3d_device->PSSetShaderResources(0, static_cast<UINT>(srs.size()), &srs[0]);
		}
	}
}
