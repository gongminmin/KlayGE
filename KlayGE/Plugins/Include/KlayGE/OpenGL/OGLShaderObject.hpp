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

		void SetShader(ShaderType type, std::string const & profile, std::string const & name, std::string const & text);

		bool HasParameter(ShaderType type, std::string const & name) const;

		void SetParameter(std::string const & name, bool value);
		void SetParameter(std::string const & name, int value);
		void SetParameter(std::string const & name, float value);
		void SetParameter(std::string const & name, float4 const & value);
		void SetParameter(std::string const & name, float4x4 const & value);
		void SetParameter(std::string const & name, SamplerPtr const & value);
		void SetParameter(std::string const & name, std::vector<bool> const & value);
		void SetParameter(std::string const & name, std::vector<int> const & value);
		void SetParameter(std::string const & name, std::vector<float> const & value);
		void SetParameter(std::string const & name, std::vector<float4> const & value);
		void SetParameter(std::string const & name, std::vector<float4x4> const & value);

		int32_t AttribIndex(VertexElementUsage usage, uint8_t usage_index);

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
		boost::array<CGprogram, ST_NumShaderTypes> shaders_;
		boost::array<CGprofile, ST_NumShaderTypes> profiles_;

		typedef std::vector<std::pair<std::string, CGparameter> > parameter_descs_t;
		boost::array<parameter_descs_t, ST_NumShaderTypes> param_descs_;

		MapVector<std::pair<VertexElementUsage, uint8_t>, uint8_t> vertex_varyings_;

		boost::array<std::vector<SamplerPtr>, ST_NumShaderTypes> samplers_;
	};

	typedef boost::shared_ptr<OGLShaderObject> OGLShaderObjectPtr;
}

#endif			// _OGLSHADEROBJECT_HPP
