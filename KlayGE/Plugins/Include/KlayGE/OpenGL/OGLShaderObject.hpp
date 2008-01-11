// OGLShaderObject.hpp
// KlayGE OpenGL shader对象类 头文件
// Ver 3.5.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
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
		~OGLShaderObject();

		void SetShader(ShaderType type, boost::shared_ptr<std::vector<shader_desc> > const & shader_descs,
			boost::shared_ptr<std::string> const & shader_text);
		ShaderObjectPtr Clone();

		bool HasParameter(ShaderType type, boost::shared_ptr<std::string> const & name) const;

		void SetParameter(boost::shared_ptr<std::string> const & name, bool value);
		void SetParameter(boost::shared_ptr<std::string> const & name, int value);
		void SetParameter(boost::shared_ptr<std::string> const & name, float value);
		void SetParameter(boost::shared_ptr<std::string> const & name, float4 const & value);
		void SetParameter(boost::shared_ptr<std::string> const & name, float4x4 const & value);
		void SetParameter(boost::shared_ptr<std::string> const & name, SamplerPtr const & value);
		void SetParameter(boost::shared_ptr<std::string> const & name, std::vector<bool> const & value);
		void SetParameter(boost::shared_ptr<std::string> const & name, std::vector<int> const & value);
		void SetParameter(boost::shared_ptr<std::string> const & name, std::vector<float> const & value);
		void SetParameter(boost::shared_ptr<std::string> const & name, std::vector<float4> const & value);
		void SetParameter(boost::shared_ptr<std::string> const & name, std::vector<float4x4> const & value);

		CGprogram VertexShader() const
		{
			return shaders_[ST_VertexShader];
		}
		CGprogram PixelShader() const
		{
			return shaders_[ST_PixelShader];
		}

		CGprofile VertexShaderProfile() const
		{
			return profiles_[ST_VertexShader];
		}
		CGprofile PixelShaderProfile() const
		{
			return profiles_[ST_PixelShader];
		}

		std::vector<SamplerPtr> const & Samplers(ShaderType type) const
		{
			return samplers_[type];
		}

	private:
		struct less_shared_ptr_string
			: public std::binary_function<boost::shared_ptr<std::string>, boost::shared_ptr<std::string>, bool>
		{
			bool operator()(boost::shared_ptr<std::string> lhs, boost::shared_ptr<std::string> rhs) const
			{
				return *lhs < *rhs;
			}
		};
		
		typedef MapVector<boost::shared_ptr<std::string>, CGparameter, less_shared_ptr_string> parameter_descs_t;
		parameter_descs_t::const_iterator FindParam(ShaderType type, boost::shared_ptr<std::string> const & name) const;

	private:
		boost::shared_ptr<std::vector<shader_desc> > shader_descs_;
		boost::shared_ptr<std::string> shader_text_;

		boost::array<CGprogram, ST_NumShaderTypes> shaders_;
		boost::array<CGprofile, ST_NumShaderTypes> profiles_;

		boost::array<parameter_descs_t, ST_NumShaderTypes> param_descs_;

		boost::array<std::vector<SamplerPtr>, ST_NumShaderTypes> samplers_;
	};

	typedef boost::shared_ptr<OGLShaderObject> OGLShaderObjectPtr;
}

#endif			// _OGLSHADEROBJECT_HPP
