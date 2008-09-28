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
#include <algorithm>
#include <sstream>
#include <iostream>
#include <boost/assert.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>

#include <d3d10_1.h>
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
		SetD3D10ShaderParameter(uint8_t* target, RenderEffectParameterPtr const & param)
			: target_(reinterpret_cast<DstType*>(target)), param_(param)
		{
		}

		void operator()()
		{
			SrcType v;
			param_->Value(v);

			*target_ = static_cast<DstType>(v);
		}

	private:
		DstType* target_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetD3D10ShaderParameter<float2, float>
	{
	public:
		SetD3D10ShaderParameter(uint8_t* target, RenderEffectParameterPtr const & param)
			: target_(reinterpret_cast<float2*>(target)), param_(param)
		{
		}

		void operator()()
		{
			float2 v;
			param_->Value(v);

			*target_ = v;
		}

	private:
		float2* target_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetD3D10ShaderParameter<float3, float>
	{
	public:
		SetD3D10ShaderParameter(uint8_t* target, RenderEffectParameterPtr const & param)
			: target_(reinterpret_cast<float3*>(target)), param_(param)
		{
		}

		void operator()()
		{
			float3 v;
			param_->Value(v);

			*target_ = v;
		}

	private:
		float3* target_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetD3D10ShaderParameter<float4, float>
	{
	public:
		SetD3D10ShaderParameter(uint8_t* target, RenderEffectParameterPtr const & param)
			: target_(reinterpret_cast<float4*>(target)), param_(param)
		{
		}

		void operator()()
		{
			float4 v;
			param_->Value(v);

			*target_ = v;
		}

	private:
		float4* target_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetD3D10ShaderParameter<float4x4, float>
	{
	public:
		SetD3D10ShaderParameter(uint8_t* target, uint32_t elements, RenderEffectParameterPtr const & param)
			: target_(reinterpret_cast<float4*>(target)), size_(elements * sizeof(float4)), param_(param)
		{
		}

		void operator()()
		{
			float4x4 v;
			param_->Value(v);

			v = MathLib::transpose(v);
			memcpy(target_, &v[0], size_);
		}

	private:
		float4* target_;
		size_t size_;
		RenderEffectParameterPtr param_;
	};

	template <typename SrcType, typename DstType>
	class SetD3D10ShaderParameter<SrcType*, DstType>
	{
	public:
		SetD3D10ShaderParameter(uint8_t* target, uint32_t elements, RenderEffectParameterPtr const & param)
			: target_(reinterpret_cast<DstType*>(target)), elements_(elements), param_(param)
		{
		}

		void operator()()
		{
			std::vector<SrcType> v;
			param_->Value(v);

			for (size_t i = 0; i < v.size(); ++ i)
			{
				target_[i] = static_cast<DstType>(v[i]);
			}
		}

	private:
		DstType* target_;
		uint32_t elements_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetD3D10ShaderParameter<float4*, float>
	{
	public:
		SetD3D10ShaderParameter(uint8_t* target, uint32_t elements, RenderEffectParameterPtr const & param)
			: target_(reinterpret_cast<float4*>(target)), size_(elements * sizeof(float4)), param_(param)
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
		}

	private:
		float4* target_;
		size_t size_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetD3D10ShaderParameter<float4x4*, float>
	{
	public:
		SetD3D10ShaderParameter(uint8_t* target, size_t rows, RenderEffectParameterPtr const & param)
			: target_(reinterpret_cast<float4*>(target)), rows_(rows), param_(param)
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
		}

	private:
		float4* target_;
		size_t rows_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetD3D10ShaderParameter<std::pair<TexturePtr, SamplerStateObjectPtr>, std::pair<TexturePtr, SamplerStateObjectPtr> >
	{
	public:
		SetD3D10ShaderParameter(std::pair<TexturePtr, SamplerStateObjectPtr>& sampler, RenderEffectParameterPtr const & param)
			: sampler_(&sampler), param_(param)
		{
		}

		void operator()()
		{
			param_->Value(*sampler_);
		}

	private:
		std::pair<TexturePtr, SamplerStateObjectPtr>* sampler_;
		RenderEffectParameterPtr param_;
	};
}

namespace KlayGE
{
	D3D10ShaderObject::D3D10ShaderObject()
	{
		is_shader_validate_.assign(true);
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
				shader_profile = D3D10GetVertexShaderProfile(d3d_device.get());
			}
			break;

		case ST_PixelShader:
			if ("auto" == shader_profile)
			{
				shader_profile = D3D10GetPixelShaderProfile(d3d_device.get());
			}
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}

		ID3D10Blob* code;
		ID3D10Blob* err_msg;
		D3D10_SHADER_MACRO macros[] = { "CONSTANT_BUFFER" };
		D3D10CompileShader(shader_text->c_str(), static_cast<UINT>(shader_text->size()), NULL, macros,
			NULL, (*shader_descs)[type].func_name.c_str(), shader_profile.c_str(),
			D3D10_SHADER_ENABLE_BACKWARDS_COMPATIBILITY, &code, &err_msg);
		if (err_msg != NULL)
		{
#ifdef KLAYGE_DEBUG
			std::cerr << *shader_text << std::endl;
			std::cerr << static_cast<char*>(err_msg->GetBufferPointer()) << std::endl;
#endif
			err_msg->Release();
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
				ID3D10VertexShader* vs;
				if (FAILED(d3d_device->CreateVertexShader(code->GetBufferPointer(), code->GetBufferSize(), &vs)))
				{
					is_shader_validate_[type] = false;
				}
				vertex_shader_ = MakeCOMPtr(vs);
				vs_code_ = MakeCOMPtr(code);
				break;

			case ST_PixelShader:
				ID3D10PixelShader* ps;
				if (FAILED(d3d_device->CreatePixelShader(code->GetBufferPointer(), code->GetBufferSize(), &ps)))
				{
					is_shader_validate_[type] = false;
				}
				pixel_shader_ = MakeCOMPtr(ps);
				code->Release();
				break;

			default:
				BOOST_ASSERT(false);
				code->Release();
				break;
			}
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

		ID3D10ShaderReflection1* reflection;
		D3DX10ReflectShader(code->GetBufferPointer(), code->GetBufferSize(), &reflection);
		if (reflection)
		{
			ID3D10ShaderReflectionConstantBuffer* reflection_cb = reflection->GetConstantBufferByName("global_cb");

			D3D10_SHADER_BUFFER_DESC cb_desc;
			reflection_cb->GetDesc(&cb_desc);
			cbufs_[type].resize(cb_desc.Size);

			for (UINT c = 0; c < cb_desc.Variables; ++ c)
			{
				ID3D10ShaderReflectionVariable* reflection_var = reflection_cb->GetVariableByIndex(c);

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
					p_handle.offset = type_desc.Offset;
					p_handle.rows = static_cast<uint8_t>(type_desc.Rows);
					p_handle.columns = static_cast<uint8_t>(type_desc.Columns);

					RenderEffectParameterPtr const & p = effect.ParameterByName(var_desc.Name);
					if (p != RenderEffectParameter::NullObject())
					{
						param_binds_[type].push_back(this->GetBindFunc(p_handle, p));
					}
				}
			}

			reflection->Release();
		}

		is_validate_ = true;
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			is_validate_ &= is_shader_validate_[i];
		}
	}

	ShaderObjectPtr D3D10ShaderObject::Clone(RenderEffect& effect)
	{
		D3D10ShaderObjectPtr ret(new D3D10ShaderObject);
		ret->is_validate_ = is_validate_;
		ret->is_shader_validate_ = is_shader_validate_;
		ret->vertex_shader_ = vertex_shader_;
		ret->pixel_shader_ = pixel_shader_;
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			ret->cbufs_[i].resize(cbufs_[i].size());
			ret->samplers_[i].resize(samplers_[i].size());

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
					ret.func = SetD3D10ShaderParameter<bool*, BOOL>(&cbufs_[p_handle.shader_type][p_handle.offset], p_handle.elements, param);
					break;

				case D3D10_SVT_INT:
					ret.func = SetD3D10ShaderParameter<bool*, int>(&cbufs_[p_handle.shader_type][p_handle.offset], p_handle.elements, param);
					break;

				case D3D10_SVT_FLOAT:
					ret.func = SetD3D10ShaderParameter<bool*, float>(&cbufs_[p_handle.shader_type][p_handle.offset], p_handle.elements, param);
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
					ret.func = SetD3D10ShaderParameter<bool, BOOL>(&cbufs_[p_handle.shader_type][p_handle.offset], param);
					break;

				case D3D10_SVT_INT:
					ret.func = SetD3D10ShaderParameter<bool, int>(&cbufs_[p_handle.shader_type][p_handle.offset], param);
					break;

				case D3D10_SVT_FLOAT:
					ret.func = SetD3D10ShaderParameter<bool, float>(&cbufs_[p_handle.shader_type][p_handle.offset], param);
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
					ret.func = SetD3D10ShaderParameter<int*, BOOL>(&cbufs_[p_handle.shader_type][p_handle.offset], p_handle.elements, param);
					break;

				case D3D10_SVT_INT:
					ret.func = SetD3D10ShaderParameter<int*, int>(&cbufs_[p_handle.shader_type][p_handle.offset], p_handle.elements, param);
					break;

				case D3D10_SVT_FLOAT:
					ret.func = SetD3D10ShaderParameter<int*, float>(&cbufs_[p_handle.shader_type][p_handle.offset], p_handle.elements, param);
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
					ret.func = SetD3D10ShaderParameter<int, BOOL>(&cbufs_[p_handle.shader_type][p_handle.offset], param);
					break;

				case D3D10_SVT_INT:
					ret.func = SetD3D10ShaderParameter<int, int>(&cbufs_[p_handle.shader_type][p_handle.offset], param);
					break;

				case D3D10_SVT_FLOAT:
					ret.func = SetD3D10ShaderParameter<int, float>(&cbufs_[p_handle.shader_type][p_handle.offset], param);
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
				ret.func = SetD3D10ShaderParameter<float*, float>(&cbufs_[p_handle.shader_type][p_handle.offset], p_handle.elements, param);
			}
			else
			{
				ret.func = SetD3D10ShaderParameter<float, float>(&cbufs_[p_handle.shader_type][p_handle.offset], param);
			}
			break;

		case REDT_float2:
			ret.func = SetD3D10ShaderParameter<float2, float>(&cbufs_[p_handle.shader_type][p_handle.offset], param);
			break;

		case REDT_float3:
			ret.func = SetD3D10ShaderParameter<float3, float>(&cbufs_[p_handle.shader_type][p_handle.offset], param);
			break;

		case REDT_float4:
			if (param->ArraySize() != 0)
			{
				ret.func = SetD3D10ShaderParameter<float4*, float>(&cbufs_[p_handle.shader_type][p_handle.offset], p_handle.elements, param);
			}
			else
			{
				ret.func = SetD3D10ShaderParameter<float4, float>(&cbufs_[p_handle.shader_type][p_handle.offset], param);
			}
			break;

		case REDT_float4x4:
			if (param->ArraySize() != 0)
			{
				ret.func = SetD3D10ShaderParameter<float4x4*, float>(&cbufs_[p_handle.shader_type][p_handle.offset], p_handle.elements, param);
			}
			else
			{
				ret.func = SetD3D10ShaderParameter<float4x4, float>(&cbufs_[p_handle.shader_type][p_handle.offset], p_handle.rows, param);
			}
			break;

		case REDT_sampler1D:
		case REDT_sampler2D:
		case REDT_sampler3D:
		case REDT_samplerCUBE:
			ret.func = SetD3D10ShaderParameter<std::pair<TexturePtr, SamplerStateObjectPtr>, std::pair<TexturePtr, SamplerStateObjectPtr> >(samplers_[p_handle.shader_type][p_handle.offset], param);
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}

		return ret;
	}

	void D3D10ShaderObject::Active()
	{
		D3D10RenderEngine& re = *checked_cast<D3D10RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D10DevicePtr const & d3d_device = re.D3DDevice();

		re.VSSetShader(vertex_shader_);
		re.PSSetShader(pixel_shader_);

		std::vector<ID3D10SamplerState*> sss;
		std::vector<ID3D10ShaderResourceView*> srs;

		sss.resize(samplers_[ST_VertexShader].size());
		srs.resize(samplers_[ST_VertexShader].size());
		for (size_t i = 0; i < samplers_[ST_VertexShader].size(); ++ i)
		{
			sss[i] = checked_pointer_cast<D3D10SamplerStateObject>(samplers_[ST_VertexShader][i].second)->D3DSamplerState().get();
			srs[i] = checked_pointer_cast<D3D10Texture>(samplers_[ST_VertexShader][i].first)->D3DShaderResourceView().get();
		}
		d3d_device->VSSetSamplers(0, samplers_[ST_VertexShader].size(), &sss[0]);
		d3d_device->VSSetShaderResources(0, samplers_[ST_VertexShader].size(), &srs[0]);

		sss.resize(samplers_[ST_PixelShader].size());
		srs.resize(samplers_[ST_PixelShader].size());
		for (size_t i = 0; i < samplers_[ST_PixelShader].size(); ++ i)
		{
			sss[i] = checked_pointer_cast<D3D10SamplerStateObject>(samplers_[ST_PixelShader][i].second)->D3DSamplerState().get();
			srs[i] = checked_pointer_cast<D3D10Texture>(samplers_[ST_PixelShader][i].first)->D3DShaderResourceView().get();
		}
		d3d_device->PSSetSamplers(0, samplers_[ST_PixelShader].size(), &sss[0]);
		d3d_device->PSSetShaderResources(0, samplers_[ST_PixelShader].size(), &srs[0]);
	}
}
