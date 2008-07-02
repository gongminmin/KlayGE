// ShaderObject.hpp
// KlayGE shader对象类 头文件
// Ver 3.5.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
//
// 3.5.0
// 初次建立 (2006.11.2)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _SHADEROBJECT_HPP
#define _SHADEROBJECT_HPP

#include <KlayGE/PreDeclare.hpp>

namespace KlayGE
{
	struct shader_desc
	{
		std::string profile;
		std::string func_name;

		friend bool operator==(shader_desc const & lhs, shader_desc const & rhs)
		{
			return (lhs.profile == rhs.profile) && (lhs.func_name == rhs.func_name);
		}
		friend bool operator!=(shader_desc const & lhs, shader_desc const & rhs)
		{
			return !(lhs == rhs);
		}
	};

	class ShaderObject
	{
	public:
		enum ShaderType
		{
			ST_VertexShader,
			ST_PixelShader,

			ST_NumShaderTypes
		};

	public:
		virtual ~ShaderObject()
		{
		}

		static ShaderObjectPtr NullObject();

		virtual void SetShader(ShaderType type, boost::shared_ptr<std::vector<shader_desc> > const & shader_descs,
			boost::shared_ptr<std::string> const & shader_text) = 0;
		virtual ShaderObjectPtr Clone() = 0;

		virtual bool HasParameter(ShaderType type, boost::shared_ptr<std::string> const & name) const = 0;

		virtual void SetParameter(boost::shared_ptr<std::string> const & name, bool value) = 0;
		virtual void SetParameter(boost::shared_ptr<std::string> const & name, int value) = 0;
		virtual void SetParameter(boost::shared_ptr<std::string> const & name, float value) = 0;
		virtual void SetParameter(boost::shared_ptr<std::string> const & name, float4 const & value) = 0;
		virtual void SetParameter(boost::shared_ptr<std::string> const & name, float4x4 const & value) = 0;
		virtual void SetParameter(boost::shared_ptr<std::string> const & name, SamplerPtr const & value) = 0;
		virtual void SetParameter(boost::shared_ptr<std::string> const & name, std::vector<bool> const & value) = 0;
		virtual void SetParameter(boost::shared_ptr<std::string> const & name, std::vector<int> const & value) = 0;
		virtual void SetParameter(boost::shared_ptr<std::string> const & name, std::vector<float> const & value) = 0;
		virtual void SetParameter(boost::shared_ptr<std::string> const & name, std::vector<float4> const & value) = 0;
		virtual void SetParameter(boost::shared_ptr<std::string> const & name, std::vector<float4x4> const & value) = 0;

		virtual void Active() = 0;

		bool Validate() const
		{
			return is_validate_;
		}

	protected:
		bool is_validate_;
	};
}

#endif			// _SHADEROBJECT_HPP
