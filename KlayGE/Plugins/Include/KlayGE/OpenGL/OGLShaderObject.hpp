// OGLShaderObject.hpp
// KlayGE OpenGL shader对象类 头文件
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

#ifndef _OGLSHADEROBJECT_HPP
#define _OGLSHADEROBJECT_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/MapVector.hpp>
#include <KlayGE/ShaderObject.hpp>

#include <Cg/cg.h>
#include <Cg/cgGL.h>

namespace KlayGE
{
	class OGLShaderObject : public ShaderObject
	{
	public:
		OGLShaderObject();
		~OGLShaderObject();

		void SetShader(RenderEffect& effect, ShaderType type, boost::shared_ptr<std::vector<shader_desc> > const & shader_descs,
			boost::shared_ptr<std::string> const & shader_text);
		ShaderObjectPtr Clone(RenderEffect& effect);

		void Active();

	private:
		typedef MapVector<RenderEffectParameterPtr, CGparameter> parameter_descs_t;

		void SetParameter(CGparameter cg_param, bool value);
		void SetParameter(CGparameter cg_param, int value);
		void SetParameter(CGparameter cg_param, float value);
		void SetParameter(CGparameter cg_param, float4 const & value);
		void SetParameter(CGparameter cg_param, float4x4 const & value);
		void SetParameter(CGparameter cg_param, ShaderType type, SamplerPtr const & value);
		void SetParameter(CGparameter cg_param, std::vector<bool> const & value);
		void SetParameter(CGparameter cg_param, std::vector<int> const & value);
		void SetParameter(CGparameter cg_param, std::vector<float> const & value);
		void SetParameter(CGparameter cg_param, std::vector<float4> const & value);
		void SetParameter(CGparameter cg_param, std::vector<float4x4> const & value);

	private:
		boost::shared_ptr<std::vector<shader_desc> > shader_descs_;
		boost::shared_ptr<std::string> shader_text_;

		boost::array<CGprogram, ST_NumShaderTypes> shaders_;
		boost::array<CGprofile, ST_NumShaderTypes> profiles_;

		boost::array<parameter_descs_t, ST_NumShaderTypes> param_descs_;
		boost::array<bool, ST_NumShaderTypes> is_shader_validate_;

		boost::array<std::vector<SamplerPtr>, ST_NumShaderTypes> samplers_;
	};

	typedef boost::shared_ptr<OGLShaderObject> OGLShaderObjectPtr;
}

#endif			// _OGLSHADEROBJECT_HPP
