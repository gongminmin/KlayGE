// D3D11ShaderObject.cpp
// KlayGE D3D11 shader对象类 实现文件
// Ver 3.9.0
// 版权所有(C) 龚敏敏, 2009
// Homepage: http://www.klayge.org
//
// 3.9.0
// 加速Shader编译 (2009.7.31)
//
// 3.8.0
// 支持Gemoetry Shader (2009.2.5)
// 初次建立 (2009.1.30)
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
#include <boost/assert.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 6011 6334)
#endif
#include <boost/functional/hash.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

#include <KlayGE/D3D11/D3D11MinGWDefs.hpp>
#include <D3DCompiler.h>

#include <KlayGE/D3D11/D3D11RenderEngine.hpp>
#include <KlayGE/D3D11/D3D11RenderStateObject.hpp>
#include <KlayGE/D3D11/D3D11Mapping.hpp>
#include <KlayGE/D3D11/D3D11Texture.hpp>
#include <KlayGE/D3D11/D3D11ShaderObject.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#pragma comment(lib, "d3dcompiler.lib")
#endif

namespace
{
	using namespace KlayGE;

	template <typename SrcType, typename DstType>
	class SetD3D11ShaderParameter
	{
	public:
		SetD3D11ShaderParameter(uint8_t* target, RenderEffectParameterPtr const & param, char* dirty)
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

	template <typename T, int N>
	class SetD3D11ShaderParameter<Vector_T<T, N>, T>
	{
	public:
		SetD3D11ShaderParameter(uint8_t* target, RenderEffectParameterPtr const & param, char* dirty)
			: target_(reinterpret_cast<Vector_T<T, N>*>(target)), param_(param), dirty_(dirty)
		{
		}

		void operator()()
		{
			Vector_T<T, N> v;
			param_->Value(v);

			if (*target_ != v)
			{
				*target_ = v;
				*dirty_ = true;
			}
		}

	private:
		Vector_T<T, N>* target_;
		RenderEffectParameterPtr param_;
		char* dirty_;
	};

	template <>
	class SetD3D11ShaderParameter<float4x4, float>
	{
	public:
		SetD3D11ShaderParameter(uint8_t* target, uint32_t rows, RenderEffectParameterPtr const & param, char* dirty)
			: target_(reinterpret_cast<float4*>(target)), size_(rows * sizeof(float4)), param_(param), dirty_(dirty)
		{
		}

		void operator()()
		{
			float4x4 v;
			param_->Value(v);

			v = MathLib::transpose(v);
			if (memcmp(target_, &v[0], size_))
			{
				memcpy(target_, &v[0], size_);
				*dirty_ = true;
			}
		}

	private:
		float4* target_;
		size_t size_;
		RenderEffectParameterPtr param_;
		char* dirty_;
	};

	template <typename SrcType, typename DstType>
	class SetD3D11ShaderParameter<SrcType*, DstType>
	{
	public:
		SetD3D11ShaderParameter(uint8_t* target, uint32_t elements, RenderEffectParameterPtr const & param, char* dirty)
			: target_(reinterpret_cast<DstType*>(target)), elements_(elements), param_(param), dirty_(dirty)
		{
		}

		void operator()()
		{
			std::vector<SrcType> v;
			param_->Value(v);

			for (size_t i = 0; i < v.size(); ++ i)
			{
				if (target_[i * 4] != static_cast<DstType>(v[i]))
				{
					target_[i * 4] = static_cast<DstType>(v[i]);
					*dirty_ = true;
				}
			}
		}

	private:
		DstType* target_;
		uint32_t elements_;
		RenderEffectParameterPtr param_;
		char* dirty_;
	};

	template <typename T>
	class SetD3D11ShaderParameter<Vector_T<T, 2>*, T>
	{
	public:
		SetD3D11ShaderParameter(uint8_t* target, uint32_t elements, RenderEffectParameterPtr const & param, char* dirty)
			: target_(reinterpret_cast<Vector_T<T, 4>*>(target)), size_(elements * sizeof(int4)), param_(param), dirty_(dirty)
		{
		}

		void operator()()
		{
			std::vector<Vector_T<T, 2> > v;
			param_->Value(v);

			if (!v.empty())
			{
				std::vector<Vector_T<T, 4> > v4(v.size());
				for (size_t i = 0; i < v.size(); ++ i)
				{
					v4[i] = Vector_T<T, 4>(v[i].x(), v[i].y(), 0, 0);
				}
				memcpy(target_, &v4[0], std::min(size_, v4.size() * sizeof(v4[0])));
			}
			*dirty_ = true;
		}

	private:
		Vector_T<T, 4>* target_;
		size_t size_;
		RenderEffectParameterPtr param_;
		char* dirty_;
	};

	template <typename T>
	class SetD3D11ShaderParameter<Vector_T<T, 3>*, T>
	{
	public:
		SetD3D11ShaderParameter(uint8_t* target, uint32_t elements, RenderEffectParameterPtr const & param, char* dirty)
			: target_(reinterpret_cast<Vector_T<T, 4>*>(target)), size_(elements * sizeof(int4)), param_(param), dirty_(dirty)
		{
		}

		void operator()()
		{
			std::vector<Vector_T<T, 3> > v;
			param_->Value(v);

			if (!v.empty())
			{
				std::vector<Vector_T<T, 4> > v4(v.size());
				for (size_t i = 0; i < v.size(); ++ i)
				{
					v4[i] = Vector_T<T, 4>(v[i].x(), v[i].y(), v[i].z(), 0);
				}
				memcpy(target_, &v4[0], std::min(size_, v4.size() * sizeof(v4[0])));
			}
			*dirty_ = true;
		}

	private:
		Vector_T<T, 4>* target_;
		size_t size_;
		RenderEffectParameterPtr param_;
		char* dirty_;
	};

	template <typename T>
	class SetD3D11ShaderParameter<Vector_T<T, 4>*, T>
	{
	public:
		SetD3D11ShaderParameter(uint8_t* target, uint32_t elements, RenderEffectParameterPtr const & param, char* dirty)
			: target_(reinterpret_cast<Vector_T<T, 4>*>(target)), size_(elements * sizeof(int4)), param_(param), dirty_(dirty)
		{
		}

		void operator()()
		{
			std::vector<Vector_T<T, 4> > v;
			param_->Value(v);

			if (!v.empty())
			{
				memcpy(target_, &v[0], std::min(size_, v.size() * sizeof(v[0])));
			}
			*dirty_ = true;
		}

	private:
		Vector_T<T, 4>* target_;
		size_t size_;
		RenderEffectParameterPtr param_;
		char* dirty_;
	};

	template <>
	class SetD3D11ShaderParameter<float4x4*, float>
	{
	public:
		SetD3D11ShaderParameter(uint8_t* target, size_t rows, RenderEffectParameterPtr const & param, char* dirty)
			: target_(reinterpret_cast<float4*>(target)), rows_(rows), param_(param), dirty_(dirty)
		{
		}

		void operator()()
		{
			std::vector<float4x4> v;
			param_->Value(v);

			size_t start = 0;
			typedef BOOST_TYPEOF(v) VType;
			BOOST_FOREACH(VType::reference mat, v)
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
	class SetD3D11ShaderParameter<SamplerStateObjectPtr, ID3D11SamplerStatePtr>
	{
	public:
		SetD3D11ShaderParameter(ID3D11SamplerStatePtr& sampler, RenderEffectParameterPtr const & param)
			: sampler_(&sampler), param_(param)
		{
		}

		void operator()()
		{
			SamplerStateObjectPtr sampler;
			param_->Value(sampler);
			if (sampler)
			{
				*sampler_ = checked_cast<D3D11SamplerStateObject*>(sampler.get())->D3DSamplerState();
			}
		}

	private:
		ID3D11SamplerStatePtr* sampler_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetD3D11ShaderParameter<TexturePtr, ID3D11ShaderResourceViewPtr>
	{
	public:
		SetD3D11ShaderParameter(boost::tuple<void*, uint32_t, uint32_t>& srvsrc, ID3D11ShaderResourceViewPtr& srv, RenderEffectParameterPtr const & param)
			: srvsrc_(&srvsrc), srv_(&srv), param_(param)
		{
		}

		void operator()()
		{
			boost::tuple<TexturePtr, uint32_t, uint32_t, uint32_t, uint32_t> tex_tuple;
			param_->Value(tex_tuple);
			if (tex_tuple.get<0>())
			{
				*srvsrc_ = boost::make_tuple(tex_tuple.get<0>().get(), tex_tuple.get<1>() * tex_tuple.get<0>()->NumMipMaps() + tex_tuple.get<3>(),
					tex_tuple.get<2>() * tex_tuple.get<4>());
				*srv_ = checked_cast<D3D11Texture*>(tex_tuple.get<0>().get())->RetriveD3DShaderResourceView(tex_tuple.get<1>(), tex_tuple.get<2>(), tex_tuple.get<3>(), tex_tuple.get<4>());
			}
			else
			{
				*srvsrc_ = NULL;
			}
		}

	private:
		boost::tuple<void*, uint32_t, uint32_t>* srvsrc_;
		ID3D11ShaderResourceViewPtr* srv_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetD3D11ShaderParameter<GraphicsBufferPtr, ID3D11ShaderResourceViewPtr>
	{
	public:
		SetD3D11ShaderParameter(boost::tuple<void*, uint32_t, uint32_t>& srvsrc, ID3D11ShaderResourceViewPtr& srv, RenderEffectParameterPtr const & param)
			: srvsrc_(&srvsrc), srv_(&srv), param_(param)
		{
		}

		void operator()()
		{
			GraphicsBufferPtr buf;
			param_->Value(buf);
			if (buf)
			{
				*srvsrc_ = boost::make_tuple(buf.get(), 0, 1);
				*srv_ = checked_cast<D3D11GraphicsBuffer*>(buf.get())->D3DShaderResourceView();
			}
			else
			{
				*srvsrc_ = NULL;
			}
		}

	private:
		boost::tuple<void*, uint32_t, uint32_t>* srvsrc_;
		ID3D11ShaderResourceViewPtr* srv_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetD3D11ShaderParameter<TexturePtr, ID3D11UnorderedAccessViewPtr>
	{
	public:
		SetD3D11ShaderParameter(void*& uavsrc, ID3D11UnorderedAccessViewPtr& uav, RenderEffectParameterPtr const & param)
			: uavsrc_(&uavsrc), uav_(&uav), param_(param)
		{
		}

		void operator()()
		{
			boost::tuple<TexturePtr, uint32_t, uint32_t, uint32_t, uint32_t> tex_tuple;
			param_->Value(tex_tuple);
			if (tex_tuple.get<0>())
			{
				*uavsrc_ = tex_tuple.get<0>().get();
				*uav_ = checked_cast<D3D11Texture*>(tex_tuple.get<0>().get())->RetriveD3DUnorderedAccessView(tex_tuple.get<1>(), tex_tuple.get<2>(), tex_tuple.get<3>());
			}
			else
			{
				*uavsrc_ = NULL;
			}
		}

	private:
		void** uavsrc_;
		ID3D11UnorderedAccessViewPtr* uav_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetD3D11ShaderParameter<GraphicsBufferPtr, ID3D11UnorderedAccessViewPtr>
	{
	public:
		SetD3D11ShaderParameter(void*& uavsrc, ID3D11UnorderedAccessViewPtr& uav, RenderEffectParameterPtr const & param)
			: uavsrc_(&uavsrc), uav_(&uav), param_(param)
		{
		}

		void operator()()
		{
			GraphicsBufferPtr buf;
			param_->Value(buf);
			if (buf)
			{
				*uavsrc_ = buf.get();
				*uav_ = checked_cast<D3D11GraphicsBuffer*>(buf.get())->D3DUnorderedAccessView();
			}
			else
			{
				*uavsrc_ = NULL;
			}
		}

	private:
		void** uavsrc_;
		ID3D11UnorderedAccessViewPtr* uav_;
		RenderEffectParameterPtr param_;
	};
}

namespace KlayGE
{
	D3D11ShaderObject::D3D11ShaderObject()
	{
		has_discard_ = true;
		has_tessellation_ = false;
		is_shader_validate_.assign(true);
	}

	std::string D3D11ShaderObject::GenShaderText(RenderEffect const & effect, ShaderType cur_type) const
	{
		std::stringstream ss;

		for (uint32_t i = 0; i < effect.NumMacros(); ++ i)
		{
			std::pair<std::string, std::string> const & name_value = effect.MacroByIndex(i);
			ss << "#define " << name_value.first << " " << name_value.second << std::endl;
		}
		ss << std::endl;

		BOOST_AUTO(cbuffers, effect.CBuffers());
		typedef BOOST_TYPEOF(cbuffers) CBuffersType;
		BOOST_FOREACH(CBuffersType::const_reference cbuff, cbuffers)
		{
			ss << "cbuffer " << cbuff.first << std::endl;
			ss << "{" << std::endl;

			typedef BOOST_TYPEOF(cbuff.second) CBuffersSecondType;
			BOOST_FOREACH(CBuffersSecondType::const_reference param_index, cbuff.second)
			{
				RenderEffectParameter& param = *effect.ParameterByIndex(param_index);
				switch (param.type())
				{
				case REDT_texture1D:
				case REDT_texture2D:
				case REDT_texture3D:
				case REDT_textureCUBE:
				case REDT_texture1DArray:
				case REDT_texture2DArray:
				case REDT_texture3DArray:
				case REDT_textureCUBEArray:
				case REDT_sampler:
				case REDT_buffer:
				case REDT_structured_buffer:
				case REDT_byte_address_buffer:
				case REDT_rw_buffer:
				case REDT_rw_structured_buffer:
				case REDT_rw_texture1D:
				case REDT_rw_texture2D:
				case REDT_rw_texture3D:
				case REDT_rw_texture1DArray:
				case REDT_rw_texture2DArray:
				case REDT_rw_byte_address_buffer:
				case REDT_append_structured_buffer:
				case REDT_consume_structured_buffer:
					break;

				default:
					ss << effect.TypeName(param.type()) << " " << *param.Name();
					if (param.ArraySize())
					{
						ss << "[" << *param.ArraySize() << "]";
					}
					ss << ";" << std::endl;
					break;
				}
			}

			ss << "};" << std::endl;
		}

		RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
		for (uint32_t i = 0; i < effect.NumParameters(); ++ i)
		{
			RenderEffectParameter& param = *effect.ParameterByIndex(i);

			switch (param.type())
			{
			case REDT_texture1D:
				{
					std::string elem_type;
					param.var()->Value(elem_type);
					ss << "Texture1D<" << elem_type << "> " << *param.Name() << ";" << std::endl;
				}
				break;

			case REDT_texture2D:
				{
					std::string elem_type;
					param.var()->Value(elem_type);
					ss << "Texture2D<" << elem_type << "> " << *param.Name() << ";" << std::endl;
				}
				break;

			case REDT_texture3D:
				{
					std::string elem_type;
					param.var()->Value(elem_type);
					ss << "Texture3D<" << elem_type << "> " << *param.Name() << ";" << std::endl;
				}
				break;

			case REDT_textureCUBE:
				{
					std::string elem_type;
					param.var()->Value(elem_type);
					ss << "TextureCube<" << elem_type << "> " << *param.Name() << ";" << std::endl;
				}
				break;

			case REDT_texture1DArray:
				if (caps.max_shader_model >= 4)
				{
					std::string elem_type;
					param.var()->Value(elem_type);
					ss << "Texture1DArray<" << elem_type << "> " << *param.Name() << ";" << std::endl;
				}
				break;

			case REDT_texture2DArray:
				if (caps.max_shader_model >= 4)
				{
					std::string elem_type;
					param.var()->Value(elem_type);
					ss << "Texture2DArray<" << elem_type << "> " << *param.Name() << ";" << std::endl;
				}
				break;

			case REDT_textureCUBEArray:
				if (caps.max_shader_model >= 4)
				{
					std::string elem_type;
					param.var()->Value(elem_type);
					ss << "TextureCubeArray<" << elem_type << "> " << *param.Name() << ";" << std::endl;
				}
				break;

			case REDT_buffer:
				if (caps.max_shader_model >= 4)
				{
					std::string elem_type;
					param.var()->Value(elem_type);
					ss << "Buffer<" << elem_type << "> " << *param.Name() << ";" << std::endl;
				}
				break;

			case REDT_sampler:
				ss << "sampler " << *param.Name() << ";" << std::endl;
				break;

			case REDT_structured_buffer:
				if (caps.max_shader_model >= 4)
				{
					std::string elem_type;
					param.var()->Value(elem_type);
					ss << "StructuredBuffer<" << elem_type << "> " << *param.Name() << ";" << std::endl;
				}
				break;

			case REDT_byte_address_buffer:
				if (caps.max_shader_model >= 4)
				{
					ss << "ByteAddressBuffer " << *param.Name() << ";" << std::endl;
				}
				break;

			case REDT_rw_buffer:
				if (caps.max_shader_model >= 5)
				{
					std::string elem_type;
					param.var()->Value(elem_type);
					ss << "RWBuffer<" << elem_type << "> " << *param.Name() << ";" << std::endl;
				}
				break;

			case REDT_rw_structured_buffer:
				if (caps.max_shader_model >= 4)
				{
					std::string elem_type;
					param.var()->Value(elem_type);
					ss << "RWStructuredBuffer<" << elem_type << "> " << *param.Name() << ";" << std::endl;
				}
				break;

			case REDT_rw_texture1D:
				if (caps.max_shader_model >= 5)
				{
					std::string elem_type;
					param.var()->Value(elem_type);
					ss << "RWTexture1D<" << elem_type << "> " << *param.Name() << ";" << std::endl;
				}
				break;

			case REDT_rw_texture2D:
				if (caps.max_shader_model >= 5)
				{
					std::string elem_type;
					param.var()->Value(elem_type);
					ss << "RWTexture2D<" << elem_type << "> " << *param.Name() << ";" << std::endl;
				}
				break;

			case REDT_rw_texture3D:
				if (caps.max_shader_model >= 5)
				{
					std::string elem_type;
					param.var()->Value(elem_type);
					ss << "RWTexture3D<" << elem_type << "> " << *param.Name() << ";" << std::endl;
				}
				break;
			case REDT_rw_texture1DArray:
				if (caps.max_shader_model >= 5)
				{
					std::string elem_type;
					param.var()->Value(elem_type);
					ss << "RWTexture1DArray<" << elem_type << "> " << *param.Name() << ";" << std::endl;
				}
				break;

			case REDT_rw_texture2DArray:
				if (caps.max_shader_model >= 5)
				{
					std::string elem_type;
					param.var()->Value(elem_type);
					ss << "RWTexture2DArray<" << elem_type << "> " << *param.Name() << ";" << std::endl;
				}
				break;

			case REDT_rw_byte_address_buffer:
				if (caps.max_shader_model >= 4)
				{
					ss << "RWByteAddressBuffer " << *param.Name() << ";" << std::endl;
				}
				break;

			case REDT_append_structured_buffer:
				if (caps.max_shader_model >= 5)
				{
					std::string elem_type;
					param.var()->Value(elem_type);
					ss << "AppendStructuredBuffer<" << elem_type << "> " << *param.Name() << ";" << std::endl;
				}
				break;

			case REDT_consume_structured_buffer:
				if (caps.max_shader_model >= 5)
				{
					std::string elem_type;
					param.var()->Value(elem_type);
					ss << "ConsumeStructuredBuffer<" << elem_type << "> " << *param.Name() << ";" << std::endl;
				}
				break;

			default:
				break;
			}
		}

		for (uint32_t i = 0; i < effect.NumShaders(); ++ i)
		{
			RenderShaderFunc const & effect_shader = effect.ShaderByIndex(i);
			ShaderType shader_type = effect_shader.Type();
			if ((ST_NumShaderTypes == shader_type) || (cur_type == shader_type))
			{
				if (caps.max_shader_model >= effect_shader.Version())
				{
					ss << effect_shader.str() << std::endl;
				}
			}
		}

		return ss.str();
	}

	std::string D3D11ShaderObject::GetShaderProfile(ShaderType type, RenderEffect const & effect, uint32_t shader_desc_id)
	{
		shader_desc const & sd = effect.GetShaderDesc(shader_desc_id);
		D3D11RenderEngine const & render_eng = *checked_cast<D3D11RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		RenderDeviceCaps const & caps = render_eng.DeviceCaps();
		std::string shader_profile = sd.profile;
		switch (type)
		{
		case ST_VertexShader:
			if ("auto" == shader_profile)
			{
				shader_profile = render_eng.VertexShaderProfile();
			}
			break;

		case ST_PixelShader:
			if ("auto" == shader_profile)
			{
				shader_profile = render_eng.PixelShaderProfile();
			}
			break;

		case ST_GeometryShader:
			if (caps.max_shader_model >= 4)
			{
				if ("auto" == shader_profile)
				{
					shader_profile = render_eng.GeometryShaderProfile();
				}
			}
			break;

		case ST_ComputeShader:
			if (caps.max_shader_model >= 4)
			{
				if ("auto" == shader_profile)
				{
					shader_profile = render_eng.ComputeShaderProfile();
				}
			}
			break;

		case ST_HullShader:
			if (caps.max_shader_model >= 5)
			{
				if ("auto" == shader_profile)
				{
					shader_profile = render_eng.HullShaderProfile();
				}
			}
			break;

		case ST_DomainShader:
			if (caps.max_shader_model >= 5)
			{
				if ("auto" == shader_profile)
				{
					shader_profile = render_eng.DomainShaderProfile();
				}
			}
			break;

		default:
			break;
		}

		return shader_profile;
	}

	bool D3D11ShaderObject::AttachNativeShader(ShaderType type, RenderEffect const & effect, std::vector<uint32_t> const & shader_desc_ids,
		std::vector<uint8_t> const & native_shader_block)
	{
		bool ret = false;

		std::string shader_profile = this->GetShaderProfile(type, effect, shader_desc_ids[type]);
		if (native_shader_block.size() >= 25 + shader_profile.size())
		{
			uint8_t const * nsbp = &native_shader_block[0];

			uint32_t fourcc;
			memcpy(&fourcc, nsbp, sizeof(fourcc));
			nsbp += sizeof(fourcc);
			LittleEndianToNative<sizeof(fourcc)>(&fourcc);
			if (MakeFourCC<'D', 'X', 'B', 'C'>::value == fourcc)
			{
				uint32_t ver;
				memcpy(&ver, nsbp, sizeof(ver));
				nsbp += sizeof(ver);
				LittleEndianToNative<sizeof(ver)>(&ver);
				if (2 == ver)
				{
					uint64_t timestamp;
					memcpy(&timestamp, nsbp, sizeof(timestamp));
					nsbp += sizeof(timestamp);
					LittleEndianToNative<sizeof(timestamp)>(&timestamp);
					if (timestamp >= effect.Timestamp())
					{
						uint64_t hash_val;
						memcpy(&hash_val, nsbp, sizeof(hash_val));
						nsbp += sizeof(hash_val);
						LittleEndianToNative<sizeof(hash_val)>(&hash_val);
						if (effect.PredefinedMacrosHash() == hash_val)
						{
							uint8_t len = *nsbp;
							++ nsbp;
							std::string profile;
							profile.resize(len);
							memcpy(&profile[0], nsbp, len);
							nsbp += len;
							if (profile == shader_profile)
							{
								uint32_t blob_size;
								memcpy(&blob_size, nsbp, sizeof(blob_size));
								nsbp += sizeof(blob_size);
								ID3DBlob* code;
								D3DCreateBlob(blob_size, &code);
								ID3DBlobPtr code_blob = MakeCOMPtr(code);

								memcpy(code_blob->GetBufferPointer(), nsbp, blob_size);
								nsbp += blob_size;

								D3D11ShaderDesc& sd = shader_desc_[type];

								uint16_t cb_desc_size;
								memcpy(&cb_desc_size, nsbp, sizeof(cb_desc_size));
								nsbp += sizeof(cb_desc_size);
								LittleEndianToNative<sizeof(cb_desc_size)>(&cb_desc_size);
								sd.cb_desc.resize(cb_desc_size);
								for (size_t i = 0; i < sd.cb_desc.size(); ++ i)
								{
									memcpy(&sd.cb_desc[i].size, nsbp, sizeof(sd.cb_desc[i].size));
									nsbp += sizeof(sd.cb_desc[i].size);
									LittleEndianToNative<sizeof(sd.cb_desc[i].size)>(&sd.cb_desc[i].size);

									uint16_t var_desc_size;
									memcpy(&var_desc_size, nsbp, sizeof(var_desc_size));
									nsbp += sizeof(var_desc_size);
									LittleEndianToNative<sizeof(var_desc_size)>(&var_desc_size);
									sd.cb_desc[i].var_desc.resize(var_desc_size);
									for (size_t j = 0; j < sd.cb_desc[i].var_desc.size(); ++ j)
									{
										len = *nsbp;
										++ nsbp;
										sd.cb_desc[i].var_desc[j].name.resize(len);
										memcpy(&sd.cb_desc[i].var_desc[j].name[0], nsbp, len);
										nsbp += len;

										memcpy(&sd.cb_desc[i].var_desc[j].start_offset, nsbp, sizeof(sd.cb_desc[i].var_desc[j].start_offset));
										nsbp += sizeof(sd.cb_desc[i].var_desc[j].start_offset);
										LittleEndianToNative<sizeof(sd.cb_desc[i].var_desc[j].start_offset)>(&sd.cb_desc[i].var_desc[j].start_offset);
										memcpy(&sd.cb_desc[i].var_desc[j].type, nsbp, sizeof(sd.cb_desc[i].var_desc[j].type));
										nsbp += sizeof(sd.cb_desc[i].var_desc[j].type);
										memcpy(&sd.cb_desc[i].var_desc[j].rows, nsbp, sizeof(sd.cb_desc[i].var_desc[j].rows));
										nsbp += sizeof(sd.cb_desc[i].var_desc[j].rows);
										memcpy(&sd.cb_desc[i].var_desc[j].columns, nsbp, sizeof(sd.cb_desc[i].var_desc[j].columns));
										nsbp += sizeof(sd.cb_desc[i].var_desc[j].columns);
										memcpy(&sd.cb_desc[i].var_desc[j].elements, nsbp, sizeof(sd.cb_desc[i].var_desc[j].elements));
										nsbp += sizeof(sd.cb_desc[i].var_desc[j].elements);
										LittleEndianToNative<sizeof(sd.cb_desc[i].var_desc[j].elements)>(&sd.cb_desc[i].var_desc[j].elements);
									}
								}

								memcpy(&sd.num_samplers, nsbp, sizeof(sd.num_samplers));
								nsbp += sizeof(sd.num_samplers);
								LittleEndianToNative<sizeof(sd.num_samplers)>(&sd.num_samplers);
								memcpy(&sd.num_srvs, nsbp, sizeof(sd.num_srvs));
								nsbp += sizeof(sd.num_srvs);
								LittleEndianToNative<sizeof(sd.num_srvs)>(&sd.num_srvs);
								memcpy(&sd.num_uavs, nsbp, sizeof(sd.num_uavs));
								nsbp += sizeof(sd.num_uavs);
								LittleEndianToNative<sizeof(sd.num_uavs)>(&sd.num_uavs);

								uint16_t res_desc_size;
								memcpy(&res_desc_size, nsbp, sizeof(res_desc_size));
								nsbp += sizeof(res_desc_size);
								LittleEndianToNative<sizeof(res_desc_size)>(&res_desc_size);
								sd.res_desc.resize(res_desc_size);
								for (size_t i = 0; i < sd.res_desc.size(); ++ i)
								{
									len = *nsbp;
									++ nsbp;
									sd.res_desc[i].name.resize(len);
									memcpy(&sd.res_desc[i].name[0], nsbp, len);
									nsbp += len;

									memcpy(&sd.res_desc[i].type, nsbp, sizeof(sd.res_desc[i].type));
									nsbp += sizeof(sd.res_desc[i].type);

									memcpy(&sd.res_desc[i].dimension, nsbp, sizeof(sd.res_desc[i].dimension));
									nsbp += sizeof(sd.res_desc[i].dimension);

									memcpy(&sd.res_desc[i].bind_point, nsbp, sizeof(sd.res_desc[i].bind_point));
									nsbp += sizeof(sd.res_desc[i].bind_point);
									LittleEndianToNative<sizeof(sd.res_desc[i].bind_point)>(&sd.res_desc[i].bind_point);
								}

								if (ST_VertexShader == type)
								{
									memcpy(&vs_signature_, nsbp, sizeof(vs_signature_));
									nsbp += sizeof(vs_signature_);
									LittleEndianToNative<sizeof(vs_signature_)>(&vs_signature_);
								}

								this->AttachShaderBytecode(type, effect, shader_desc_ids, code_blob);

								ret = true;
							}
						}
					}
				}
			}
		}

		return ret;
	}

	void D3D11ShaderObject::ExtractNativeShader(ShaderType type, RenderEffect const & effect, std::vector<uint8_t>& native_shader_block)
	{
		native_shader_block.clear();

		ID3DBlobPtr code_blob = shader_code_[type].first;
		if (code_blob)
		{
			std::ostringstream oss(std::ios_base::binary | std::ios_base::out);

			uint32_t fourcc = MakeFourCC<'D', 'X', 'B', 'C'>::value;
			NativeToLittleEndian<sizeof(fourcc)>(&fourcc);
			oss.write(reinterpret_cast<char const *>(&fourcc), sizeof(fourcc));

			uint32_t ver = 2;
			NativeToLittleEndian<sizeof(ver)>(&ver);
			oss.write(reinterpret_cast<char const *>(&ver), sizeof(ver));

			uint64_t timestamp = effect.Timestamp();
			NativeToLittleEndian<sizeof(timestamp)>(&timestamp);
			oss.write(reinterpret_cast<char const *>(&timestamp), sizeof(timestamp));

			uint64_t hash_val = effect.PredefinedMacrosHash();
			NativeToLittleEndian<sizeof(hash_val)>(&hash_val);
			oss.write(reinterpret_cast<char const *>(&hash_val), sizeof(hash_val));

			uint8_t len = static_cast<uint8_t>(shader_code_[type].second.size());
			oss.write(reinterpret_cast<char const *>(&len), sizeof(len));
			oss.write(reinterpret_cast<char const *>(&shader_code_[type].second[0]), len);

			uint32_t blob_size = static_cast<uint32_t>(code_blob->GetBufferSize());
			NativeToLittleEndian<sizeof(blob_size)>(&blob_size);
			oss.write(reinterpret_cast<char const *>(&blob_size), sizeof(blob_size));
			oss.write(reinterpret_cast<char const *>(code_blob->GetBufferPointer()), code_blob->GetBufferSize());

			D3D11ShaderDesc const & sd = shader_desc_[type];

			uint16_t cb_desc_size = static_cast<uint16_t>(sd.cb_desc.size());
			NativeToLittleEndian<sizeof(cb_desc_size)>(&cb_desc_size);
			oss.write(reinterpret_cast<char const *>(&cb_desc_size), sizeof(cb_desc_size));
			for (size_t i = 0; i < sd.cb_desc.size(); ++ i)
			{
				uint32_t size = sd.cb_desc[i].size;
				NativeToLittleEndian<sizeof(size)>(&size);
				oss.write(reinterpret_cast<char const *>(&size), sizeof(size));

				uint16_t var_desc_size = static_cast<uint16_t>(sd.cb_desc[i].var_desc.size());
				NativeToLittleEndian<sizeof(var_desc_size)>(&var_desc_size);
				oss.write(reinterpret_cast<char const *>(&var_desc_size), sizeof(var_desc_size));
				for (size_t j = 0; j < sd.cb_desc[i].var_desc.size(); ++ j)
				{
					len = static_cast<uint8_t>(sd.cb_desc[i].var_desc[j].name.size());
					oss.write(reinterpret_cast<char const *>(&len), sizeof(len));
					oss.write(reinterpret_cast<char const *>(&sd.cb_desc[i].var_desc[j].name[0]), len);

					uint32_t start_offset = sd.cb_desc[i].var_desc[j].start_offset;
					NativeToLittleEndian<sizeof(start_offset)>(&start_offset);
					oss.write(reinterpret_cast<char const *>(&start_offset), sizeof(start_offset));
					oss.write(reinterpret_cast<char const *>(&sd.cb_desc[i].var_desc[j].type), sizeof(sd.cb_desc[i].var_desc[j].type));
					oss.write(reinterpret_cast<char const *>(&sd.cb_desc[i].var_desc[j].rows), sizeof(sd.cb_desc[i].var_desc[j].rows));
					oss.write(reinterpret_cast<char const *>(&sd.cb_desc[i].var_desc[j].columns), sizeof(sd.cb_desc[i].var_desc[j].columns));
					uint16_t elements = sd.cb_desc[i].var_desc[j].elements;
					NativeToLittleEndian<sizeof(elements)>(&elements);
					oss.write(reinterpret_cast<char const *>(&elements), sizeof(elements));
				}
			}

			uint16_t num_samplers = sd.num_samplers;
			NativeToLittleEndian<sizeof(num_samplers)>(&num_samplers);
			oss.write(reinterpret_cast<char const *>(&num_samplers), sizeof(num_samplers));
			uint16_t num_srvs = sd.num_srvs;
			NativeToLittleEndian<sizeof(num_srvs)>(&num_srvs);
			oss.write(reinterpret_cast<char const *>(&num_srvs), sizeof(num_srvs));
			uint16_t num_uavs = sd.num_uavs;
			NativeToLittleEndian<sizeof(num_uavs)>(&num_uavs);
			oss.write(reinterpret_cast<char const *>(&sd.num_uavs), sizeof(sd.num_uavs));

			uint16_t res_desc_size = static_cast<uint16_t>(sd.res_desc.size());
			NativeToLittleEndian<sizeof(res_desc_size)>(&res_desc_size);
			oss.write(reinterpret_cast<char const *>(&res_desc_size), sizeof(res_desc_size));
			for (size_t i = 0; i < sd.res_desc.size(); ++ i)
			{
				len = static_cast<uint8_t>(sd.res_desc[i].name.size());
				oss.write(reinterpret_cast<char const *>(&len), sizeof(len));
				oss.write(reinterpret_cast<char const *>(&sd.res_desc[i].name[0]), len);

				oss.write(reinterpret_cast<char const *>(&sd.res_desc[i].type), sizeof(sd.res_desc[i].type));
				oss.write(reinterpret_cast<char const *>(&sd.res_desc[i].dimension), sizeof(sd.res_desc[i].dimension));
				uint16_t bind_point = sd.res_desc[i].bind_point;
				NativeToLittleEndian<sizeof(bind_point)>(&bind_point);
				oss.write(reinterpret_cast<char const *>(&bind_point), sizeof(bind_point));
			}

			if (ST_VertexShader == type)
			{
				uint32_t vs_signature = vs_signature_;
				NativeToLittleEndian<sizeof(vs_signature)>(&vs_signature);
				oss.write(reinterpret_cast<char const *>(&vs_signature), sizeof(vs_signature));
			}

			std::string out_str = oss.str();
			native_shader_block.resize(out_str.size());
			memcpy(&native_shader_block[0], &out_str[0], out_str.size());
		}
	}

	ID3DBlobPtr D3D11ShaderObject::CompiteToBytecode(ShaderType type, RenderEffect const & effect, std::vector<uint32_t> const & shader_desc_ids)
	{
		D3D11RenderEngine const & render_eng = *checked_cast<D3D11RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		D3D_FEATURE_LEVEL feature_level = render_eng.DeviceFeatureLevel();
		RenderDeviceCaps const & caps = render_eng.DeviceCaps();

		std::string max_sm_str;
		{
			std::stringstream ss;
			ss << static_cast<int>(caps.max_shader_model);
			max_sm_str = ss.str();
		}
		std::string max_tex_array_str;
		{
			std::stringstream ss;
			ss << caps.max_texture_array_length;
			max_tex_array_str = ss.str();
		}
		std::string max_tex_depth_str;
		{
			std::stringstream ss;
			ss << caps.max_texture_depth;
			max_tex_depth_str = ss.str();
		}
		std::string max_tex_units_str;
		{
			std::stringstream ss;
			ss << static_cast<int>(caps.max_pixel_texture_units);
			max_tex_units_str = ss.str();
		}
		std::string flipping_str;
		{
			std::stringstream ss;
			ss << (render_eng.RequiresFlipping() ? -1 : +1);
			flipping_str = ss.str();
		}
		std::string standard_derivatives_str;
		{
			std::stringstream ss;
			ss << (caps.standard_derivatives_support ? 1 : 0);
			standard_derivatives_str = ss.str();
		}

		shader_desc const & sd = effect.GetShaderDesc(shader_desc_ids[type]);

		std::string shader_text = this->GenShaderText(effect, static_cast<ShaderType>(type));

		is_shader_validate_[type] = true;

		std::string shader_profile = sd.profile;
		switch (type)
		{
		case ST_VertexShader:
			if ("auto" == shader_profile)
			{
				shader_profile = render_eng.VertexShaderProfile();
			}
			break;

		case ST_PixelShader:
			if ("auto" == shader_profile)
			{
				shader_profile = render_eng.PixelShaderProfile();
			}
			break;

		case ST_GeometryShader:
			if (caps.max_shader_model < 4)
			{
				is_shader_validate_[type] = false;
			}
			else
			{
				if ("auto" == shader_profile)
				{
					shader_profile = render_eng.GeometryShaderProfile();
				}
			}
			break;

		case ST_ComputeShader:
			if (caps.max_shader_model < 4)
			{
				is_shader_validate_[type] = false;
			}
			else
			{
				if ("auto" == shader_profile)
				{
					shader_profile = render_eng.ComputeShaderProfile();
				}
				if (("cs_5_0" == shader_profile) && (caps.max_shader_model < 5))
				{
					is_shader_validate_[type] = false;
				}
			}
			break;

		case ST_HullShader:
			if (caps.max_shader_model < 5)
			{
				is_shader_validate_[type] = false;
			}
			else
			{
				if ("auto" == shader_profile)
				{
					shader_profile = render_eng.HullShaderProfile();
				}
			}
			break;

		case ST_DomainShader:
			if (caps.max_shader_model < 5)
			{
				is_shader_validate_[type] = false;
			}
			else
			{
				if ("auto" == shader_profile)
				{
					shader_profile = render_eng.DomainShaderProfile();
				}
			}
			break;

		default:
			is_shader_validate_[type] = false;
			break;
		}
		shader_code_[type].second = shader_profile;

		ID3DBlob* code = NULL;
		if (is_shader_validate_[type])
		{
			ID3DBlob* err_msg;
			std::vector<D3D_SHADER_MACRO> macros;
			{
				D3D_SHADER_MACRO macro_cb = { "CONSTANT_BUFFER", "1" };
				macros.push_back(macro_cb);
			}
			{
				D3D_SHADER_MACRO macro_d3d11 = { "KLAYGE_D3D11", "1" };
				macros.push_back(macro_d3d11);
			}
			{
				D3D_SHADER_MACRO macro_d3d11 = { "KLAYGE_SHADER_MODEL", max_sm_str.c_str() };
				macros.push_back(macro_d3d11);
			}
			{
				D3D_SHADER_MACRO macro_d3d11 = { "KLAYGE_MAX_TEX_ARRAY_LEN", max_tex_array_str.c_str() };
				macros.push_back(macro_d3d11);
			}
			{
				D3D_SHADER_MACRO macro_d3d11 = { "KLAYGE_MAX_TEX_DEPTH", max_tex_depth_str.c_str() };
				macros.push_back(macro_d3d11);
			}
			{
				D3D_SHADER_MACRO macro_d3d11 = { "KLAYGE_MAX_TEX_UNITS", max_tex_units_str.c_str() };
				macros.push_back(macro_d3d11);
			}
			{
				D3D_SHADER_MACRO macro_d3d11 = { "KLAYGE_NO_TEX_LOD", "0" };
				macros.push_back(macro_d3d11);
			}
			{
				D3D_SHADER_MACRO macro_d3d11 = { "KLAYGE_FLIPPING", flipping_str.c_str() };
				macros.push_back(macro_d3d11);
			}
			{
				D3D_SHADER_MACRO macro_d3d11 = { "KLAYGE_DERIVATIVES", standard_derivatives_str.c_str() };
				macros.push_back(macro_d3d11);
			}
			if (feature_level <= D3D_FEATURE_LEVEL_9_3)
			{
				D3D_SHADER_MACRO macro_bc5_as_bc3 = { "KLAYGE_BC5_AS_AG", "1" };
				macros.push_back(macro_bc5_as_bc3);
			}
			{
				D3D_SHADER_MACRO macro_end = { NULL, NULL };
				macros.push_back(macro_end);
			}
			D3DCompile(shader_text.c_str(), static_cast<UINT>(shader_text.size()), NULL, &macros[0],
				NULL, sd.func_name.c_str(), shader_profile.c_str(),
				0, 0, &code, &err_msg);
			if (err_msg != NULL)
			{
				LogError("Error when compiling %s:", sd.func_name.c_str());

				std::string err_str(static_cast<char*>(err_msg->GetBufferPointer()));
				std::string::size_type pos = err_str.find("): error X");
				if (pos == std::string::npos)
				{
					pos = err_str.find("): warning X");
				}
				if (pos != std::string::npos)
				{
					std::string part_err_str = err_str.substr(0, pos);
					pos = part_err_str.rfind("(");
					part_err_str = part_err_str.substr(pos + 1);
					int err_line;
					std::istringstream iss(part_err_str);
					iss >> err_line;

					iss.str(shader_text);
					std::string s;
					int line = 1;
					LogError("...");
					while (iss)
					{
						std::getline(iss, s);
						if ((line - err_line > -3) && (line - err_line < 3))
						{
							LogError("%d %s", line, s.c_str());
						}
						++ line;
					}
					LogError("...");
					LogError(err_str.c_str());
				}
				else
				{
					LogError(err_str.c_str());
				}

				err_msg->Release();
			}

			if (code)
			{
				ID3D11ShaderReflection* reflection;
				D3DReflect(code->GetBufferPointer(), code->GetBufferSize(), IID_ID3D11ShaderReflection, reinterpret_cast<void**>(&reflection));
				if (reflection != NULL)
				{
					D3D11_SHADER_DESC desc;
					reflection->GetDesc(&desc);

					shader_desc_[type].cb_desc.resize(desc.ConstantBuffers);
					for (UINT c = 0; c < desc.ConstantBuffers; ++ c)
					{
						ID3D11ShaderReflectionConstantBuffer* reflection_cb = reflection->GetConstantBufferByIndex(c);

						D3D11_SHADER_BUFFER_DESC cb_desc;
						reflection_cb->GetDesc(&cb_desc);
						shader_desc_[type].cb_desc[c].size = cb_desc.Size;

						for (UINT v = 0; v < cb_desc.Variables; ++ v)
						{
							ID3D11ShaderReflectionVariable* reflection_var = reflection_cb->GetVariableByIndex(v);

							D3D11_SHADER_VARIABLE_DESC var_desc;
							reflection_var->GetDesc(&var_desc);
							if (var_desc.uFlags & D3D_SVF_USED)
							{
								RenderEffectParameterPtr const & p = effect.ParameterByName(var_desc.Name);
								if (p)
								{
									D3D11_SHADER_TYPE_DESC type_desc;
									reflection_var->GetType()->GetDesc(&type_desc);

									D3D11ShaderDesc::ConstantBufferDesc::VariableDesc vd;
									vd.name = var_desc.Name;
									vd.start_offset = var_desc.StartOffset;
									vd.type = static_cast<uint8_t>(type_desc.Type);
									vd.rows = static_cast<uint8_t>(type_desc.Rows);
									vd.columns = static_cast<uint8_t>(type_desc.Columns);
									vd.elements = static_cast<uint16_t>(type_desc.Elements);
									shader_desc_[type].cb_desc[c].var_desc.push_back(vd);
								}
							}
						}
					}

					int num_samplers = -1;
					int num_srvs = -1;
					int num_uavs = -1;
					for (uint32_t i = 0; i < desc.BoundResources; ++ i)
					{
						D3D11_SHADER_INPUT_BIND_DESC si_desc;
						reflection->GetResourceBindingDesc(i, &si_desc);

						switch (si_desc.Type)
						{
						case D3D_SIT_SAMPLER:
							num_samplers = std::max(num_samplers, static_cast<int>(si_desc.BindPoint));
							break;

						case D3D_SIT_TEXTURE:
						case D3D_SIT_STRUCTURED:
						case D3D_SIT_BYTEADDRESS:
							num_srvs = std::max(num_srvs, static_cast<int>(si_desc.BindPoint));
							break;

						case D3D_SIT_UAV_RWTYPED:
						case D3D_SIT_UAV_RWSTRUCTURED:
						case D3D_SIT_UAV_RWBYTEADDRESS:
						case D3D_SIT_UAV_APPEND_STRUCTURED:
						case D3D_SIT_UAV_CONSUME_STRUCTURED:
							num_uavs = std::max(num_uavs, static_cast<int>(si_desc.BindPoint));
							break;

						default:
							break;
						}
					}

					shader_desc_[type].num_samplers = static_cast<uint16_t>(num_samplers + 1);
					shader_desc_[type].num_srvs = static_cast<uint16_t>(num_srvs + 1);
					shader_desc_[type].num_uavs = static_cast<uint16_t>(num_uavs + 1);

					for (uint32_t i = 0; i < desc.BoundResources; ++ i)
					{
						D3D11_SHADER_INPUT_BIND_DESC si_desc;
						reflection->GetResourceBindingDesc(i, &si_desc);

						switch (si_desc.Type)
						{
						case D3D_SIT_TEXTURE:
						case D3D_SIT_SAMPLER:
						case D3D_SIT_STRUCTURED:
						case D3D_SIT_BYTEADDRESS:
						case D3D_SIT_UAV_RWTYPED:
						case D3D_SIT_UAV_RWSTRUCTURED:
						case D3D_SIT_UAV_RWBYTEADDRESS:
						case D3D_SIT_UAV_APPEND_STRUCTURED:
						case D3D_SIT_UAV_CONSUME_STRUCTURED:
							{
								RenderEffectParameterPtr const & p = effect.ParameterByName(si_desc.Name);
								if (p)
								{
									D3D11ShaderDesc::BoundResourceDesc brd;
									brd.name = si_desc.Name;
									brd.type = static_cast<uint8_t>(si_desc.Type);
									brd.bind_point = static_cast<uint16_t>(si_desc.BindPoint);
									shader_desc_[type].res_desc.push_back(brd);
								}
							}
							break;

						default:
							break;
						}
					}

					if (ST_VertexShader == type)
					{
						vs_signature_ = 0;
						D3D11_SIGNATURE_PARAMETER_DESC signature;
						for (uint32_t i = 0; i < desc.InputParameters; ++ i)
						{
							reflection->GetInputParameterDesc(i, &signature);

							size_t seed = boost::hash_range(signature.SemanticName, signature.SemanticName + strlen(signature.SemanticName));
							boost::hash_combine(seed, signature.SemanticIndex);
							boost::hash_combine(seed, signature.Register);
							boost::hash_combine(seed, signature.SystemValueType);
							boost::hash_combine(seed, signature.ComponentType);
							boost::hash_combine(seed, signature.Mask);
							boost::hash_combine(seed, signature.ReadWriteMask);
							boost::hash_combine(seed, signature.Stream);

							boost::hash_combine(vs_signature_, seed);
						}
					}

					reflection->Release();
				}

				ID3DBlob* strip_code = NULL;
				D3DStripShader(code->GetBufferPointer(), code->GetBufferSize(),
					D3DCOMPILER_STRIP_REFLECTION_DATA | D3DCOMPILER_STRIP_DEBUG_INFO | D3DCOMPILER_STRIP_TEST_BLOBS,
					&strip_code);
				if (strip_code)
				{
					code->Release();
					code = strip_code;
				}
			}
		}

		return MakeCOMPtr(code);
	}

	void D3D11ShaderObject::AttachShaderBytecode(ShaderType type, RenderEffect const & effect, std::vector<uint32_t> const & shader_desc_ids, ID3DBlobPtr const & code_blob)
	{
		if (!code_blob)
		{
			is_shader_validate_[type] = false;
		}
		else
		{
			D3D11RenderEngine const & render_eng = *checked_cast<D3D11RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			ID3D11DevicePtr const & d3d_device = render_eng.D3DDevice();
			RenderDeviceCaps const & caps = render_eng.DeviceCaps();

			shader_desc const & sd = effect.GetShaderDesc(shader_desc_ids[type]);

			switch (type)
			{
			case ST_VertexShader:
				ID3D11VertexShader* vs;
				if (FAILED(d3d_device->CreateVertexShader(code_blob->GetBufferPointer(), code_blob->GetBufferSize(), NULL, &vs)))
				{
					is_shader_validate_[type] = false;
				}
				else
				{
					vertex_shader_ = MakeCOMPtr(vs);

					if (!sd.so_decl.empty())
					{
						std::vector<D3D11_SO_DECLARATION_ENTRY> d3d11_decl(sd.so_decl.size());
						for (size_t i = 0; i < sd.so_decl.size(); ++ i)
						{
							d3d11_decl[i] = D3D11Mapping::Mapping(sd.so_decl[i], static_cast<uint8_t>(i));
						}

						UINT rasterized_stream = 0;
						if ((caps.max_shader_model >= 5) && (effect.GetShaderDesc(shader_desc_ids[ST_PixelShader]).func_name.empty()))
						{
							rasterized_stream = D3D11_SO_NO_RASTERIZED_STREAM;
						}

						ID3D11GeometryShader* gs;
						if (FAILED(d3d_device->CreateGeometryShaderWithStreamOutput(code_blob->GetBufferPointer(), code_blob->GetBufferSize(),
							&d3d11_decl[0], static_cast<UINT>(d3d11_decl.size()), 0, 0, rasterized_stream, NULL, &gs)))
						{
							is_shader_validate_[type] = false;
						}
						else
						{
							geometry_shader_ = MakeCOMPtr(gs);
						}
					}

					shader_code_[type].first = code_blob;
				}
				break;

			case ST_PixelShader:
				ID3D11PixelShader* ps;
				if (FAILED(d3d_device->CreatePixelShader(code_blob->GetBufferPointer(), code_blob->GetBufferSize(), NULL, &ps)))
				{
					is_shader_validate_[type] = false;
				}
				else
				{
					pixel_shader_ = MakeCOMPtr(ps);
					shader_code_[type].first = code_blob;
				}
				break;

			case ST_GeometryShader:
				if (!sd.so_decl.empty())
				{
					std::vector<D3D11_SO_DECLARATION_ENTRY> d3d11_decl(sd.so_decl.size());
					for (size_t i = 0; i < sd.so_decl.size(); ++ i)
					{
						d3d11_decl[i] = D3D11Mapping::Mapping(sd.so_decl[i], static_cast<uint8_t>(i));
					}

					UINT rasterized_stream = 0;
					if ((caps.max_shader_model >= 5) && (effect.GetShaderDesc(shader_desc_ids[ST_PixelShader]).func_name.empty()))
					{
						rasterized_stream = D3D11_SO_NO_RASTERIZED_STREAM;
					}

					ID3D11GeometryShader* gs;
					if (FAILED(d3d_device->CreateGeometryShaderWithStreamOutput(code_blob->GetBufferPointer(), code_blob->GetBufferSize(),
						&d3d11_decl[0], static_cast<UINT>(d3d11_decl.size()), 0, 0, rasterized_stream, NULL, &gs)))
					{
						is_shader_validate_[type] = false;
					}
					else
					{
						geometry_shader_ = MakeCOMPtr(gs);
						shader_code_[type].first = code_blob;
					}
				}
				else
				{
					ID3D11GeometryShader* gs;
					if (FAILED(d3d_device->CreateGeometryShader(code_blob->GetBufferPointer(), code_blob->GetBufferSize(), NULL, &gs)))
					{
						is_shader_validate_[type] = false;
					}
					else
					{
						geometry_shader_ = MakeCOMPtr(gs);
						shader_code_[type].first = code_blob;
					}
				}
				break;

			case ST_ComputeShader:
				ID3D11ComputeShader* cs;
				if (FAILED(d3d_device->CreateComputeShader(code_blob->GetBufferPointer(), code_blob->GetBufferSize(), NULL, &cs)))
				{
					is_shader_validate_[type] = false;
				}
				else
				{
					compute_shader_ = MakeCOMPtr(cs);
					shader_code_[type].first = code_blob;
				}
				break;

			case ST_HullShader:
				ID3D11HullShader* hs;
				if (FAILED(d3d_device->CreateHullShader(code_blob->GetBufferPointer(), code_blob->GetBufferSize(), NULL, &hs)))
				{
					is_shader_validate_[type] = false;
				}
				else
				{
					hull_shader_ = MakeCOMPtr(hs);
					shader_code_[type].first = code_blob;
					has_tessellation_ = true;
				}
				break;

			case ST_DomainShader:
				ID3D11DomainShader* ds;
				if (FAILED(d3d_device->CreateDomainShader(code_blob->GetBufferPointer(), code_blob->GetBufferSize(), NULL, &ds)))
				{
					is_shader_validate_[type] = false;
				}
				else
				{
					domain_shader_ = MakeCOMPtr(ds);
					shader_code_[type].first = code_blob;
					has_tessellation_ = true;
				}
				break;

			default:
				is_shader_validate_[type] = false;
				break;
			}

			// Shader reflection
			dirty_[type].resize(shader_desc_[type].cb_desc.size());
			cbufs_[type].resize(shader_desc_[type].cb_desc.size());
			mem_cbufs_[type].resize(shader_desc_[type].cb_desc.size());
			for (size_t c = 0; c < shader_desc_[type].cb_desc.size(); ++ c)
			{
				mem_cbufs_[type][c].resize(shader_desc_[type].cb_desc[c].size);

				for (size_t v = 0; v < shader_desc_[type].cb_desc[c].var_desc.size(); ++ v)
				{
					RenderEffectParameterPtr const & p = effect.ParameterByName(shader_desc_[type].cb_desc[c].var_desc[v].name);
					BOOST_ASSERT(p);

					D3D11ShaderParameterHandle p_handle;
					p_handle.shader_type = static_cast<uint8_t>(type);
					p_handle.param_type = static_cast<D3D_SHADER_VARIABLE_TYPE>(shader_desc_[type].cb_desc[c].var_desc[v].type);
					p_handle.cbuff = static_cast<uint32_t>(c);
					p_handle.offset = shader_desc_[type].cb_desc[c].var_desc[v].start_offset;
					p_handle.elements = shader_desc_[type].cb_desc[c].var_desc[v].elements;
					p_handle.rows = shader_desc_[type].cb_desc[c].var_desc[v].rows;
					p_handle.columns = shader_desc_[type].cb_desc[c].var_desc[v].columns;

					param_binds_[type].push_back(this->GetBindFunc(p_handle, p));
				}

				D3D11_BUFFER_DESC buf_desc;
				buf_desc.ByteWidth = (shader_desc_[type].cb_desc[c].size + 15) / 16 * 16;
				buf_desc.Usage = D3D11_USAGE_DYNAMIC;
				buf_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
				buf_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
				buf_desc.MiscFlags = 0;
				ID3D11Buffer* tmp_buf;
				TIF(d3d_device->CreateBuffer(&buf_desc, NULL, &tmp_buf));
				cbufs_[type][c] = MakeCOMPtr(tmp_buf);
			}

			samplers_[type].resize(shader_desc_[type].num_samplers);
			srvsrcs_[type].resize(shader_desc_[type].num_srvs, NULL);
			srvs_[type].resize(shader_desc_[type].num_srvs);
			uavsrcs_[type].resize(shader_desc_[type].num_uavs, NULL);
			uavs_[type].resize(shader_desc_[type].num_uavs);

			for (size_t i = 0; i < shader_desc_[type].res_desc.size(); ++ i)
			{
				RenderEffectParameterPtr const & p = effect.ParameterByName(shader_desc_[type].res_desc[i].name);
				BOOST_ASSERT(p);

				D3D11ShaderParameterHandle p_handle;
				p_handle.shader_type = static_cast<uint8_t>(type);
				if (D3D_SIT_SAMPLER == shader_desc_[type].res_desc[i].type)
				{
					p_handle.param_type = D3D_SVT_SAMPLER;
				}
				else
				{
					if (D3D_SRV_DIMENSION_BUFFER == shader_desc_[type].res_desc[i].dimension)
					{
						p_handle.param_type = D3D_SVT_BUFFER;
					}
					else
					{
						p_handle.param_type = D3D_SVT_TEXTURE;
					}
				}
				p_handle.cbuff = 0;
				p_handle.offset = shader_desc_[type].res_desc[i].bind_point;
				p_handle.elements = 1;
				p_handle.rows = 0;
				p_handle.columns = 1;

				param_binds_[type].push_back(this->GetBindFunc(p_handle, p));
			}
		}
	}

	void D3D11ShaderObject::AttachShader(ShaderType type, RenderEffect const & effect, std::vector<uint32_t> const & shader_desc_ids)
	{
		ID3DBlobPtr code_blob = this->CompiteToBytecode(type, effect, shader_desc_ids);
		this->AttachShaderBytecode(type, effect, shader_desc_ids, code_blob);
	}

	void D3D11ShaderObject::AttachShader(ShaderType type, RenderEffect const & /*effect*/, ShaderObjectPtr const & shared_so)
	{
		D3D11RenderEngine const & render_eng = *checked_cast<D3D11RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11DevicePtr const & d3d_device = render_eng.D3DDevice();

		if (shared_so)
		{
			D3D11ShaderObject const & so = *checked_cast<D3D11ShaderObject*>(shared_so.get());

			is_shader_validate_[type] = so.is_shader_validate_[type];
			shader_code_[type] = so.shader_code_[type];
			shader_desc_[type] = so.shader_desc_[type];
			switch (type)
			{
			case ST_VertexShader:
				vertex_shader_ = so.vertex_shader_;
				vs_signature_ = so.vs_signature_;
				geometry_shader_ = so.geometry_shader_;
				break;

			case ST_PixelShader:
				pixel_shader_ = so.pixel_shader_;
				break;

			case ST_GeometryShader:
				geometry_shader_ = so.geometry_shader_;
				break;

			case ST_ComputeShader:
				compute_shader_ = so.compute_shader_;
				break;

			case ST_HullShader:
				hull_shader_ = so.hull_shader_;
				if (hull_shader_)
				{
					has_tessellation_ = true;
				}
				break;

			case ST_DomainShader:
				domain_shader_ = so.domain_shader_;
				if (domain_shader_)
				{
					has_tessellation_ = true;
				}
				break;

			default:
				is_shader_validate_[type] = false;
				break;
			}

			samplers_[type].resize(so.samplers_[type].size());
			srvsrcs_[type].resize(so.srvs_[type].size(), NULL);
			srvs_[type].resize(so.srvs_[type].size());
			uavsrcs_[type].resize(so.uavs_[type].size(), NULL);
			uavs_[type].resize(so.uavs_[type].size());

			mem_cbufs_[type] = so.mem_cbufs_[type];
			dirty_[type] = so.dirty_[type];

			cbufs_[type].resize(so.cbufs_[type].size());
			D3D11_BUFFER_DESC desc;
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.MiscFlags = 0;
			for (size_t j = 0; j < so.cbufs_[type].size(); ++ j)
			{
				desc.ByteWidth = static_cast<UINT>(so.mem_cbufs_[type][j].size());
				ID3D11Buffer* tmp_buf;
				TIF(d3d_device->CreateBuffer(&desc, NULL, &tmp_buf));
				cbufs_[type][j] = MakeCOMPtr(tmp_buf);
			}

			param_binds_[type].reserve(so.param_binds_[type].size());
			typedef BOOST_TYPEOF(so.param_binds_[type]) ParamBindsType;
			BOOST_FOREACH(ParamBindsType::const_reference pb, so.param_binds_[type])
			{
				param_binds_[type].push_back(this->GetBindFunc(pb.p_handle, pb.param));
			}
		}
	}

	void D3D11ShaderObject::LinkShaders(RenderEffect const & /*effect*/)
	{
		is_validate_ = true;
		for (size_t type = 0; type < ShaderObject::ST_NumShaderTypes; ++ type)
		{
			is_validate_ &= is_shader_validate_[type];
		}
	}
	
	ShaderObjectPtr D3D11ShaderObject::Clone(RenderEffect const & effect)
	{
		ID3D11DevicePtr const & d3d_device = checked_cast<D3D11RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance())->D3DDevice();

		D3D11ShaderObjectPtr ret = MakeSharedPtr<D3D11ShaderObject>();
		ret->has_discard_ = has_discard_;
		ret->has_tessellation_ = has_tessellation_;
		ret->is_validate_ = is_validate_;
		ret->is_shader_validate_ = is_shader_validate_;
		ret->vertex_shader_ = vertex_shader_;
		ret->pixel_shader_ = pixel_shader_;
		ret->geometry_shader_ = geometry_shader_;
		ret->compute_shader_ = compute_shader_;
		ret->hull_shader_ = hull_shader_;
		ret->domain_shader_ = domain_shader_;
		ret->vs_signature_ = vs_signature_;
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			ret->shader_code_[i] = shader_code_[i];
			ret->shader_desc_[i] = shader_desc_[i];

			ret->samplers_[i].resize(samplers_[i].size());
			ret->srvsrcs_[i].resize(srvsrcs_[i].size(), NULL);
			ret->srvs_[i].resize(srvs_[i].size());
			ret->uavsrcs_[i].resize(uavsrcs_[i].size(), NULL);
			ret->uavs_[i].resize(uavs_[i].size());

			ret->mem_cbufs_[i] = mem_cbufs_[i];
			ret->dirty_[i] = dirty_[i];

			ret->cbufs_[i].resize(cbufs_[i].size());
			D3D11_BUFFER_DESC desc;
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.MiscFlags = 0;
			for (size_t j = 0; j < cbufs_[i].size(); ++ j)
			{
				desc.ByteWidth = static_cast<UINT>((mem_cbufs_[i][j].size() + 15) / 16 * 16);
				ID3D11Buffer* tmp_buf;
				TIF(d3d_device->CreateBuffer(&desc, NULL, &tmp_buf));
				ret->cbufs_[i][j] = MakeCOMPtr(tmp_buf);
			}

			ret->param_binds_[i].reserve(param_binds_[i].size());
			typedef BOOST_TYPEOF(param_binds_[i]) ParamBindsType;
			BOOST_FOREACH(ParamBindsType::const_reference pb, param_binds_[i])
			{
				ret->param_binds_[i].push_back(ret->GetBindFunc(pb.p_handle, effect.ParameterByName(*(pb.param->Name()))));
			}
		}

		return ret;
	}

	D3D11ShaderObject::parameter_bind_t D3D11ShaderObject::GetBindFunc(D3D11ShaderParameterHandle const & p_handle, RenderEffectParameterPtr const & param)
	{
		parameter_bind_t ret;
		ret.param = param;
		memcpy(&ret.p_handle, &p_handle, sizeof(p_handle));

		switch (param->type())
		{
		case REDT_bool:
			if (param->ArraySize())
			{
				switch (p_handle.param_type)
				{
				case D3D_SVT_BOOL:
					ret.func = SetD3D11ShaderParameter<bool*, BOOL>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.elements, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
					break;

				case D3D_SVT_UINT:
					ret.func = SetD3D11ShaderParameter<bool*, uint32_t>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.elements, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
					break;

				case D3D_SVT_INT:
					ret.func = SetD3D11ShaderParameter<bool*, int32_t>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.elements, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
					break;

				case D3D_SVT_FLOAT:
					ret.func = SetD3D11ShaderParameter<bool*, float>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.elements, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
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
				case D3D_SVT_BOOL:
					ret.func = SetD3D11ShaderParameter<bool, BOOL>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
					break;

				case D3D_SVT_UINT:
					ret.func = SetD3D11ShaderParameter<bool, uint32_t>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
					break;

				case D3D_SVT_INT:
					ret.func = SetD3D11ShaderParameter<bool, int32_t>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
					break;

				case D3D_SVT_FLOAT:
					ret.func = SetD3D11ShaderParameter<bool, float>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
					break;

				default:
					BOOST_ASSERT(false);
					break;
				}
			}
			break;

		case REDT_uint:
			if (param->ArraySize())
			{
				switch (p_handle.param_type)
				{
				case D3D_SVT_BOOL:
					ret.func = SetD3D11ShaderParameter<uint32_t*, BOOL>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.elements, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
					break;

				case D3D_SVT_UINT:
					ret.func = SetD3D11ShaderParameter<uint32_t*, uint32_t>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.elements, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
					break;

				case D3D_SVT_INT:
					ret.func = SetD3D11ShaderParameter<uint32_t*, int32_t>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.elements, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
					break;

				case D3D_SVT_FLOAT:
					ret.func = SetD3D11ShaderParameter<uint32_t*, float>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.elements, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
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
				case D3D_SVT_BOOL:
					ret.func = SetD3D11ShaderParameter<uint32_t, BOOL>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
					break;

				case D3D_SVT_UINT:
					ret.func = SetD3D11ShaderParameter<uint32_t, uint32_t>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
					break;

				case D3D_SVT_INT:
					ret.func = SetD3D11ShaderParameter<uint32_t, int32_t>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
					break;

				case D3D_SVT_FLOAT:
					ret.func = SetD3D11ShaderParameter<uint32_t, float>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
					break;

				default:
					BOOST_ASSERT(false);
					break;
				}
			}
			break;

		case REDT_int:
			if (param->ArraySize())
			{
				switch (p_handle.param_type)
				{
				case D3D_SVT_BOOL:
					ret.func = SetD3D11ShaderParameter<int32_t*, BOOL>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.elements, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
					break;

				case D3D_SVT_UINT:
					ret.func = SetD3D11ShaderParameter<int32_t*, uint32_t>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.elements, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
					break;

				case D3D_SVT_INT:
					ret.func = SetD3D11ShaderParameter<int32_t*, int32_t>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.elements, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
					break;

				case D3D_SVT_FLOAT:
					ret.func = SetD3D11ShaderParameter<int32_t*, float>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.elements, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
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
				case D3D_SVT_BOOL:
					ret.func = SetD3D11ShaderParameter<int32_t, BOOL>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
					break;

				case D3D_SVT_UINT:
					ret.func = SetD3D11ShaderParameter<int32_t, uint32_t>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
					break;

				case D3D_SVT_INT:
					ret.func = SetD3D11ShaderParameter<int32_t, int32_t>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
					break;

				case D3D_SVT_FLOAT:
					ret.func = SetD3D11ShaderParameter<int32_t, float>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
					break;

				default:
					BOOST_ASSERT(false);
					break;
				}
			}
			break;

		case REDT_float:
			if (param->ArraySize())
			{
				ret.func = SetD3D11ShaderParameter<float*, float>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.elements, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
			}
			else
			{
				ret.func = SetD3D11ShaderParameter<float, float>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
			}
			break;

		case REDT_uint2:
			if (param->ArraySize())
			{
				ret.func = SetD3D11ShaderParameter<uint2*, uint32_t>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.elements, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
			}
			else
			{
				ret.func = SetD3D11ShaderParameter<uint2, uint32_t>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
			}
			break;

		case REDT_uint3:
			if (param->ArraySize())
			{
				ret.func = SetD3D11ShaderParameter<uint3*, uint32_t>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.elements, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
			}
			else
			{
				ret.func = SetD3D11ShaderParameter<uint3, uint32_t>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
			}
			break;

		case REDT_uint4:
			if (param->ArraySize())
			{
				ret.func = SetD3D11ShaderParameter<uint4*, uint32_t>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.elements, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
			}
			else
			{
				ret.func = SetD3D11ShaderParameter<uint4, uint32_t>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
			}
			break;

		case REDT_int2:
			if (param->ArraySize())
			{
				ret.func = SetD3D11ShaderParameter<int2*, int32_t>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.elements, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
			}
			else
			{
				ret.func = SetD3D11ShaderParameter<int2, int32_t>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
			}
			break;

		case REDT_int3:
			if (param->ArraySize())
			{
				ret.func = SetD3D11ShaderParameter<int3*, int32_t>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.elements, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
			}
			else
			{
				ret.func = SetD3D11ShaderParameter<int3, int32_t>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
			}
			break;

		case REDT_int4:
			if (param->ArraySize())
			{
				ret.func = SetD3D11ShaderParameter<int4*, int32_t>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.elements, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
			}
			else
			{
				ret.func = SetD3D11ShaderParameter<int4, int32_t>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
			}
			break;

		case REDT_float2:
			if (param->ArraySize())
			{
				ret.func = SetD3D11ShaderParameter<float2*, float>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.elements, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
			}
			else
			{
				ret.func = SetD3D11ShaderParameter<float2, float>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
			}
			break;

		case REDT_float3:
			if (param->ArraySize())
			{
				ret.func = SetD3D11ShaderParameter<float3*, float>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.elements, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
			}
			else
			{
				ret.func = SetD3D11ShaderParameter<float3, float>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
			}
			break;

		case REDT_float4:
			if (param->ArraySize())
			{
				ret.func = SetD3D11ShaderParameter<float4*, float>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.elements, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
			}
			else
			{
				ret.func = SetD3D11ShaderParameter<float4, float>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
			}
			break;

		case REDT_float4x4:
			if (param->ArraySize())
			{
				ret.func = SetD3D11ShaderParameter<float4x4*, float>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.rows, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
			}
			else
			{
				ret.func = SetD3D11ShaderParameter<float4x4, float>(&mem_cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.rows, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
			}
			break;

		case REDT_sampler:
			ret.func = SetD3D11ShaderParameter<SamplerStateObjectPtr, ID3D11SamplerStatePtr>(samplers_[p_handle.shader_type][p_handle.offset], param);
			break;

		case REDT_texture1D:
		case REDT_texture2D:
		case REDT_texture3D:
		case REDT_textureCUBE:
		case REDT_texture1DArray:
		case REDT_texture2DArray:
		case REDT_texture3DArray:
		case REDT_textureCUBEArray:
			ret.func = SetD3D11ShaderParameter<TexturePtr, ID3D11ShaderResourceViewPtr>(srvsrcs_[p_handle.shader_type][p_handle.offset], srvs_[p_handle.shader_type][p_handle.offset], param);
			break;

		case REDT_buffer:
		case REDT_structured_buffer:
		case REDT_consume_structured_buffer:
		case REDT_append_structured_buffer:
		case REDT_byte_address_buffer:
			ret.func = SetD3D11ShaderParameter<GraphicsBufferPtr, ID3D11ShaderResourceViewPtr>(srvsrcs_[p_handle.shader_type][p_handle.offset], srvs_[p_handle.shader_type][p_handle.offset], param);
			break;

		case REDT_rw_texture1D:
		case REDT_rw_texture2D:
		case REDT_rw_texture3D:
		case REDT_rw_texture1DArray:
		case REDT_rw_texture2DArray:
			ret.func = SetD3D11ShaderParameter<TexturePtr, ID3D11UnorderedAccessViewPtr>(uavsrcs_[p_handle.shader_type][p_handle.offset], uavs_[p_handle.shader_type][p_handle.offset], param);
			break;

		case REDT_rw_buffer:
		case REDT_rw_structured_buffer:
		case REDT_rw_byte_address_buffer:
			ret.func = SetD3D11ShaderParameter<GraphicsBufferPtr, ID3D11UnorderedAccessViewPtr>(uavsrcs_[p_handle.shader_type][p_handle.offset], uavs_[p_handle.shader_type][p_handle.offset], param);
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}

		return ret;
	}

	void D3D11ShaderObject::Bind()
	{
		D3D11RenderEngine& re = *checked_cast<D3D11RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11DeviceContextPtr const & d3d_imm_ctx = re.D3DDeviceImmContext();

		re.VSSetShader(vertex_shader_);
		re.GSSetShader(geometry_shader_);
		re.PSSetShader(pixel_shader_);
		re.CSSetShader(compute_shader_);
		re.HSSetShader(hull_shader_);
		re.DSSetShader(domain_shader_);

		for (size_t st = 0; st < ST_NumShaderTypes; ++ st)
		{
			typedef BOOST_TYPEOF(param_binds_[st]) ParamBindsType;
			BOOST_FOREACH(ParamBindsType::reference pb, param_binds_[st])
			{
				pb.func();
			}
		}

		for (size_t i = 0; i < cbufs_.size(); ++ i)
		{
			for (size_t j = 0; j < cbufs_[i].size(); ++ j)
			{
				if (dirty_[i][j])
				{
					D3D11_MAPPED_SUBRESOURCE mapped;
					d3d_imm_ctx->Map(cbufs_[i][j].get(), D3D11CalcSubresource(0, 0, 1), D3D11_MAP_WRITE_DISCARD, 0, &mapped);
					memcpy(mapped.pData, &mem_cbufs_[i][j][0], static_cast<UINT>(mem_cbufs_[i][j].size()));
					d3d_imm_ctx->Unmap(cbufs_[i][j].get(), D3D11CalcSubresource(0, 0, 1));

					dirty_[i][j] = false;
				}
			}
		}

		for (size_t st = 0; st < ST_NumShaderTypes; ++ st)
		{
			if (!srvs_[st].empty())
			{
				re.SetShaderResources(static_cast<ShaderObject::ShaderType>(st), srvsrcs_[st], srvs_[st]);
			}

			if (!samplers_[st].empty())
			{
				re.SetSamplers(static_cast<ShaderObject::ShaderType>(st), samplers_[st]);
			}

			if (!cbufs_[st].empty())
			{
				re.SetConstantBuffers(static_cast<ShaderObject::ShaderType>(st), cbufs_[st]);
			}
		}

		if (!uavs_[ST_ComputeShader].empty())
		{
			std::vector<ID3D11UnorderedAccessView*> uavs_ptr(uavs_[ST_ComputeShader].size());
			for (uint32_t i = 0; i < uavs_[ST_ComputeShader].size(); ++ i)
			{
				if (uavsrcs_[ST_ComputeShader][i] != NULL)
				{
					re.DetachSRV(uavsrcs_[ST_ComputeShader][i], 0, 1);
				}

				uavs_ptr[i] = uavs_[ST_ComputeShader][i].get();
			}

			d3d_imm_ctx->CSSetUnorderedAccessViews(0, static_cast<UINT>(uavs_[ST_ComputeShader].size()), &uavs_ptr[0],
				reinterpret_cast<UINT*>(&uavs_ptr[0]));
		}
	}

	void D3D11ShaderObject::Unbind()
	{
		D3D11RenderEngine& re = *checked_cast<D3D11RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11DeviceContextPtr const & d3d_imm_ctx = re.D3DDeviceImmContext();

		if (!uavs_[ST_ComputeShader].empty())
		{
			std::vector<ID3D11UnorderedAccessView*> uavs(uavs_[ST_ComputeShader].size(), NULL);
			d3d_imm_ctx->CSSetUnorderedAccessViews(0, static_cast<UINT>(uavs_[ST_ComputeShader].size()), &uavs[0],
				reinterpret_cast<UINT*>(&uavs[0]));
		}
	}
}
