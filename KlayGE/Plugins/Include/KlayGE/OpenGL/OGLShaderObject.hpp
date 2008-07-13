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

#include <boost/function.hpp>

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
		struct parameter_bind_t
		{
			RenderEffectParameterPtr param;
			CGparameter cg_param;
			boost::function<void()> func;
		};
		typedef std::vector<parameter_bind_t> parameter_binds_t;

		parameter_bind_t GetBindFunc(CGparameter cg_param, RenderEffectParameterPtr const & param, ShaderType type);

	private:
		boost::shared_ptr<std::vector<shader_desc> > shader_descs_;
		boost::shared_ptr<std::string> shader_text_;

		boost::array<CGprogram, ST_NumShaderTypes> shaders_;
		boost::array<CGprofile, ST_NumShaderTypes> profiles_;

		boost::array<parameter_binds_t, ST_NumShaderTypes> param_binds_;
		boost::array<bool, ST_NumShaderTypes> is_shader_validate_;

		boost::array<std::vector<SamplerPtr>, ST_NumShaderTypes> samplers_;
	};

	typedef boost::shared_ptr<OGLShaderObject> OGLShaderObjectPtr;
}

#endif			// _OGLSHADEROBJECT_HPP
