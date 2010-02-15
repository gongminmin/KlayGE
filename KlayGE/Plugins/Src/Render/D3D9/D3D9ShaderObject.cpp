// D3D9ShaderObject.cpp
// KlayGE D3D9 shader对象类 实现文件
// Ver 3.9.0
// 版权所有(C) 龚敏敏, 2006-2009
// Homepage: http://klayge.sourceforge.net
//
// 3.9.0
// 加速Shader编译 (2009.7.31)
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
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 6328)
#endif
#include <boost/tokenizer.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

#include <KlayGE/D3D9/D3D9MinGWDefs.hpp>

#include <d3dx9.h>

#include <KlayGE/D3D9/D3D9RenderEngine.hpp>
#include <KlayGE/D3D9/D3D9Mapping.hpp>
#include <KlayGE/D3D9/D3D9Texture.hpp>
#include <KlayGE/D3D9/D3D9RenderStateObject.hpp>
#include <KlayGE/D3D9/D3D9ShaderObject.hpp>

namespace
{
	using namespace KlayGE;


	char const * predefined_funcs = "\n								\
	float4 tex1DLevel(sampler s, float location, float lod)\n		\
	{\n																\
		return tex1Dlod(s, float4(location, 0, 0, lod));\n			\
	}\n																\
	\n																\
	float4 tex2DLevel(sampler s, float2 location, float lod)\n		\
	{\n																\
		return tex2Dlod(s, float4(location, 0, lod));\n				\
	}\n																\
	\n																\
	float4 tex3DLevel(sampler s, float3 location, float lod)\n		\
	{\n																\
		return tex3Dlod(s, float4(location, lod));\n				\
	}\n																\
	\n																\
	float4 texCUBELevel(sampler s, float3 location, float lod)\n	\
	{\n																\
		return texCUBElod(s, float4(location, lod));\n				\
	}\n																\
	\n																\
	\n																\
	float4 tex1DBias(sampler s, float location, float lod)\n		\
	{\n																\
		return tex1Dbias(s, float4(location, 0, 0, lod));\n			\
	}\n																\
	\n																\
	float4 tex2DBias(sampler s, float2 location, float lod)\n		\
	{\n																\
		return tex2Dbias(s, float4(location, 0, lod));\n			\
	}\n																\
	\n																\
	float4 tex3DBias(sampler s, float3 location, float lod)\n		\
	{\n																\
		return tex3Dbias(s, float4(location, lod));\n				\
	}\n																\
	\n																\
	float4 texCUBEBias(sampler s, float3 location, float lod)\n		\
	{\n																\
		return texCUBEbias(s, float4(location, lod));\n				\
	}\n																\
	";


	template <typename SrcType, typename DstType>
	class SetD3D9ShaderParameter
	{
	};

	template <typename SrcType>
	class SetD3D9ShaderParameter<SrcType, bool>
	{
	public:
		SetD3D9ShaderParameter(BOOL& bool_reg, RenderEffectParameterPtr const & param)
			: bool_reg_(&bool_reg), param_(param)
		{
		}

		void operator()()
		{
			SrcType v;
			param_->Value(v);

			*bool_reg_ = static_cast<BOOL>(v);
		}

	private:
		BOOL* bool_reg_;
		RenderEffectParameterPtr param_;
	};

	template <typename SrcType>
	class SetD3D9ShaderParameter<SrcType, int32_t>
	{
	public:
		SetD3D9ShaderParameter(int32_t& int_reg, RenderEffectParameterPtr const & param)
			: int_reg_(&int_reg), param_(param)
		{
		}

		void operator()()
		{
			SrcType v;
			param_->Value(v);

			*int_reg_ = static_cast<int32_t>(v);
		}

	private:
		int32_t* int_reg_;
		RenderEffectParameterPtr param_;
	};

	template <typename SrcType>
	class SetD3D9ShaderParameter<SrcType, float>
	{
	public:
		SetD3D9ShaderParameter(float& float_reg, RenderEffectParameterPtr const & param)
			: float_reg_(&float_reg), param_(param)
		{
		}

		void operator()()
		{
			SrcType v;
			param_->Value(v);

			*float_reg_ = static_cast<float>(v);
		}

	private:
		float* float_reg_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetD3D9ShaderParameter<int2, int32_t>
	{
	public:
		SetD3D9ShaderParameter(int4& int_reg, RenderEffectParameterPtr const & param)
			: int_reg_(&int_reg), param_(param)
		{
		}

		void operator()()
		{
			int2 v;
			param_->Value(v);

			int_reg_->x() = v.x();
			int_reg_->y() = v.y();
		}

	private:
		int4* int_reg_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetD3D9ShaderParameter<int2, float>
	{
	public:
		SetD3D9ShaderParameter(float4& float_reg, RenderEffectParameterPtr const & param)
			: float_reg_(&float_reg), param_(param)
		{
		}

		void operator()()
		{
			int2 v;
			param_->Value(v);

			float_reg_->x() = static_cast<float>(v.x());
			float_reg_->y() = static_cast<float>(v.y());
		}

	private:
		float4* float_reg_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetD3D9ShaderParameter<int3, int32_t>
	{
	public:
		SetD3D9ShaderParameter(int4& int_reg, RenderEffectParameterPtr const & param)
			: int_reg_(&int_reg), param_(param)
		{
		}

		void operator()()
		{
			int3 v;
			param_->Value(v);

			int_reg_->x() = v.x();
			int_reg_->y() = v.y();
			int_reg_->z() = v.z();
		}

	private:
		int4* int_reg_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetD3D9ShaderParameter<int3, float>
	{
	public:
		SetD3D9ShaderParameter(float4& float_reg, RenderEffectParameterPtr const & param)
			: float_reg_(&float_reg), param_(param)
		{
		}

		void operator()()
		{
			int3 v;
			param_->Value(v);

			float_reg_->x() = static_cast<float>(v.x());
			float_reg_->y() = static_cast<float>(v.y());
			float_reg_->z() = static_cast<float>(v.z());
		}

	private:
		float4* float_reg_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetD3D9ShaderParameter<int4, int32_t>
	{
	public:
		SetD3D9ShaderParameter(int4& int_reg, RenderEffectParameterPtr const & param)
			: int_reg_(&int_reg), param_(param)
		{
		}

		void operator()()
		{
			param_->Value(*int_reg_);
		}

	private:
		int4* int_reg_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetD3D9ShaderParameter<int4, float>
	{
	public:
		SetD3D9ShaderParameter(float4& float_reg, RenderEffectParameterPtr const & param)
			: float_reg_(&float_reg), param_(param)
		{
		}

		void operator()()
		{
			float4 v;
			param_->Value(v);

			float_reg_->x() = static_cast<float>(v.x());
			float_reg_->y() = static_cast<float>(v.y());
			float_reg_->z() = static_cast<float>(v.z());
			float_reg_->w() = static_cast<float>(v.w());
		}

	private:
		float4* float_reg_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetD3D9ShaderParameter<float2, float>
	{
	public:
		SetD3D9ShaderParameter(float4& float_reg, RenderEffectParameterPtr const & param)
			: float_reg_(&float_reg), param_(param)
		{
		}

		void operator()()
		{
			float2 v;
			param_->Value(v);

			float_reg_->x() = v.x();
			float_reg_->y() = v.y();
		}

	private:
		float4* float_reg_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetD3D9ShaderParameter<float3, float>
	{
	public:
		SetD3D9ShaderParameter(float4& float_reg, RenderEffectParameterPtr const & param)
			: float_reg_(&float_reg), param_(param)
		{
		}

		void operator()()
		{
			float3 v;
			param_->Value(v);

			float_reg_->x() = v.x();
			float_reg_->y() = v.y();
			float_reg_->z() = v.z();
		}

	private:
		float4* float_reg_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetD3D9ShaderParameter<float4, float>
	{
	public:
		SetD3D9ShaderParameter(float4& float_reg, RenderEffectParameterPtr const & param)
			: float_reg_(&float_reg), param_(param)
		{
		}

		void operator()()
		{
			param_->Value(*float_reg_);
		}

	private:
		float4* float_reg_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetD3D9ShaderParameter<float4x4, float>
	{
	public:
		SetD3D9ShaderParameter(float4* float_reg, uint16_t reg_count, RenderEffectParameterPtr const & param)
			: float_reg_(float_reg), reg_size_(reg_count * sizeof(float4)), param_(param)
		{
		}

		void operator()()
		{
			float4x4 v;
			param_->Value(v);

			v = MathLib::transpose(v);
			memcpy(float_reg_, &v[0], reg_size_);
		}

	private:
		float4* float_reg_;
		size_t reg_size_;
		RenderEffectParameterPtr param_;
	};

	template <typename SrcType>
	class SetD3D9ShaderParameter<SrcType*, bool>
	{
	public:
		SetD3D9ShaderParameter(Vector_T<BOOL, 4>* bool_regs, RenderEffectParameterPtr const & param)
			: bool_regs_(bool_regs), param_(param)
		{
		}

		void operator()()
		{
			std::vector<SrcType> v;
			param_->Value(v);

			for (size_t i = 0; i < v.size(); ++ i)
			{
				bool_regs_[i].x() = v[i];
			}
		}

	private:
		Vector_T<BOOL, 4>* bool_regs_;
		RenderEffectParameterPtr param_;
	};

	template <typename SrcType>
	class SetD3D9ShaderParameter<SrcType*, int32_t>
	{
	public:
		SetD3D9ShaderParameter(int4* int_regs, RenderEffectParameterPtr const & param)
			: int_regs_(int_regs), param_(param)
		{
		}

		void operator()()
		{
			std::vector<SrcType> v;
			param_->Value(v);

			for (size_t i = 0; i < v.size(); ++ i)
			{
				int_regs_[i].x() = v[i];
			}
		}

	private:
		int4* int_regs_;
		RenderEffectParameterPtr param_;
	};

	template <typename SrcType>
	class SetD3D9ShaderParameter<SrcType*, float>
	{
	public:
		SetD3D9ShaderParameter(float4* float_regs, RenderEffectParameterPtr const & param)
			: float_regs_(float_regs), param_(param)
		{
		}

		void operator()()
		{
			std::vector<SrcType> v;
			param_->Value(v);

			for (size_t i = 0; i < v.size(); ++ i)
			{
				float_regs_[i].x() = static_cast<float>(v[i]);
			}
		}

	private:
		float4* float_regs_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetD3D9ShaderParameter<int2*, int32_t>
	{
	public:
		SetD3D9ShaderParameter(int4* int_regs, uint16_t reg_count, RenderEffectParameterPtr const & param)
			: int_regs_(int_regs), reg_size_(reg_count * sizeof(int4)), param_(param)
		{
		}

		void operator()()
		{
			std::vector<int2> v;
			param_->Value(v);

			std::vector<int4> v4(v.size());
			for (size_t i = 0; i < v.size(); ++ i)
			{
				v4[i] = int4(v[i].x(), v[i].y(), 0, 0);
			}

			if (!v.empty())
			{
				memcpy(int_regs_, &v4[0], std::min(reg_size_, v4.size() * sizeof(int4)));
			}
		}

	private:
		int4* int_regs_;
		size_t reg_size_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetD3D9ShaderParameter<int2*, float>
	{
	public:
		SetD3D9ShaderParameter(float4* float_regs, uint16_t reg_count, RenderEffectParameterPtr const & param)
			: float_regs_(float_regs), reg_size_(reg_count * sizeof(int4)), param_(param)
		{
		}

		void operator()()
		{
			std::vector<int2> v;
			param_->Value(v);

			std::vector<float4> v4(v.size());
			for (size_t i = 0; i < v.size(); ++ i)
			{
				v4[i] = float4(static_cast<float>(v[i].x()), static_cast<float>(v[i].y()), 0, 0);
			}

			if (!v.empty())
			{
				memcpy(float_regs_, &v4[0], std::min(reg_size_, v4.size() * sizeof(float4)));
			}
		}

	private:
		float4* float_regs_;
		size_t reg_size_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetD3D9ShaderParameter<int3*, int32_t>
	{
	public:
		SetD3D9ShaderParameter(int4* int_regs, uint16_t reg_count, RenderEffectParameterPtr const & param)
			: int_regs_(int_regs), reg_size_(reg_count * sizeof(int4)), param_(param)
		{
		}

		void operator()()
		{
			std::vector<int3> v;
			param_->Value(v);

			std::vector<int4> v4(v.size());
			for (size_t i = 0; i < v.size(); ++ i)
			{
				v4[i] = int4(v[i].x(), v[i].y(), v[i].z(), 0);
			}

			if (!v.empty())
			{
				memcpy(int_regs_, &v4[0], std::min(reg_size_, v4.size() * sizeof(int4)));
			}
		}

	private:
		int4* int_regs_;
		size_t reg_size_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetD3D9ShaderParameter<int3*, float>
	{
	public:
		SetD3D9ShaderParameter(float4* float_regs, uint16_t reg_count, RenderEffectParameterPtr const & param)
			: float_regs_(float_regs), reg_size_(reg_count * sizeof(int4)), param_(param)
		{
		}

		void operator()()
		{
			std::vector<int3> v;
			param_->Value(v);

			std::vector<float4> v4(v.size());
			for (size_t i = 0; i < v.size(); ++ i)
			{
				v4[i] = float4(static_cast<float>(v[i].x()), static_cast<float>(v[i].y()), static_cast<float>(v[i].z()), 0);
			}

			if (!v.empty())
			{
				memcpy(float_regs_, &v4[0], std::min(reg_size_, v4.size() * sizeof(float4)));
			}
		}

	private:
		float4* float_regs_;
		size_t reg_size_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetD3D9ShaderParameter<int4*, int32_t>
	{
	public:
		SetD3D9ShaderParameter(int4* int_regs, uint16_t reg_count, RenderEffectParameterPtr const & param)
			: int_regs_(int_regs), reg_size_(reg_count * sizeof(int4)), param_(param)
		{
		}

		void operator()()
		{
			std::vector<int4> v;
			param_->Value(v);

			if (!v.empty())
			{
				memcpy(int_regs_, &v[0], std::min(reg_size_, v.size() * sizeof(int4)));
			}
		}

	private:
		int4* int_regs_;
		size_t reg_size_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetD3D9ShaderParameter<int4*, float>
	{
	public:
		SetD3D9ShaderParameter(float4* float_regs, uint16_t reg_count, RenderEffectParameterPtr const & param)
			: float_regs_(float_regs), reg_size_(reg_count * sizeof(int4)), param_(param)
		{
		}

		void operator()()
		{
			std::vector<int4> v;
			param_->Value(v);

			std::vector<float4> v4(v.size());
			for (size_t i = 0; i < v.size(); ++ i)
			{
				v4[i] = v[i];
			}

			if (!v.empty())
			{
				memcpy(float_regs_, &v4[0], std::min(reg_size_, v4.size() * sizeof(float4)));
			}
		}

	private:
		float4* float_regs_;
		size_t reg_size_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetD3D9ShaderParameter<float2*, float>
	{
	public:
		SetD3D9ShaderParameter(float4* float_regs, uint16_t reg_count, RenderEffectParameterPtr const & param)
			: float_regs_(float_regs), reg_size_(reg_count * sizeof(float4)), param_(param)
		{
		}

		void operator()()
		{
			std::vector<float2> v;
			param_->Value(v);

			std::vector<float4> v4(v.size());
			for (size_t i = 0; i < v.size(); ++ i)
			{
				v4[i] = float4(v[i].x(), v[i].y(), 0, 0);
			}

			if (!v.empty())
			{
				memcpy(float_regs_, &v4[0], std::min(reg_size_, v4.size() * sizeof(float4)));
			}
		}

	private:
		float4* float_regs_;
		size_t reg_size_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetD3D9ShaderParameter<float3*, float>
	{
	public:
		SetD3D9ShaderParameter(float4* float_regs, uint16_t reg_count, RenderEffectParameterPtr const & param)
			: float_regs_(float_regs), reg_size_(reg_count * sizeof(float4)), param_(param)
		{
		}

		void operator()()
		{
			std::vector<float3> v;
			param_->Value(v);

			std::vector<float4> v4(v.size());
			for (size_t i = 0; i < v.size(); ++ i)
			{
				v4[i] = float4(v[i].x(), v[i].y(), v[i].z(), 0);
			}

			if (!v.empty())
			{
				memcpy(float_regs_, &v4[0], std::min(reg_size_, v4.size() * sizeof(float4)));
			}
		}

	private:
		float4* float_regs_;
		size_t reg_size_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetD3D9ShaderParameter<float4*, float>
	{
	public:
		SetD3D9ShaderParameter(float4* float_regs, uint16_t reg_count, RenderEffectParameterPtr const & param)
			: float_regs_(float_regs), reg_size_(reg_count * sizeof(float4)), param_(param)
		{
		}

		void operator()()
		{
			std::vector<float4> v;
			param_->Value(v);

			if (!v.empty())
			{
				memcpy(float_regs_, &v[0], std::min(reg_size_, v.size() * sizeof(float4)));
			}
		}

	private:
		float4* float_regs_;
		size_t reg_size_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetD3D9ShaderParameter<float4x4*, float>
	{
	public:
		SetD3D9ShaderParameter(float4* float_regs, size_t rows, RenderEffectParameterPtr const & param)
			: float_regs_(float_regs), rows_(rows), param_(param)
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
				memcpy(&float_regs_[start], &mat[0], rows_ * sizeof(float4));
				start += rows_;
			}
		}

	private:
		float4* float_regs_;
		size_t rows_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetD3D9ShaderParameter<std::pair<TexturePtr, SamplerStateObjectPtr>, std::pair<TexturePtr, SamplerStateObjectPtr> >
	{
	public:
		SetD3D9ShaderParameter(std::pair<TexturePtr, SamplerStateObjectPtr>& sampler,
				RenderEffectParameterPtr const & tex_param, RenderEffectParameterPtr const & sampler_param)
			: sampler_(&sampler), tex_param_(tex_param), sampler_param_(sampler_param)
		{
		}

		void operator()()
		{
			tex_param_->Value(sampler_->first);
			sampler_param_->Value(sampler_->second);
		}

	private:
		std::pair<TexturePtr, SamplerStateObjectPtr>* sampler_;
		RenderEffectParameterPtr tex_param_;
		RenderEffectParameterPtr sampler_param_;
	};
}

namespace KlayGE
{
	D3D9ShaderObject::D3D9ShaderObject()
	{
		is_shader_validate_.assign(true);
	}

	std::string D3D9ShaderObject::GenShaderText(RenderEffect const & effect) const
	{
		std::stringstream shader_ss;

		bool sample_helper = false;
		for (uint32_t i = 0; i < effect.NumShaders(); ++ i)
		{
			std::string const & s = effect.ShaderByIndex(i).str();
			boost::char_separator<char> sep("", " \t\n.,():;+-*/%&!|^[]{}'\"?");
			boost::tokenizer<boost::char_separator<char> > tok(s, sep);
			std::string this_token;
			for (BOOST_AUTO(beg, tok.begin()); beg != tok.end();)
			{
				this_token = *beg;

				RenderEffectParameterPtr const & param = effect.ParameterByName(this_token);
				if (param &&
					((REDT_texture1D == param->type()) || (REDT_texture2D == param->type()) || (REDT_texture3D == param->type())
						|| (REDT_textureCUBE == param->type())))
				{
					std::vector<std::string> sample_tokens;
					sample_tokens.push_back(this_token);
					++ beg;
					if ("." == *beg)
					{
						while (*beg != ",")
						{
							if ((*beg != " ") && (*beg != "\t") && (*beg != "\n"))
							{
								sample_tokens.push_back(*beg);
							}
							++ beg;
						}

						std::string combined_sampler_name = sample_tokens[0] + "__" + sample_tokens[4];
						bool found = false;
						for (uint32_t j = 0; j < tex_sampler_binds_.size(); ++ j)
						{
							if (tex_sampler_binds_[j].first == combined_sampler_name)
							{
								found = true;
								break;
							}
						}
						if (!found)
						{
							tex_sampler_binds_.push_back(std::make_pair(combined_sampler_name,
								std::make_pair(param, effect.ParameterByName(sample_tokens[4]))));
						}

						if ((!sample_helper) && (("SampleLevel" == sample_tokens[2]) || ("SampleBias" == sample_tokens[2])))
						{
							sample_helper = true;
						}

						switch (param->type())
						{
						case REDT_texture1D:
						case REDT_texture2D:
						case REDT_texture3D:
						case REDT_textureCUBE:
							shader_ss << "tex";

							switch (param->type())
							{
							case REDT_texture1D:
								shader_ss << "1D";
								break;

							case REDT_texture2D:
								shader_ss << "2D";
								break;

							case REDT_texture3D:
								shader_ss << "3D";
								break;

							case REDT_textureCUBE:
								shader_ss << "CUBE";
								break;
							}

							if ("SampleLevel" == sample_tokens[2])
							{
								shader_ss << "Level";
							}
							else
							{
								if ("SampleBias" == sample_tokens[2])
								{
									shader_ss << "Bias";
								}
							}

							break;
						}
						shader_ss << "(" << combined_sampler_name << ",";

						++ beg;
					}
					else
					{
						shader_ss << this_token;
					}
				}
				else
				{
					if ("SV_Position" == this_token)
					{
						shader_ss << "POSITION";
					}
					else
					{
						if ("SV_Depth" == this_token)
						{
							shader_ss << "DEPTH";
						}
						else
						{
							if (0 == this_token.find("SV_Target"))
							{
								shader_ss << "COLOR" << this_token.substr(9);
							}
							else
							{
								if ("[" == this_token)
								{
									++ beg;
									if (("branch" == *beg)
										|| ("flatten" == *beg)
										|| ("forcecase" == *beg)
										|| ("call" == *beg)
										|| ("unroll" == *beg)
										|| ("loop" == *beg))
									{
										std::string attr = *beg;
										++ beg;
										if (*beg != "]")
										{
											shader_ss << "[" << attr << *beg;
										}
									}
									else
									{
										shader_ss << "[" << *beg;
									}
								}
								else
								{
									shader_ss << this_token;
								}
							}
						}
					}

					++ beg;
				}
			}
			shader_ss << std::endl;
		}

		std::stringstream ss;
		if (sample_helper)
		{
			ss << predefined_funcs << std::endl;
		}

		for (uint32_t i = 0; i < effect.NumMacros(); ++ i)
		{
			std::pair<std::string, std::string> const & name_value = effect.MacroByIndex(i);
			ss << "#define " << name_value.first << " " << name_value.second << std::endl;
		}
		ss << std::endl;

		BOOST_AUTO(cbuffers, effect.CBuffers());
		BOOST_FOREACH(BOOST_TYPEOF(cbuffers)::const_reference cbuff, cbuffers)
		{
			BOOST_FOREACH(BOOST_TYPEOF(cbuff.second)::const_reference param_index, cbuff.second)
			{
				RenderEffectParameter& param = *effect.ParameterByIndex(param_index);

				switch (param.type())
				{
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
					ss << std::endl;
					break;

				default:
					if ((REDT_uint == param.type())
						|| (REDT_uint2 == param.type())
						|| (REDT_uint3 == param.type())
						|| (REDT_uint4 == param.type()))
					{
						ss << "int";
					}
					else
					{
						ss << effect.TypeName(param.type());
					}

					ss << " " << *param.Name();
					if (param.ArraySize())
					{
						ss << "[" << *param.ArraySize() << "]";
					}
					ss << ";" << std::endl;
					break;
				}
			}
		}

		for (uint32_t i = 0; i < tex_sampler_binds_.size(); ++ i)
		{
			ss << "sampler " << tex_sampler_binds_[i].first << ";" << std::endl;
		}

		ss << shader_ss.str();

		return ss.str();
	}

	void D3D9ShaderObject::SetShader(RenderEffect& effect, boost::shared_ptr<std::vector<uint32_t> > const & shader_desc_ids,
		uint32_t tech_index, uint32_t pass_index)
	{
		D3D9RenderEngine const & render_eng = *checked_cast<D3D9RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D9DevicePtr const & d3d_device = render_eng.D3DDevice();

		is_validate_ = true;
		for (size_t type = 0; type < ShaderObject::ST_NumShaderTypes; ++ type)
		{
			shader_desc& sd = effect.GetShaderDesc((*shader_desc_ids)[type]);
			if (sd.tech_pass != 0xFFFFFFFF)
			{
				D3D9ShaderObjectPtr so = checked_pointer_cast<D3D9ShaderObject>(effect.TechniqueByIndex(sd.tech_pass >> 16)->Pass(sd.tech_pass & 0xFFFF)->GetShaderObject());

				is_shader_validate_[type] = so->is_shader_validate_[type];
				switch (type)
				{
				case ST_VertexShader:
					vertex_shader_ = so->vertex_shader_;
					break;

				case ST_PixelShader:
					pixel_shader_ = so->pixel_shader_;
					break;

				default:
					is_shader_validate_[type] = false;
					break;
				}

				bool_start_[type] = so->bool_start_[type];
				int_start_[type] = so->int_start_[type];
				float_start_[type] = so->float_start_[type];

				bool_registers_[type].resize(so->bool_registers_[type].size());
				int_registers_[type].resize(so->int_registers_[type].size());
				float_registers_[type].resize(so->float_registers_[type].size());
				samplers_[type].resize(so->samplers_[type].size());

				param_binds_[type].reserve(so->param_binds_[type].size());
				BOOST_FOREACH(BOOST_TYPEOF(so->param_binds_[type])::const_reference pb, so->param_binds_[type])
				{
					if (pb.param)
					{
						param_binds_[type].push_back(this->GetBindFunc(pb.p_handle, effect.ParameterByName(*(pb.param->Name()))));
					}
					else
					{
						for (size_t j = 0; j < tex_sampler_binds_.size(); ++ j)
						{
							if (tex_sampler_binds_[j].first == pb.combined_sampler_name)
							{
								parameter_bind_t new_pb;
								new_pb.combined_sampler_name = pb.combined_sampler_name;
								new_pb.p_handle = pb.p_handle;
								new_pb.func = SetD3D9ShaderParameter<std::pair<TexturePtr, SamplerStateObjectPtr>,
									std::pair<TexturePtr, SamplerStateObjectPtr> >(samplers_[pb.p_handle.shader_type][pb.p_handle.register_index],
									tex_sampler_binds_[j].second.first, tex_sampler_binds_[j].second.second);
								param_binds_[type].push_back(new_pb);
								break;
							}
						}
					}
				}
			}
			else
			{
				if (!sd.profile.empty())
				{
					std::string shader_text = this->GenShaderText(effect);

					is_shader_validate_[type] = true;

					std::string shader_profile = sd.profile;
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
						is_shader_validate_[type] = false;
						break;
					}

					ID3DXConstantTable* constant_table = NULL;
					ID3DXBuffer* code = NULL;
					if (is_shader_validate_[type])
					{
						ID3DXBuffer* err_msg;
						std::vector<D3DXMACRO> macros;
						D3DXMACRO macro_d3d9 = { "KLAYGE_D3D9", "1" };
						D3DXMACRO macro_end = { NULL, NULL };
						macros.push_back(macro_d3d9);
						if (!render_eng.DeviceCaps().bc5_support)
						{
							D3DXMACRO macro_bc5_as_bc3 = { "KLAYGE_BC5_AS_AG", "1" };
							macros.push_back(macro_bc5_as_bc3);
						}
						else
						{
							D3DXMACRO macro_bc5_as_bc3 = { "KLAYGE_BC5_AS_GR", "1" };
							macros.push_back(macro_bc5_as_bc3);
						}
						macros.push_back(macro_end);
						D3DXCompileShader(shader_text.c_str(), static_cast<UINT>(shader_text.size()), &macros[0], NULL,
							sd.func_name.c_str(), shader_profile.c_str(),
							0, &code, &err_msg, &constant_table);
						if (err_msg != NULL)
						{
#ifdef D3DXSHADER_USE_LEGACY_D3DX9_31_DLL
							ID3DXConstantTable* constant_table_legacy;
							ID3DXBuffer* code_legacy;
							ID3DXBuffer* err_msg_legacy;
							D3DXCompileShader(shader_text.c_str(), static_cast<UINT>(shader_text.size()), &macros[0], NULL,
								sd.func_name.c_str(), shader_profile.c_str(),
								D3DXSHADER_USE_LEGACY_D3DX9_31_DLL, &code_legacy, &err_msg_legacy, &constant_table_legacy);
							if (err_msg_legacy != NULL)
							{
#ifdef KLAYGE_DEBUG
								std::istringstream iss(shader_text);
								std::string s;
								int line = 1;
								while (iss)
								{
									std::getline(iss, s);
									std::cerr << line << " " << s << std::endl;
									++ line;
								}
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
							std::cerr << shader_text << std::endl;
							std::cerr << static_cast<char*>(err_msg->GetBufferPointer()) << std::endl;
#endif

							err_msg->Release();
#endif
						}
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
							is_shader_validate_[type] = false;
							break;
						}

						code->Release();
					}

					RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
					switch (type)
					{
					case ST_VertexShader:
						samplers_[type].resize(re.DeviceCaps().max_vertex_texture_units);
						break;

					case ST_PixelShader:
						samplers_[type].resize(re.DeviceCaps().max_pixel_texture_units);
						break;

					default:
						is_shader_validate_[type] = false;
						break;
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
							if (p)
							{
								param_binds_[type].push_back(this->GetBindFunc(p_handle, p));
							}
							else
							{
								for (size_t i = 0; i < tex_sampler_binds_.size(); ++ i)
								{
									if (tex_sampler_binds_[i].first == constant_desc.Name)
									{
										parameter_bind_t pb;
										pb.combined_sampler_name = tex_sampler_binds_[i].first;
										pb.p_handle = p_handle;
										pb.func = SetD3D9ShaderParameter<std::pair<TexturePtr, SamplerStateObjectPtr>,
											std::pair<TexturePtr, SamplerStateObjectPtr> >(samplers_[p_handle.shader_type][p_handle.register_index],
											tex_sampler_binds_[i].second.first, tex_sampler_binds_[i].second.second);
										param_binds_[type].push_back(pb);
										break;
									}
								}
							}
						}

						constant_table->Release();
					}

					sd.tech_pass = (tech_index << 16) + pass_index;
				}
			}
		
			is_validate_ &= is_shader_validate_[type];
		}
	}

	ShaderObjectPtr D3D9ShaderObject::Clone(RenderEffect& effect)
	{
		D3D9ShaderObjectPtr ret = MakeSharedPtr<D3D9ShaderObject>();
		ret->is_validate_ = is_validate_;
		ret->is_shader_validate_ = is_shader_validate_;
		ret->vertex_shader_ = vertex_shader_;
		ret->pixel_shader_ = pixel_shader_;
		ret->bool_start_ = bool_start_;
		ret->int_start_ = int_start_;
		ret->float_start_ = float_start_;
		ret->tex_sampler_binds_.resize(tex_sampler_binds_.size());
		for (size_t i = 0; i < tex_sampler_binds_.size(); ++ i)
		{
			ret->tex_sampler_binds_[i].first = tex_sampler_binds_[i].first;
			ret->tex_sampler_binds_[i].second.first = effect.ParameterByName(*(tex_sampler_binds_[i].second.first->Name()));
			ret->tex_sampler_binds_[i].second.second = effect.ParameterByName(*(tex_sampler_binds_[i].second.second->Name()));
		}
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			ret->bool_registers_[i].resize(bool_registers_[i].size());
			ret->int_registers_[i].resize(int_registers_[i].size());
			ret->float_registers_[i].resize(float_registers_[i].size());
			ret->samplers_[i].resize(samplers_[i].size());

			ret->param_binds_[i].reserve(param_binds_[i].size());
			BOOST_FOREACH(BOOST_TYPEOF(param_binds_[i])::const_reference pb, param_binds_[i])
			{
				if (pb.param)
				{
					ret->param_binds_[i].push_back(ret->GetBindFunc(pb.p_handle, effect.ParameterByName(*(pb.param->Name()))));
				}
				else
				{
					for (size_t j = 0; j < ret->tex_sampler_binds_.size(); ++ j)
					{
						if (ret->tex_sampler_binds_[j].first == pb.combined_sampler_name)
						{
							parameter_bind_t new_pb;
							new_pb.combined_sampler_name = pb.combined_sampler_name;
							new_pb.p_handle = pb.p_handle;
							new_pb.func = SetD3D9ShaderParameter<std::pair<TexturePtr, SamplerStateObjectPtr>,
								std::pair<TexturePtr, SamplerStateObjectPtr> >(ret->samplers_[pb.p_handle.shader_type][pb.p_handle.register_index],
								ret->tex_sampler_binds_[j].second.first, ret->tex_sampler_binds_[j].second.second);
							ret->param_binds_[i].push_back(new_pb);
							break;
						}
					}
				}
			}
		}

		return ret;
	}

	D3D9ShaderObject::parameter_bind_t D3D9ShaderObject::GetBindFunc(D3D9ShaderParameterHandle const & p_handle, RenderEffectParameterPtr const & param)
	{
		parameter_bind_t ret;
		ret.param = param;
		ret.p_handle = p_handle;

		switch (param->type())
		{
		case REDT_bool:
			if (param->ArraySize())
			{
				switch (p_handle.register_set)
				{
				case D3DXRS_BOOL:
					ret.func = SetD3D9ShaderParameter<bool*, bool>(&bool_registers_[p_handle.shader_type][p_handle.register_index], param);
					break;

				case D3DXRS_INT4:
					ret.func = SetD3D9ShaderParameter<bool*, int32_t>(&int_registers_[p_handle.shader_type][p_handle.register_index], param);
					break;

				case D3DXRS_FLOAT4:
					ret.func = SetD3D9ShaderParameter<bool*, float>(&float_registers_[p_handle.shader_type][p_handle.register_index], param);
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
					ret.func = SetD3D9ShaderParameter<bool, bool>(bool_registers_[p_handle.shader_type][p_handle.register_index].x(), param);
					break;

				case D3DXRS_INT4:
					ret.func = SetD3D9ShaderParameter<bool, int32_t>(int_registers_[p_handle.shader_type][p_handle.register_index].x(), param);
					break;

				case D3DXRS_FLOAT4:
					ret.func = SetD3D9ShaderParameter<bool, float>(float_registers_[p_handle.shader_type][p_handle.register_index].x(), param);
					break;

				default:
					BOOST_ASSERT(false);
					break;
				}
			}
			break;

		case REDT_uint:
		case REDT_int:
			if (param->ArraySize())
			{
				switch (p_handle.register_set)
				{
				case D3DXRS_BOOL:
					ret.func = SetD3D9ShaderParameter<int32_t*, bool>(&bool_registers_[p_handle.shader_type][p_handle.register_index], param);
					break;

				case D3DXRS_INT4:
					ret.func = SetD3D9ShaderParameter<int32_t*, int32_t>(&int_registers_[p_handle.shader_type][p_handle.register_index], param);
					break;

				case D3DXRS_FLOAT4:
					ret.func = SetD3D9ShaderParameter<int32_t*, float>(&float_registers_[p_handle.shader_type][p_handle.register_index], param);
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
					ret.func = SetD3D9ShaderParameter<int32_t, bool>(bool_registers_[p_handle.shader_type][p_handle.register_index].x(), param);
					break;

				case D3DXRS_INT4:
					ret.func = SetD3D9ShaderParameter<int32_t, int32_t>(int_registers_[p_handle.shader_type][p_handle.register_index].x(), param);
					break;

				case D3DXRS_FLOAT4:
					ret.func = SetD3D9ShaderParameter<int32_t, float>(float_registers_[p_handle.shader_type][p_handle.register_index].x(), param);
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
				ret.func = SetD3D9ShaderParameter<float*, float>(&float_registers_[p_handle.shader_type][p_handle.register_index], param);
			}
			else
			{
				ret.func = SetD3D9ShaderParameter<float, float>(float_registers_[p_handle.shader_type][p_handle.register_index].x(), param);
			}
			break;

		case REDT_uint2:
		case REDT_int2:
			if (param->ArraySize())
			{
				switch (p_handle.register_set)
				{
				case D3DXRS_INT4:
					ret.func = SetD3D9ShaderParameter<int2*, int32_t>(&int_registers_[p_handle.shader_type][p_handle.register_index], p_handle.register_count, param);
					break;

				case D3DXRS_FLOAT4:
					ret.func = SetD3D9ShaderParameter<int2*, float>(&float_registers_[p_handle.shader_type][p_handle.register_index], p_handle.register_count, param);
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
				case D3DXRS_INT4:
					ret.func = SetD3D9ShaderParameter<int2, int32_t>(int_registers_[p_handle.shader_type][p_handle.register_index], param);
					break;

				case D3DXRS_FLOAT4:
					ret.func = SetD3D9ShaderParameter<int2, float>(float_registers_[p_handle.shader_type][p_handle.register_index], param);
					break;

				default:
					BOOST_ASSERT(false);
					break;
				}
			}
			break;

		case REDT_uint3:
		case REDT_int3:
			if (param->ArraySize())
			{
				switch (p_handle.register_set)
				{
				case D3DXRS_INT4:
					ret.func = SetD3D9ShaderParameter<int3*, int32_t>(&int_registers_[p_handle.shader_type][p_handle.register_index], p_handle.register_count, param);
					break;

				case D3DXRS_FLOAT4:
					ret.func = SetD3D9ShaderParameter<int3*, float>(&float_registers_[p_handle.shader_type][p_handle.register_index], p_handle.register_count, param);
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
				case D3DXRS_INT4:
					ret.func = SetD3D9ShaderParameter<int3, int32_t>(int_registers_[p_handle.shader_type][p_handle.register_index], param);
					break;

				case D3DXRS_FLOAT4:
					ret.func = SetD3D9ShaderParameter<int3, float>(float_registers_[p_handle.shader_type][p_handle.register_index], param);
					break;

				default:
					BOOST_ASSERT(false);
					break;
				}
			}
			break;

		case REDT_uint4:
		case REDT_int4:
			if (param->ArraySize())
			{
				switch (p_handle.register_set)
				{
				case D3DXRS_INT4:
					ret.func = SetD3D9ShaderParameter<int4*, int32_t>(&int_registers_[p_handle.shader_type][p_handle.register_index], p_handle.register_count, param);
					break;

				case D3DXRS_FLOAT4:
					ret.func = SetD3D9ShaderParameter<int4*, float>(&float_registers_[p_handle.shader_type][p_handle.register_index], p_handle.register_count, param);
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
				case D3DXRS_INT4:
					ret.func = SetD3D9ShaderParameter<int4, int32_t>(int_registers_[p_handle.shader_type][p_handle.register_index], param);
					break;

				case D3DXRS_FLOAT4:
					ret.func = SetD3D9ShaderParameter<int4, float>(float_registers_[p_handle.shader_type][p_handle.register_index], param);
					break;

				default:
					BOOST_ASSERT(false);
					break;
				}
			}
			break;

		case REDT_float2:
			if (param->ArraySize())
			{
				ret.func = SetD3D9ShaderParameter<float2*, float>(&float_registers_[p_handle.shader_type][p_handle.register_index], p_handle.register_count, param);
			}
			else
			{
				ret.func = SetD3D9ShaderParameter<float2, float>(float_registers_[p_handle.shader_type][p_handle.register_index], param);
			}
			break;

		case REDT_float3:
			if (param->ArraySize())
			{
				ret.func = SetD3D9ShaderParameter<float3*, float>(&float_registers_[p_handle.shader_type][p_handle.register_index], p_handle.register_count, param);
			}
			else
			{
				ret.func = SetD3D9ShaderParameter<float3, float>(float_registers_[p_handle.shader_type][p_handle.register_index], param);
			}
			break;

		case REDT_float4:
			if (param->ArraySize())
			{
				ret.func = SetD3D9ShaderParameter<float4*, float>(&float_registers_[p_handle.shader_type][p_handle.register_index], p_handle.register_count, param);
			}
			else
			{
				ret.func = SetD3D9ShaderParameter<float4, float>(float_registers_[p_handle.shader_type][p_handle.register_index], param);
			}
			break;

		case REDT_float4x4:
			if (param->ArraySize())
			{
				ret.func = SetD3D9ShaderParameter<float4x4*, float>(&float_registers_[p_handle.shader_type][p_handle.register_index], p_handle.rows, param);
			}
			else
			{
				BOOST_ASSERT(D3DXRS_FLOAT4 == p_handle.register_set);
				ret.func = SetD3D9ShaderParameter<float4x4, float>(&float_registers_[p_handle.shader_type][p_handle.register_index], p_handle.register_count, param);
			}
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}

		return ret;
	}

	void D3D9ShaderObject::Bind()
	{
		D3D9RenderEngine& re = *checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		re.SetVertexShader(vertex_shader_.get());
		re.SetPixelShader(pixel_shader_.get());

		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			ShaderType type = static_cast<ShaderType>(i);

			BOOST_FOREACH(BOOST_TYPEOF(param_binds_[i])::reference pb, param_binds_[i])
			{
				pb.func();
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
					re.SetVertexShaderConstantI(int_start_[type], reinterpret_cast<int*>(&int_registers_[type][0].x()),
						static_cast<uint32_t>(int_registers_[type].size()));
				}
				else
				{
					re.SetPixelShaderConstantI(int_start_[type], reinterpret_cast<int*>(&int_registers_[type][0].x()),
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

				std::pair<TexturePtr, SamplerStateObjectPtr> const & sampler = samplers_[type][j];
				if (!sampler.first || !sampler.second)
				{
					re.SetTexture(stage, NULL);
				}
				else
				{
					checked_pointer_cast<D3D9SamplerStateObject>(sampler.second)->Active(stage, sampler.first);
				}
			}
		}
	}

	void D3D9ShaderObject::Unbind()
	{
	}
}
