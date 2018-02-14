// OGLShaderObject.cpp
// KlayGE OpenGL shader对象类 实现文件
// Ver 3.12.0
// 版权所有(C) 龚敏敏, 2006-2010
// Homepage: http://www.klayge.org
//
// 3.11.0
// Geometry shader to GLSL compiler works (2010.8.16)
// Reuse generated GLSL between passes (2010.9.30)
//
// 3.9.0
// Cg载入后编译成GLSL使用 (2009.4.26)
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
#include <KFL/ErrorHandling.hpp>
#include <KFL/Util.hpp>
#include <KFL/ResIdentifier.hpp>
#include <KlayGE/Context.hpp>
#include <KFL/Math.hpp>
#include <KFL/Matrix.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KFL/Hash.hpp>

#include <cstdio>
#include <string>
#include <algorithm>
#include <sstream>
#include <cstring>
#include <boost/assert.hpp>
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable" // Ignore unused variable (mpl_assertion_in_line_xxx) in boost
#endif
#include <boost/lexical_cast.hpp>
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic pop
#endif

#include <glloader/glloader.h>

#include <DXBC2GLSL/DXBC2GLSL.hpp>

#ifndef D3DCOMPILE_SKIP_OPTIMIZATION
#define D3DCOMPILE_SKIP_OPTIMIZATION 0x00000004
#endif
#ifndef D3DCOMPILE_PREFER_FLOW_CONTROL
#define D3DCOMPILE_PREFER_FLOW_CONTROL 0x00000400
#endif
#ifndef D3DCOMPILE_ENABLE_STRICTNESS
#define D3DCOMPILE_ENABLE_STRICTNESS 0x00000800
#endif

#include <KlayGE/OpenGL/OGLRenderFactory.hpp>
#include <KlayGE/OpenGL/OGLRenderEngine.hpp>
#include <KlayGE/OpenGL/OGLMapping.hpp>
#include <KlayGE/OpenGL/OGLTexture.hpp>
#include <KlayGE/OpenGL/OGLRenderStateObject.hpp>
#include <KlayGE/OpenGL/OGLGraphicsBuffer.hpp>
#include <KlayGE/OpenGL/OGLShaderObject.hpp>

namespace
{
	using namespace KlayGE;

	template <typename SrcType>
	class SetOGLShaderParameter
	{
	};

	template <>
	class SetOGLShaderParameter<GraphicsBufferPtr>
	{
	public:
		SetOGLShaderParameter(std::vector<TextureBind>& buffers,
					std::vector<GLuint>& gl_bind_targets, std::vector<GLuint>& gl_bind_textures, std::vector<GLuint>& gl_bind_samplers,
					GLint location, GLuint stage,
					RenderEffectParameter* buff_param)
			: buffers_(&buffers),
				gl_bind_targets_(&gl_bind_targets), gl_bind_textures_(&gl_bind_textures), gl_bind_samplers_(&gl_bind_samplers),
				location_(location), stage_(stage), buff_param_(buff_param)
		{
		}

		void operator()()
		{
			buff_param_->Value((*buffers_)[stage_].tex_buff);

			if ((*buffers_)[stage_].tex_buff)
			{
				(*gl_bind_targets_)[stage_] = GL_TEXTURE_BUFFER;
				(*gl_bind_textures_)[stage_] = checked_cast<OGLGraphicsBuffer*>((*buffers_)[stage_].tex_buff.get())->GLtex();
			}
			else
			{
				(*gl_bind_targets_)[stage_] = GL_TEXTURE_BUFFER;
				(*gl_bind_textures_)[stage_] = 0;
			}
			(*gl_bind_samplers_)[stage_] = 0;

			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.Uniform1i(location_, stage_);
		}

	private:
		std::vector<TextureBind>* buffers_;
		std::vector<GLuint>* gl_bind_targets_;
		std::vector<GLuint>* gl_bind_textures_;
		std::vector<GLuint>* gl_bind_samplers_;
		GLint location_;
		GLuint stage_;
		RenderEffectParameter* buff_param_;
	};

	template <>
	class SetOGLShaderParameter<TexturePtr>
	{
	public:
		SetOGLShaderParameter(std::vector<TextureBind>& samplers,
					std::vector<GLuint>& gl_bind_targets, std::vector<GLuint>& gl_bind_textures, std::vector<GLuint>& gl_bind_samplers,
					GLint location, GLuint stage,
					RenderEffectParameter* tex_param, RenderEffectParameter* sampler_param)
			: samplers_(&samplers),
				gl_bind_targets_(&gl_bind_targets), gl_bind_textures_(&gl_bind_textures), gl_bind_samplers_(&gl_bind_samplers),
				location_(location), stage_(stage), tex_param_(tex_param), sampler_param_(sampler_param)
		{
		}

		void operator()()
		{
			tex_param_->Value((*samplers_)[stage_].tex);
			sampler_param_->Value((*samplers_)[stage_].sampler);

			if ((*samplers_)[stage_].tex)
			{
				auto gl_tex = checked_cast<OGLTexture*>((*samplers_)[stage_].tex.get());
				auto gl_sampler = checked_cast<OGLSamplerStateObject*>((*samplers_)[stage_].sampler.get());

				(*gl_bind_targets_)[stage_] = gl_tex->GLType();
				(*gl_bind_textures_)[stage_] = gl_tex->GLTexture();
				(*gl_bind_samplers_)[stage_] = gl_sampler->GLSampler();
			}
			else
			{
				(*gl_bind_targets_)[stage_] = GL_TEXTURE_2D;
				(*gl_bind_textures_)[stage_] = 0;
				(*gl_bind_samplers_)[stage_] = 0;
			}

			OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			re.Uniform1i(location_, stage_);
		}

	private:
		std::vector<TextureBind>* samplers_;
		std::vector<GLuint>* gl_bind_targets_;
		std::vector<GLuint>* gl_bind_textures_;
		std::vector<GLuint>* gl_bind_samplers_;
		GLint location_;
		GLuint stage_;
		RenderEffectParameter* tex_param_;
		RenderEffectParameter* sampler_param_;
	};
}

namespace KlayGE
{
	OGLShaderObject::OGLShaderObjectTemplate::OGLShaderObjectTemplate()
		: gs_input_type_(0), gs_output_type_(0), gs_max_output_vertex_(0),
			ds_partitioning_(STP_Undefined), ds_output_primitive_(STOP_Undefined)
	{
	}

	OGLShaderObject::OGLShaderObject()
		: OGLShaderObject(MakeSharedPtr<OGLShaderObjectTemplate>())
	{
	}

	OGLShaderObject::OGLShaderObject(std::shared_ptr<OGLShaderObjectTemplate> const & so_template)
		: so_template_(so_template)
	{
		has_discard_ = false;
		has_tessellation_ = false;
		is_shader_validate_.fill(true);

		glsl_program_ = glCreateProgram();
	}

	OGLShaderObject::~OGLShaderObject()
	{
		glDeleteProgram(glsl_program_);
	}

	bool OGLShaderObject::AttachNativeShader(ShaderType type, RenderEffect const & effect,
		std::array<uint32_t, ST_NumShaderTypes> const & shader_desc_ids, std::vector<uint8_t> const & native_shader_block)
	{
		bool ret = false;

		auto const & sd = effect.GetShaderDesc(shader_desc_ids[type]);

		so_template_->shader_func_names_[type] = sd.func_name;

		is_shader_validate_[type] = false;
		if (native_shader_block.size() >= 24)
		{
			uint8_t const * nsbp = &native_shader_block[0];
			
			is_shader_validate_[type] = true;

			uint32_t len32;
			std::memcpy(&len32, nsbp, sizeof(len32));
			nsbp += sizeof(len32);
			len32 = LE2Native(len32);
			so_template_->glsl_srcs_[type] = MakeSharedPtr<std::string>(len32, '\0');
			std::memcpy(&(*so_template_->glsl_srcs_[type])[0], nsbp, len32);
			nsbp += len32;

			uint16_t num16;
			std::memcpy(&num16, nsbp, sizeof(num16));
			nsbp += sizeof(num16);
			num16 = LE2Native(num16);
			so_template_->pnames_[type] = MakeSharedPtr<std::vector<std::string>>(num16);
			for (size_t i = 0; i < num16; ++ i)
			{
				uint8_t len8;
				std::memcpy(&len8, nsbp, sizeof(len8));
				nsbp += sizeof(len8);

				(*so_template_->pnames_[type])[i].resize(len8);
				std::memcpy(&(*so_template_->pnames_[type])[i][0], nsbp, len8);
				nsbp += len8;
			}

			std::memcpy(&num16, nsbp, sizeof(num16));
			nsbp += sizeof(num16);
			num16 = LE2Native(num16);
			so_template_->glsl_res_names_[type] = MakeSharedPtr<std::vector<std::string>>(num16);
			for (size_t i = 0; i < num16; ++ i)
			{
				uint8_t len8;
				std::memcpy(&len8, nsbp, sizeof(len8));
				nsbp += sizeof(len8);

				(*so_template_->glsl_res_names_[type])[i].resize(len8);
				std::memcpy(&(*so_template_->glsl_res_names_[type])[i][0], nsbp, len8);
				nsbp += len8;
			}

			std::memcpy(&num16, nsbp, sizeof(num16));
			nsbp += sizeof(num16);
			num16 = LE2Native(num16);
			for (size_t i = 0; i < num16; ++ i)
			{
				uint8_t len8;
				std::memcpy(&len8, nsbp, sizeof(len8));
				nsbp += sizeof(len8);

				std::string tex_name;
				tex_name.resize(len8);
				std::memcpy(&tex_name[0], nsbp, len8);
				nsbp += len8;

				std::memcpy(&len8, nsbp, sizeof(len8));
				nsbp += sizeof(len8);

				std::string sampler_name;
				sampler_name.resize(len8);
				std::memcpy(&sampler_name[0], nsbp, len8);
				nsbp += len8;

				std::string combined_sampler_name = tex_name + "_" + sampler_name;

				bool found = false;
				for (uint32_t k = 0; k < tex_sampler_binds_.size(); ++ k)
				{
					if (std::get<0>(tex_sampler_binds_[k]) == combined_sampler_name)
					{
						std::get<3>(tex_sampler_binds_[k]) |= 1UL << type;
						found = true;
						break;
					}
				}
				if (!found)
				{
					tex_sampler_binds_.push_back(std::make_tuple(combined_sampler_name,
						effect.ParameterByName(tex_name), effect.ParameterByName(sampler_name), 1UL << type));
				}
			}

			if (ST_VertexShader == type)
			{
				uint8_t num8;
				std::memcpy(&num8, nsbp, sizeof(num8));
				nsbp += sizeof(num8);
				so_template_->vs_usages_.resize(num8);
				for (size_t i = 0; i < num8; ++ i)
				{
					uint8_t veu;
					std::memcpy(&veu, nsbp, sizeof(veu));
					nsbp += sizeof(veu);

					so_template_->vs_usages_[i] = static_cast<VertexElementUsage>(veu);
				}

				std::memcpy(&num8, nsbp, sizeof(num8));
				nsbp += sizeof(num8);
				if (num8 > 0)
				{
					so_template_->vs_usage_indices_.resize(num8);
					std::memcpy(&so_template_->vs_usage_indices_[0], nsbp, num8 * sizeof(so_template_->vs_usage_indices_[0]));
					nsbp += num8 * sizeof(so_template_->vs_usage_indices_[0]);
				}

				std::memcpy(&num8, nsbp, sizeof(num8));
				nsbp += sizeof(num8);
				so_template_->glsl_vs_attrib_names_.resize(num8);
				for (size_t i = 0; i < num8; ++ i)
				{
					uint8_t len8;
					std::memcpy(&len8, nsbp, sizeof(len8));
					nsbp += sizeof(len8);

					so_template_->glsl_vs_attrib_names_[i].resize(len8);
					std::memcpy(&so_template_->glsl_vs_attrib_names_[i][0], nsbp, len8);
					nsbp += len8;
				}
			}
			else if (ST_GeometryShader == type)
			{
				std::memcpy(&so_template_->gs_input_type_, nsbp, sizeof(so_template_->gs_input_type_));
				nsbp += sizeof(so_template_->gs_input_type_);
				so_template_->gs_input_type_ = LE2Native(so_template_->gs_input_type_);

				std::memcpy(&so_template_->gs_output_type_, nsbp, sizeof(so_template_->gs_output_type_));
				nsbp += sizeof(so_template_->gs_output_type_);
				so_template_->gs_output_type_ = LE2Native(so_template_->gs_output_type_);

				std::memcpy(&so_template_->gs_max_output_vertex_, nsbp, sizeof(so_template_->gs_max_output_vertex_));
				nsbp += sizeof(so_template_->gs_max_output_vertex_);
				so_template_->gs_max_output_vertex_ = LE2Native(so_template_->gs_max_output_vertex_);
			}

			this->FillTFBVaryings(sd);
			this->AttachGLSL(type);

			ret = is_shader_validate_[type];
		}

		return ret;
	}

	bool OGLShaderObject::StreamIn(ResIdentifierPtr const & res, ShaderType type, RenderEffect const & effect,
		std::array<uint32_t, ST_NumShaderTypes> const & shader_desc_ids)
	{
		uint32_t len;
		res->read(&len, sizeof(len));
		len = LE2Native(len);
		std::vector<uint8_t> native_shader_block(len);
		if (len > 0)
		{
			res->read(&native_shader_block[0], len * sizeof(native_shader_block[0]));
		}

		return this->AttachNativeShader(type, effect, shader_desc_ids, native_shader_block);
	}

	void OGLShaderObject::StreamOut(std::ostream& os, ShaderType type)
	{
		std::vector<uint8_t> native_shader_block;

		if (so_template_->glsl_srcs_[type])
		{
			std::ostringstream oss(std::ios_base::binary | std::ios_base::out);

			uint32_t len32 = Native2LE(static_cast<uint32_t>(so_template_->glsl_srcs_[type]->size()));
			oss.write(reinterpret_cast<char const *>(&len32), sizeof(len32));
			oss.write(&(*so_template_->glsl_srcs_[type])[0], so_template_->glsl_srcs_[type]->size());

			uint16_t num16 = Native2LE(static_cast<uint16_t>(so_template_->pnames_[type]->size()));
			oss.write(reinterpret_cast<char const *>(&num16), sizeof(num16));
			for (size_t i = 0; i < so_template_->pnames_[type]->size(); ++ i)
			{
				uint8_t len8 = static_cast<uint8_t>((*so_template_->pnames_[type])[i].size());
				oss.write(reinterpret_cast<char const *>(&len8), sizeof(len8));
				oss.write(&(*so_template_->pnames_[type])[i][0], (*so_template_->pnames_[type])[i].size());
			}

			num16 = Native2LE(static_cast<uint16_t>(so_template_->glsl_res_names_[type]->size()));
			oss.write(reinterpret_cast<char const *>(&num16), sizeof(num16));
			for (size_t i = 0; i < so_template_->glsl_res_names_[type]->size(); ++ i)
			{
				uint8_t len8 = static_cast<uint8_t>((*so_template_->glsl_res_names_[type])[i].size());
				oss.write(reinterpret_cast<char const *>(&len8), sizeof(len8));
				oss.write(&(*so_template_->glsl_res_names_[type])[i][0], (*so_template_->glsl_res_names_[type])[i].size());
			}

			std::vector<std::pair<std::string, std::string>> tex_sampler_pairs;
			for (size_t i = 0; i < tex_sampler_binds_.size(); ++ i)
			{
				if (std::get<3>(tex_sampler_binds_[i]) | (1UL << type))
				{
					tex_sampler_pairs.emplace_back(std::get<1>(tex_sampler_binds_[i])->Name(),
						std::get<2>(tex_sampler_binds_[i])->Name());
				}
			}

			num16 = Native2LE(static_cast<uint16_t>(tex_sampler_pairs.size()));
			oss.write(reinterpret_cast<char const *>(&num16), sizeof(num16));
			for (size_t i = 0; i < num16; ++ i)
			{
				uint8_t len8 = static_cast<uint8_t>(tex_sampler_pairs[i].first.size());
				oss.write(reinterpret_cast<char const *>(&len8), sizeof(len8));
				oss.write(&tex_sampler_pairs[i].first[0], len8);

				len8 = static_cast<uint8_t>(tex_sampler_pairs[i].second.size());
				oss.write(reinterpret_cast<char const *>(&len8), sizeof(len8));
				oss.write(&tex_sampler_pairs[i].second[0], len8);
			}

			if (ST_VertexShader == type)
			{
				uint8_t num8 = static_cast<uint8_t>(so_template_->vs_usages_.size());
				oss.write(reinterpret_cast<char const *>(&num8), sizeof(num8));
				for (size_t i = 0; i < so_template_->vs_usages_.size(); ++ i)
				{
					uint8_t veu = static_cast<uint8_t>(so_template_->vs_usages_[i]);
					oss.write(reinterpret_cast<char const *>(&veu), sizeof(veu));
				}

				num8 = static_cast<uint8_t>(so_template_->vs_usage_indices_.size());
				oss.write(reinterpret_cast<char const *>(&num8), sizeof(num8));
				if (!so_template_->vs_usage_indices_.empty())
				{
					oss.write(reinterpret_cast<char const *>(&so_template_->vs_usage_indices_[0]),
						so_template_->vs_usage_indices_.size() * sizeof(so_template_->vs_usage_indices_[0]));
				}

				num8 = static_cast<uint8_t>(so_template_->glsl_vs_attrib_names_.size());
				oss.write(reinterpret_cast<char const *>(&num8), sizeof(num8));
				for (size_t i = 0; i < so_template_->glsl_vs_attrib_names_.size(); ++ i)
				{
					uint8_t len8 = static_cast<uint8_t>(so_template_->glsl_vs_attrib_names_[i].size());
					oss.write(reinterpret_cast<char const *>(&len8), sizeof(len8));
					oss.write(&so_template_->glsl_vs_attrib_names_[i][0], so_template_->glsl_vs_attrib_names_[i].size());
				}
			}
			else if (ST_GeometryShader == type)
			{
				uint32_t git = Native2LE(so_template_->gs_input_type_);
				oss.write(reinterpret_cast<char const *>(&git), sizeof(git));

				uint32_t got = Native2LE(so_template_->gs_output_type_);
				oss.write(reinterpret_cast<char const *>(&got), sizeof(got));

				uint32_t gmov = Native2LE(so_template_->gs_max_output_vertex_);
				oss.write(reinterpret_cast<char const *>(&gmov), sizeof(gmov));
			}

			std::string out_str = oss.str();
			native_shader_block.resize(out_str.size());
			std::memcpy(&native_shader_block[0], &out_str[0], out_str.size());
		}

		uint32_t len = static_cast<uint32_t>(native_shader_block.size());
		{
			uint32_t tmp = Native2LE(len);
			os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
		}
		if (len > 0)
		{
			os.write(reinterpret_cast<char const *>(&native_shader_block[0]), len * sizeof(native_shader_block[0]));
		}
	}

	void OGLShaderObject::AttachShader(ShaderType type, RenderEffect const & effect,
			RenderTechnique const & tech, RenderPass const & pass, std::array<uint32_t, ST_NumShaderTypes> const & shader_desc_ids)
	{
		ShaderDesc const & sd = effect.GetShaderDesc(shader_desc_ids[type]);

		so_template_->shader_func_names_[type] = sd.func_name;

		bool has_gs = false;
		if (!effect.GetShaderDesc(shader_desc_ids[ST_GeometryShader]).func_name.empty())
		{
			has_gs = true;
		}
		bool has_ps = false;
		if (!effect.GetShaderDesc(shader_desc_ids[ST_PixelShader]).func_name.empty())
		{
			has_ps = true;
		}

		is_shader_validate_[type] = true;
		switch (type)
		{
		case ST_VertexShader:
		case ST_PixelShader:
		case ST_GeometryShader:
		case ST_HullShader:
		case ST_DomainShader:
			break;

		default:
			is_shader_validate_[type] = false;
			break;
		}

		if (is_shader_validate_[type])
		{
			auto const & re = *checked_cast<OGLRenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			auto const & caps = re.DeviceCaps();

			is_shader_validate_[type] = true;

			char const * shader_profile = sd.profile.c_str();
			size_t const shader_profile_hash = RT_HASH(shader_profile);
			switch (type)
			{
			case ST_VertexShader:
				if (CT_HASH("auto") == shader_profile_hash)
				{
					shader_profile = "vs_5_0";
				}
				break;

			case ST_PixelShader:
				if (CT_HASH("auto") == shader_profile_hash)
				{
					shader_profile = "ps_5_0";
				}
				break;

			case ST_GeometryShader:
				if (caps.gs_support)
				{
					if (CT_HASH("auto") == shader_profile_hash)
					{
						shader_profile = "gs_5_0";
					}
				}
				else
				{
					is_shader_validate_[type] = false;
				}
				break;

			case ST_ComputeShader:
				if (caps.cs_support)
				{
					if (CT_HASH("auto") == shader_profile_hash)
					{
						shader_profile = "cs_5_0";
					}
					if ((CT_HASH("cs_5_0") == shader_profile_hash) && (caps.max_shader_model < ShaderModel(5, 0)))
					{
						is_shader_validate_[type] = false;
					}
				}
				else
				{
					is_shader_validate_[type] = false;
				}
				break;

			case ST_HullShader:
				if (caps.hs_support)
				{
					if (CT_HASH("auto") == shader_profile_hash)
					{
						shader_profile = "hs_5_0";
					}
				}
				else
				{
					is_shader_validate_[type] = false;
				}
				break;

			case ST_DomainShader:
				if (caps.ds_support)
				{
					if (CT_HASH("auto") == shader_profile_hash)
					{
						shader_profile = "ds_5_0";
					}
				}
				else
				{
					is_shader_validate_[type] = false;
				}
				break;

			default:
				is_shader_validate_[type] = false;
				break;
			}

			std::vector<uint8_t> code;
			if (is_shader_validate_[type])
			{
				std::string err_msg;
				std::vector<std::pair<char const *, char const *>> macros;
				macros.emplace_back("KLAYGE_DXBC2GLSL", "1");
				macros.emplace_back("KLAYGE_OPENGL", "1");
				if (!caps.texture_format_support(EF_BC5) || !caps.texture_format_support(EF_BC5_SRGB))
				{
					macros.emplace_back("KLAYGE_BC5_AS_AG", "1");
				}
				if (!caps.texture_format_support(EF_BC4) || !caps.texture_format_support(EF_BC4_SRGB))
				{
					macros.emplace_back("KLAYGE_BC4_AS_G", "1");
				}
				macros.emplace_back("KLAYGE_FRAG_DEPTH", "1");

				uint32_t const flags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_PREFER_FLOW_CONTROL | D3DCOMPILE_SKIP_OPTIMIZATION;
				code = this->CompileToDXBC(type, effect, tech, pass, macros, sd.func_name.c_str(), shader_profile, flags);
				if (code.empty())
				{
					is_shader_validate_[type] = false;
				}
				else
				{
					try
					{
						GLSLVersion gsv;
						if (glloader_GL_VERSION_4_6())
						{
							gsv = GSV_460;
						}
						else if (glloader_GL_VERSION_4_5())
						{
							gsv = GSV_450;
						}
						else if (glloader_GL_VERSION_4_4())
						{
							gsv = GSV_440;
						}
						else if (glloader_GL_VERSION_4_3())
						{
							gsv = GSV_430;
						}
						else if (glloader_GL_VERSION_4_2())
						{
							gsv = GSV_420;
						}
						else //if (glloader_GL_VERSION_4_1())
						{
							gsv = GSV_410;
						}

						DXBC2GLSL::DXBC2GLSL dxbc2glsl;
						uint32_t rules = DXBC2GLSL::DXBC2GLSL::DefaultRules(gsv);
						rules &= ~GSR_UniformBlockBinding;
						dxbc2glsl.FeedDXBC(&code[0],
							has_gs, has_ps, static_cast<ShaderTessellatorPartitioning>(so_template_->ds_partitioning_),
							static_cast<ShaderTessellatorOutputPrimitive>(so_template_->ds_output_primitive_),
							gsv, rules);
						so_template_->glsl_srcs_[type] = MakeSharedPtr<std::string>(dxbc2glsl.GLSLString());
						so_template_->pnames_[type] = MakeSharedPtr<std::vector<std::string>>();
						so_template_->glsl_res_names_[type] = MakeSharedPtr<std::vector<std::string>>();

						for (uint32_t i = 0; i < dxbc2glsl.NumCBuffers(); ++ i)
						{
							for (uint32_t j = 0; j < dxbc2glsl.NumVariables(i); ++ j)
							{
								if (dxbc2glsl.VariableUsed(i, j))
								{
									so_template_->pnames_[type]->push_back(dxbc2glsl.VariableName(i, j));
									so_template_->glsl_res_names_[type]->push_back(dxbc2glsl.VariableName(i, j));
								}
							}
						}

						std::vector<char const *> tex_names;
						std::vector<char const *> sampler_names;
						for (uint32_t i = 0; i < dxbc2glsl.NumResources(); ++ i)
						{
							if (dxbc2glsl.ResourceUsed(i))
							{
								char const * res_name = dxbc2glsl.ResourceName(i);

								if (SIT_TEXTURE == dxbc2glsl.ResourceType(i))
								{
									if (SSD_BUFFER == dxbc2glsl.ResourceDimension(i))
									{
										so_template_->pnames_[type]->push_back(res_name);
										so_template_->glsl_res_names_[type]->push_back(res_name);
									}
									else
									{
										tex_names.push_back(res_name);
									}
								}
								else if (SIT_SAMPLER == dxbc2glsl.ResourceType(i))
								{
									sampler_names.push_back(res_name);
								}
							}
						}

						for (size_t i = 0; i < tex_names.size(); ++ i)
						{
							RenderEffectParameter* param = effect.ParameterByName(tex_names[i]);
							for (size_t j = 0; j < sampler_names.size(); ++ j)
							{
								std::string combined_sampler_name = std::string(tex_names[i]) + "_" + sampler_names[j];
								bool found = false;
								for (uint32_t k = 0; k < tex_sampler_binds_.size(); ++ k)
								{
									if (std::get<0>(tex_sampler_binds_[k]) == combined_sampler_name)
									{
										std::get<3>(tex_sampler_binds_[k]) |= 1UL << type;
										found = true;
										break;
									}
								}
								if (!found)
								{
									tex_sampler_binds_.push_back(std::make_tuple(combined_sampler_name,
										param, effect.ParameterByName(sampler_names[j]), 1UL << type));
								}

								so_template_->pnames_[type]->push_back(combined_sampler_name);
								so_template_->glsl_res_names_[type]->push_back(combined_sampler_name);
							}
						}

						if (ST_VertexShader == type)
						{
							for (uint32_t i = 0; i < dxbc2glsl.NumInputParams(); ++ i)
							{
								if (dxbc2glsl.InputParam(i).mask != 0)
								{
									std::string semantic = dxbc2glsl.InputParam(i).semantic_name;
									uint32_t semantic_index = dxbc2glsl.InputParam(i).semantic_index;
									std::string glsl_param_name = semantic;
									size_t const semantic_hash = RT_HASH(semantic.c_str());

									if ((CT_HASH("SV_VertexID") != semantic_hash)
										&& (CT_HASH("SV_InstanceID") != semantic_hash))
									{
										VertexElementUsage usage = VEU_Position;
										uint8_t usage_index = 0;
										if (CT_HASH("POSITION") == semantic_hash)
										{
											usage = VEU_Position;
											glsl_param_name = "POSITION0";
										}
										else if (CT_HASH("NORMAL") == semantic_hash)
										{
											usage = VEU_Normal;
											glsl_param_name = "NORMAL0";
										}
										else if (CT_HASH("COLOR") == semantic_hash)
										{
											if (0 == semantic_index)
											{
												usage = VEU_Diffuse;
												glsl_param_name = "COLOR0";
											}
											else
											{
												usage = VEU_Specular;
												glsl_param_name = "COLOR1";
											}
										}
										else if (CT_HASH("BLENDWEIGHT") == semantic_hash)
										{
											usage = VEU_BlendWeight;
											glsl_param_name = "BLENDWEIGHT0";
										}
										else if (CT_HASH("BLENDINDICES") == semantic_hash)
										{
											usage = VEU_BlendIndex;
											glsl_param_name = "BLENDINDICES0";
										}
										else if (0 == semantic.find("TEXCOORD"))
										{
											usage = VEU_TextureCoord;
											usage_index = static_cast<uint8_t>(semantic_index);
											glsl_param_name = "TEXCOORD" + boost::lexical_cast<std::string>(semantic_index);
										}
										else if (CT_HASH("TANGENT") == semantic_hash)
										{
											usage = VEU_Tangent;
											glsl_param_name = "TANGENT0";
										}
										else if (CT_HASH("BINORMAL") == semantic_hash)
										{
											usage = VEU_Binormal;
											glsl_param_name = "BINORMAL0";
										}
										else
										{
											KFL_UNREACHABLE("Invalid semantic");
										}

										so_template_->vs_usages_.push_back(usage);
										so_template_->vs_usage_indices_.push_back(usage_index);
										so_template_->glsl_vs_attrib_names_.push_back(glsl_param_name);
									}
								}
							}
						}
						else if (ST_GeometryShader == type)
						{
							switch (dxbc2glsl.GSInputPrimitive())
							{
							case SP_Point:
								so_template_->gs_input_type_ = GL_POINTS;
								break;

							case SP_Line:
								so_template_->gs_input_type_ = GL_LINES;
								break;

							case SP_LineAdj:
								so_template_->gs_input_type_ = GL_LINES_ADJACENCY;
								break;

							case SP_Triangle:
								so_template_->gs_input_type_ = GL_TRIANGLES;
								break;

							case SP_TriangleAdj:
								so_template_->gs_input_type_ = GL_TRIANGLES_ADJACENCY;
								break;

							default:
								KFL_UNREACHABLE("Invalid GS input type");
							}

							switch (dxbc2glsl.GSOutputTopology(0))
							{
							case SPT_PointList:
								so_template_->gs_output_type_ = GL_POINTS;
								break;

							case SPT_LineStrip:
								so_template_->gs_output_type_ = GL_LINE_STRIP;
								break;

							case SPT_TriangleStrip:
								so_template_->gs_output_type_ = GL_TRIANGLE_STRIP;
								break;

							default:
								KFL_UNREACHABLE("Invalid GS output topology");
							}

							so_template_->gs_max_output_vertex_ = dxbc2glsl.MaxGSOutputVertex();
						}
						else if (ST_HullShader == type)
						{
							so_template_->ds_partitioning_ = dxbc2glsl.DSPartitioning();
							so_template_->ds_output_primitive_ = dxbc2glsl.DSOutputPrimitive();
						}
					}
					catch (std::exception& ex)
					{
						is_shader_validate_[type] = false;

						LogError("Error(s) in conversion: %s/%s/%s", tech.Name().c_str(), pass.Name().c_str(), sd.func_name.c_str());
						LogError(ex.what());
						LogError("Please send this information and your shader to webmaster at klayge.org. We'll fix this ASAP.");
					}
				}
			}
		}

		if (is_shader_validate_[type])
		{
			this->FillTFBVaryings(sd);
			this->AttachGLSL(type);
		}
	}

	void OGLShaderObject::AttachShader(ShaderType type, RenderEffect const & /*effect*/,
			RenderTechnique const & /*tech*/, RenderPass const & /*pass*/, ShaderObjectPtr const & shared_so)
	{
		auto so = checked_cast<OGLShaderObject*>(shared_so.get());

		is_shader_validate_[type] = so->is_shader_validate_[type];
		so_template_->shader_func_names_[type] = so->so_template_->shader_func_names_[type];

		if (is_shader_validate_[type])
		{
			so_template_->glsl_srcs_[type] = so->so_template_->glsl_srcs_[type];

			so_template_->pnames_[type] = so->so_template_->pnames_[type];
			so_template_->glsl_res_names_[type] = so->so_template_->glsl_res_names_[type];
			if (ST_VertexShader == type)
			{
				so_template_->vs_usages_ = so->so_template_->vs_usages_;
				so_template_->vs_usage_indices_ = so->so_template_->vs_usage_indices_;
				so_template_->glsl_vs_attrib_names_ = so->so_template_->glsl_vs_attrib_names_;
				so_template_->glsl_tfb_varyings_ = so->so_template_->glsl_tfb_varyings_;
			}
			else if (ST_GeometryShader == type)
			{
				so_template_->gs_input_type_ = so->so_template_->gs_input_type_;
				so_template_->gs_output_type_ = so->so_template_->gs_output_type_;
				so_template_->gs_max_output_vertex_ = so->so_template_->gs_max_output_vertex_;
				so_template_->glsl_tfb_varyings_ = so->so_template_->glsl_tfb_varyings_;
			}
			else if (ST_PixelShader == type)
			{
				has_discard_ = so->has_discard_;
			}
			else if (ST_HullShader == type)
			{
				so_template_->ds_partitioning_ = so->so_template_->ds_partitioning_;
				so_template_->ds_output_primitive_ = so->so_template_->ds_output_primitive_;
			}

			for (uint32_t j = 0; j < so->tex_sampler_binds_.size(); ++ j)
			{
				if (std::get<3>(so->tex_sampler_binds_[j]) | (1UL << type))
				{
					std::string const & combined_sampler_name = std::get<0>(so->tex_sampler_binds_[j]);
					bool found = false;
					for (uint32_t k = 0; k < tex_sampler_binds_.size(); ++ k)
					{
						if (std::get<0>(tex_sampler_binds_[k]) == combined_sampler_name)
						{
							std::get<3>(tex_sampler_binds_[k]) |= 1UL << type;
							found = true;
							break;
						}
					}
					if (!found)
					{
						tex_sampler_binds_.push_back(std::make_tuple(combined_sampler_name,
							std::get<1>(so->tex_sampler_binds_[j]), std::get<2>(so->tex_sampler_binds_[j]), 1UL << type));
					}
				}
			}

			this->AttachGLSL(type);
		}
	}

	void OGLShaderObject::LinkShaders(RenderEffect const & effect)
	{
		is_validate_ = true;
		for (size_t type = 0; type < ShaderObject::ST_NumShaderTypes; ++ type)
		{
			if (!so_template_->shader_func_names_[type].empty())
			{
				is_validate_ &= is_shader_validate_[type];
			}
		}

		if (is_validate_)
		{
			glProgramParameteri(glsl_program_, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);

			this->LinkGLSL();
			this->AttachUBOs(effect);

			if (is_validate_)
			{
				GLint num = 0;
				glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &num);
				if (num > 0)
				{
					GLint len = 0;
					glGetProgramiv(glsl_program_, GL_PROGRAM_BINARY_LENGTH, &len);
					so_template_->glsl_bin_program_.resize(len);
					glGetProgramBinary(glsl_program_, len, nullptr, &so_template_->glsl_bin_format_,
						so_template_->glsl_bin_program_.data());
				}
			}

			for (int type = 0; type < ST_NumShaderTypes; ++ type)
			{
				if (so_template_->pnames_[type])
				{
					for (size_t pi = 0; pi < so_template_->pnames_[type]->size(); ++ pi)
					{
						GLint location = glGetUniformLocation(glsl_program_,
							(*so_template_->glsl_res_names_[type])[pi].c_str());
						if (location != -1)
						{
							RenderEffectParameter* p = effect.ParameterByName((*so_template_->pnames_[type])[pi]);
							if (p)
							{
								BOOST_ASSERT(REDT_buffer == p->Type());

								ParameterBind pb;
								pb.param = p;
								pb.location = location;

								uint32_t index = static_cast<uint32_t>(textures_.size());
								textures_.resize(index + 1);
								gl_bind_targets_.resize(index + 1);
								gl_bind_textures_.resize(index + 1);
								gl_bind_samplers_.resize(index + 1);

								pb.func = SetOGLShaderParameter<GraphicsBufferPtr>(textures_,
									gl_bind_targets_, gl_bind_textures_, gl_bind_samplers_, location, index, p);

								param_binds_.push_back(pb);
							}
							else
							{
								for (size_t i = 0; i < tex_sampler_binds_.size(); ++ i)
								{
									if (std::get<0>(tex_sampler_binds_[i]) == (*so_template_->pnames_[type])[pi])
									{
										ParameterBind pb;
										pb.combined_sampler_name = std::get<0>(tex_sampler_binds_[i]);
										pb.param = nullptr;
										pb.location = location;
										pb.tex_sampler_bind_index = static_cast<int>(i);

										uint32_t index = static_cast<uint32_t>(textures_.size());
										textures_.resize(index + 1);
										gl_bind_targets_.resize(index + 1);
										gl_bind_textures_.resize(index + 1);
										gl_bind_samplers_.resize(index + 1);

										pb.func = SetOGLShaderParameter<TexturePtr>(textures_,
											gl_bind_targets_, gl_bind_textures_, gl_bind_samplers_,
											location, index, std::get<1>(tex_sampler_binds_[i]), std::get<2>(tex_sampler_binds_[i]));

										param_binds_.push_back(pb);

										break;
									}
								}
							}
						}
					}
				}

				if (ST_VertexShader == type)
				{
					for (size_t pi = 0; pi < so_template_->glsl_vs_attrib_names_.size(); ++ pi)
					{
						attrib_locs_.emplace(std::make_pair(so_template_->vs_usages_[pi], so_template_->vs_usage_indices_[pi]),
								glGetAttribLocation(glsl_program_, so_template_->glsl_vs_attrib_names_[pi].c_str()));
					}
				}
			}
		}
	}

	ShaderObjectPtr OGLShaderObject::Clone(RenderEffect const & effect)
	{
		OGLShaderObjectPtr ret = MakeSharedPtr<OGLShaderObject>(so_template_);

		ret->is_shader_validate_ = is_shader_validate_;
		ret->is_validate_ = is_validate_;
		ret->has_discard_ = has_discard_;
		ret->has_tessellation_ = has_tessellation_;

		ret->tex_sampler_binds_.resize(tex_sampler_binds_.size());
		for (size_t i = 0; i < tex_sampler_binds_.size(); ++ i)
		{
			std::get<0>(ret->tex_sampler_binds_[i]) = std::get<0>(tex_sampler_binds_[i]);
			std::get<1>(ret->tex_sampler_binds_[i]) = effect.ParameterByName(std::get<1>(tex_sampler_binds_[i])->Name());
			std::get<2>(ret->tex_sampler_binds_[i]) = effect.ParameterByName(std::get<2>(tex_sampler_binds_[i])->Name());
			std::get<3>(ret->tex_sampler_binds_[i]) = std::get<3>(tex_sampler_binds_[i]);
		}

		if (ret->is_validate_)
		{
			if (!so_template_->glsl_bin_program_.empty())
			{
				glProgramParameteri(ret->glsl_program_, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);
				glProgramBinary(ret->glsl_program_, so_template_->glsl_bin_format_,
					so_template_->glsl_bin_program_.data(), static_cast<GLsizei>(so_template_->glsl_bin_program_.size()));

#ifdef KLAYGE_DEBUG
				GLint linked = false;
				glGetProgramiv(ret->glsl_program_, GL_LINK_STATUS, &linked);
				if (!linked)
				{
					GLint len = 0;
					glGetProgramiv(ret->glsl_program_, GL_INFO_LOG_LENGTH, &len);
					if (len > 0)
					{
						std::vector<char> info(len + 1, 0);
						glGetProgramInfoLog(ret->glsl_program_, len, &len, &info[0]);
						LogError(&info[0]);
					}
				}
#endif
			}
			else
			{
				for (size_t type = 0; type < ST_NumShaderTypes; ++ type)
				{
					if (ret->is_shader_validate_[type])
					{
						if (so_template_->glsl_srcs_[type] && !so_template_->glsl_srcs_[type]->empty())
						{
							ret->AttachGLSL(static_cast<uint32_t>(type));
						}
					}
				}

				ret->LinkGLSL();
			}

			ret->AttachUBOs(effect);
			ret->attrib_locs_ = attrib_locs_;
			for (auto const & pb : param_binds_)
			{
				if (pb.param)
				{
					RenderEffectParameter* p = effect.ParameterByName(pb.param->Name());
					BOOST_ASSERT(REDT_buffer == p->Type());

					ParameterBind new_pb;
					new_pb.param = p;
					new_pb.location = pb.location;

					uint32_t index = static_cast<uint32_t>(ret->textures_.size());
					ret->textures_.resize(index + 1);
					ret->gl_bind_targets_.resize(index + 1);
					ret->gl_bind_textures_.resize(index + 1);
					ret->gl_bind_samplers_.resize(index + 1);

					new_pb.func = SetOGLShaderParameter<GraphicsBufferPtr>(ret->textures_,
						ret->gl_bind_targets_, ret->gl_bind_textures_, ret->gl_bind_samplers_,
						new_pb.location, index, p);

					ret->param_binds_.push_back(new_pb);
				}
				else
				{
					std::string const & pname = pb.combined_sampler_name;
					for (size_t j = 0; j < ret->tex_sampler_binds_.size(); ++ j)
					{
						if (std::get<0>(ret->tex_sampler_binds_[j]) == pname)
						{
							ParameterBind new_pb;
							new_pb.combined_sampler_name = pname;
							new_pb.param = nullptr;
							new_pb.location = pb.location;
							new_pb.tex_sampler_bind_index = pb.tex_sampler_bind_index;

							uint32_t index = static_cast<uint32_t>(ret->textures_.size());
							ret->textures_.resize(index + 1);
							ret->gl_bind_targets_.resize(index + 1);
							ret->gl_bind_textures_.resize(index + 1);
							ret->gl_bind_samplers_.resize(index + 1);

							new_pb.func = SetOGLShaderParameter<TexturePtr>(ret->textures_,
								ret->gl_bind_targets_, ret->gl_bind_textures_, ret->gl_bind_samplers_,
								new_pb.location, index,
								std::get<1>(ret->tex_sampler_binds_[new_pb.tex_sampler_bind_index]),
								std::get<2>(ret->tex_sampler_binds_[new_pb.tex_sampler_bind_index]));

							ret->param_binds_.push_back(new_pb);

							break;
						}
					}
				}
			}
		}

		return ret;
	}

	GLint OGLShaderObject::GetAttribLocation(VertexElementUsage usage, uint8_t usage_index)
	{
		auto iter = attrib_locs_.find(std::make_pair(usage, usage_index));
		if (iter != attrib_locs_.end())
		{
			return iter->second;
		}
		else
		{
			return -1;
		}
	}

	void OGLShaderObject::AttachGLSL(uint32_t type)
	{
		GLenum shader_type;
		switch (type)
		{
		case ST_VertexShader:
			shader_type = GL_VERTEX_SHADER;
			break;

		case ST_PixelShader:
			shader_type = GL_FRAGMENT_SHADER;
			break;

		case ST_GeometryShader:
			shader_type = GL_GEOMETRY_SHADER;
			break;

		case ST_HullShader:
			shader_type = GL_TESS_CONTROL_SHADER;
			break;

		case ST_DomainShader:
			shader_type = GL_TESS_EVALUATION_SHADER;
			break;

		default:
			shader_type = 0;
			break;
		}

		char const * glsl = so_template_->glsl_srcs_[type]->c_str();
		GLuint object = glCreateShader(shader_type);
		if (0 == object)
		{
			is_shader_validate_[type] = false;
		}
		else
		{
			glShaderSource(object, 1, &glsl, nullptr);

			glCompileShader(object);

			GLint compiled = false;
			glGetShaderiv(object, GL_COMPILE_STATUS, &compiled);
			if (!compiled)
			{
				LogError("Error when compiling GLSL %s:", so_template_->shader_func_names_[type].c_str());

				GLint len = 0;
				glGetShaderiv(object, GL_INFO_LOG_LENGTH, &len);
				if (len > 0)
				{
					std::vector<char> info(len + 1, 0);
					glGetShaderInfoLog(object, len, &len, &info[0]);
					this->PrintGLSLError(static_cast<ShaderType>(type), &info[0]);
				}

				is_shader_validate_[type] = false;
			}

			glAttachShader(glsl_program_, object);

			glDeleteShader(object);
		}
	}

	void OGLShaderObject::LinkGLSL()
	{
		if (!so_template_->glsl_tfb_varyings_.empty())
		{
			std::vector<GLchar const *> names(so_template_->glsl_tfb_varyings_.size());
			for (size_t i = 0; i < so_template_->glsl_tfb_varyings_.size(); ++ i)
			{
				names[i] = so_template_->glsl_tfb_varyings_[i].c_str();
			}

			glTransformFeedbackVaryings(glsl_program_, static_cast<GLsizei>(so_template_->glsl_tfb_varyings_.size()),
				&names[0], so_template_->tfb_separate_attribs_ ? GL_SEPARATE_ATTRIBS : GL_INTERLEAVED_ATTRIBS);
		}

		glLinkProgram(glsl_program_);

		GLint linked = false;
		glGetProgramiv(glsl_program_, GL_LINK_STATUS, &linked);
#ifdef KLAYGE_DEBUG
		if (!linked)
		{
			std::string shader_names;
			for (size_t type = 0; type < ShaderObject::ST_NumShaderTypes; ++ type)
			{
				if (!so_template_->shader_func_names_[type].empty())
				{
					shader_names += so_template_->shader_func_names_[type] + '/';
				}
			}
			if (!shader_names.empty())
			{
				shader_names.resize(shader_names.size() - 1);
			}

			LogError("Error when linking GLSLs %s:", shader_names.c_str());

			GLint len = 0;
			glGetProgramiv(glsl_program_, GL_INFO_LOG_LENGTH, &len);
			if (len > 0)
			{
				std::vector<char> info(len + 1, 0);
				glGetProgramInfoLog(glsl_program_, len, &len, &info[0]);

				OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				if (re.HackForNV())
				{
					std::istringstream err_iss(&info[0]);
					std::string type1, type2;
					err_iss >> type1 >> type2;
					if ("Vertex" == type1)
					{
						this->PrintGLSLError(ST_VertexShader, &info[0]);
					}
					else if ("Geometry" == type1)
					{
						this->PrintGLSLError(ST_GeometryShader, &info[0]);
					}
					else if ("Fragment" == type1)
					{
						this->PrintGLSLError(ST_PixelShader, &info[0]);
					}
					else if ("Tessellation" == type1)
					{
						if ("control" == type2)
						{
							this->PrintGLSLError(ST_HullShader, &info[0]);
						}
						else
						{
							this->PrintGLSLError(ST_DomainShader, &info[0]);
						}
					}
					else
					{
						LogError(&info[0]);
					}
				}
				else
				{
					LogError(&info[0]);
				}
			}
		}
#endif
		is_validate_ &= linked ? true : false;
	}

	void OGLShaderObject::AttachUBOs(RenderEffect const & effect)
	{
		GLint active_ubos = 0;
		glGetProgramiv(glsl_program_, GL_ACTIVE_UNIFORM_BLOCKS, &active_ubos);
		all_cbuffs_.resize(active_ubos);
		gl_bind_cbuffs_.resize(active_ubos);
		for (int i = 0; i < active_ubos; ++ i)
		{
			GLint length = 0;
			glGetActiveUniformBlockiv(glsl_program_, i, GL_UNIFORM_BLOCK_NAME_LENGTH, &length);

			std::vector<GLchar> ubo_name(length, '\0');
			glGetActiveUniformBlockName(glsl_program_, i, length, nullptr, &ubo_name[0]);

			auto cbuff = effect.CBufferByName(&ubo_name[0]);
			BOOST_ASSERT(cbuff);
			all_cbuffs_[i] = cbuff;

			glUniformBlockBinding(glsl_program_, glGetUniformBlockIndex(glsl_program_, &ubo_name[0]), i);

			GLint ubo_size = 0;
			glGetActiveUniformBlockiv(glsl_program_, i, GL_UNIFORM_BLOCK_DATA_SIZE, &ubo_size);
			cbuff->Resize(ubo_size);
			gl_bind_cbuffs_[i] = checked_cast<OGLGraphicsBuffer*>(cbuff->HWBuff().get())->GLvbo();

			GLint uniforms = 0;
			glGetActiveUniformBlockiv(glsl_program_, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &uniforms);

			std::vector<GLuint> uniform_indices(uniforms);
			glGetActiveUniformBlockiv(glsl_program_, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES,
				reinterpret_cast<GLint*>(&uniform_indices[0]));

			std::vector<GLint> uniform_name_lens(uniforms);
			glGetActiveUniformsiv(glsl_program_, uniforms, &uniform_indices[0],
				GL_UNIFORM_NAME_LENGTH, &uniform_name_lens[0]);

			std::vector<GLint> uniform_array_strides(uniforms);
			glGetActiveUniformsiv(glsl_program_, uniforms, &uniform_indices[0],
				GL_UNIFORM_ARRAY_STRIDE, &uniform_array_strides[0]);

			std::vector<GLint> uniform_matrix_strides(uniforms);
			glGetActiveUniformsiv(glsl_program_, uniforms, &uniform_indices[0],
				GL_UNIFORM_MATRIX_STRIDE, &uniform_matrix_strides[0]);

			std::vector<GLint> uniform_offsets(uniforms);
			glGetActiveUniformsiv(glsl_program_, uniforms, &uniform_indices[0],
				GL_UNIFORM_OFFSET, &uniform_offsets[0]);

			for (GLint j = 0; j < uniforms; ++ j)
			{
				std::vector<GLchar> uniform_name(uniform_name_lens[j], '\0');
				GLint size;
				GLenum type;
				glGetActiveUniform(glsl_program_, uniform_indices[j], uniform_name_lens[j],
					nullptr, &size, &type, &uniform_name[0]);

				auto iter = std::find(uniform_name.begin(), uniform_name.end(), '[');
				if (iter != uniform_name.end())
				{
					*iter = '\0';
				}

				RenderEffectParameter* param = effect.ParameterByName(&uniform_name[0]);
				GLint stride;
				if (param->ArraySize())
				{
					stride = uniform_array_strides[j];
				}
				else
				{
					if (param->Type() != REDT_float4x4)
					{
						stride = 4;
					}
					else
					{
						stride = uniform_matrix_strides[j];
					}
				}
				param->BindToCBuffer(*cbuff, uniform_offsets[j], stride);
			}
		}
	}

	void OGLShaderObject::FillTFBVaryings(ShaderDesc const & sd)
	{
		int slot = -1;
		so_template_->tfb_separate_attribs_ = false;
		for (auto const & decl : sd.so_decl)
		{
			if (slot < 0)
			{
				slot = decl.slot;
			}
			else
			{
				if (slot != decl.slot)
				{
					so_template_->tfb_separate_attribs_ = true;
				}
			}

			std::string glsl_param_name;
			switch (decl.usage)
			{
			case VEU_Position:
				glsl_param_name = "gl_Position";
				break;

			case VEU_Normal:
				glsl_param_name = "v_NORMAL0";
				break;

			case VEU_Diffuse:
				glsl_param_name = "v_COLOR0";
				break;

			case VEU_Specular:
				glsl_param_name = "v_COLOR1";
				break;

			case VEU_BlendWeight:
				glsl_param_name = "v_BLENDWEIGHT0";
				break;

			case VEU_BlendIndex:
				glsl_param_name = "v_BLENDINDICES0";
				break;

			case VEU_TextureCoord:
				glsl_param_name = "v_TEXCOORD" + boost::lexical_cast<std::string>(static_cast<int>(decl.usage_index));
				break;

			case VEU_Tangent:
				glsl_param_name = "v_TANGENT0";
				break;

			case VEU_Binormal:
				glsl_param_name = "v_BINORMAL0";
				break;

			default:
				KFL_UNREACHABLE("Invalid usage");
			}

			so_template_->glsl_tfb_varyings_.push_back(glsl_param_name);
		}
	}

	void OGLShaderObject::Bind()
	{
		if (!so_template_->glsl_srcs_[ShaderObject::ST_PixelShader]
			|| so_template_->glsl_srcs_[ShaderObject::ST_PixelShader]->empty())
		{
			glEnable(GL_RASTERIZER_DISCARD);
		}

		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		re.UseProgram(glsl_program_);

		for (auto const & pb : param_binds_)
		{
			pb.func();
		}

		for (size_t i = 0; i < all_cbuffs_.size(); ++ i)
		{
			all_cbuffs_[i]->Update();
		}

		if (!gl_bind_cbuffs_.empty())
		{
			re.BindBuffersBase(GL_UNIFORM_BUFFER, 0, static_cast<GLsizei>(all_cbuffs_.size()), &gl_bind_cbuffs_[0]);
		}

		if (!gl_bind_textures_.empty())
		{
			re.BindTextures(0, static_cast<GLsizei>(gl_bind_textures_.size()), &gl_bind_targets_[0], &gl_bind_textures_[0]);
		}

		if (!gl_bind_samplers_.empty())
		{
			re.BindSamplers(0, static_cast<GLsizei>(gl_bind_samplers_.size()), &gl_bind_samplers_[0]);
		}

#ifdef KLAYGE_DEBUG
		glValidateProgram(glsl_program_);

		GLint validated = false;
		glGetProgramiv(glsl_program_, GL_VALIDATE_STATUS, &validated);
		if (!validated)
		{
			GLint len = 0;
			glGetProgramiv(glsl_program_, GL_INFO_LOG_LENGTH, &len);
			if (len > 0)
			{
				std::vector<char> info(len + 1, 0);
				glGetProgramInfoLog(glsl_program_, len, &len, &info[0]);
				LogError(&info[0]);
			}
		}
#endif
	}

	void OGLShaderObject::Unbind()
	{
		if (!so_template_->glsl_srcs_[ShaderObject::ST_PixelShader]
			|| so_template_->glsl_srcs_[ShaderObject::ST_PixelShader]->empty())
		{
			glDisable(GL_RASTERIZER_DISCARD);
		}
	}

	void OGLShaderObject::PrintGLSLError(ShaderType type, char const * info)
	{
		OGLRenderEngine& re = *checked_cast<OGLRenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		std::string const & glsl = *so_template_->glsl_srcs_[type];

		if (re.HackForIntel())
		{
			std::istringstream err_iss(info);
			std::string err_str;
			while (err_iss)
			{
				std::getline(err_iss, err_str);
				if (!err_str.empty())
				{
					std::string::size_type pos = err_str.find(": 0:");
					if (pos != std::string::npos)
					{
						pos += 4;
						std::string::size_type pos2 = err_str.find(':', pos + 1);
						std::string part_err_str = err_str.substr(pos, pos2 - pos);
						this->PrintGLSLErrorAtLine(glsl, boost::lexical_cast<int>(part_err_str));
					}

					LogError(err_str.c_str());
					LogError("\n");
				}
			}
		}
		else if (re.HackForNV())
		{
			std::istringstream err_iss(info);
			std::string err_str;
			while (err_iss)
			{
				std::getline(err_iss, err_str);
				if (!err_str.empty())
				{
					std::string::size_type pos = err_str.find(") : error");
					if (pos != std::string::npos)
					{
						std::string::size_type pos2 = err_str.find('(') + 1;
						std::string part_err_str = err_str.substr(pos2, pos - pos2);
						this->PrintGLSLErrorAtLine(glsl, boost::lexical_cast<int>(part_err_str));
					}

					LogError(err_str.c_str());
					LogError("\n");
				}
			}
		}
		else
		{
			std::istringstream iss(glsl);
			std::string s;
			int line = 1;
			while (iss)
			{
				std::getline(iss, s);
				LogError("%d %s", line, s.c_str());
				++ line;
			}

			LogError(&info[0]);
			LogError("\n");
		}
	}

	void OGLShaderObject::PrintGLSLErrorAtLine(std::string const & glsl, int err_line)
	{
		std::istringstream iss(glsl);
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
	}
}
