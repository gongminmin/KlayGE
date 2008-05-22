// ShaderObject.cpp
// KlayGE shader对象类 实现文件
// Ver 3.5.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
//
// 3.5.0
// 初次建立 (2006.11.2)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>

#include <string>
#include <vector>

#include <KlayGE/ShaderObject.hpp>

namespace KlayGE
{
	class NullShaderObject : public ShaderObject
	{
	public:
		void SetShader(ShaderType /*type*/, boost::shared_ptr<std::vector<shader_desc> > const & /*shader_descs*/,
			boost::shared_ptr<std::string> const & /*shader_text*/)
		{
			is_validate_ = true;
		}

		ShaderObjectPtr Clone()
		{
			return ShaderObject::NullObject();
		}

		bool HasParameter(ShaderType /*type*/, boost::shared_ptr<std::string> const & /*name*/) const
		{
			return false;
		}

		void SetParameter(boost::shared_ptr<std::string> const & /*name*/, bool /*value*/)
		{
		}
		void SetParameter(boost::shared_ptr<std::string> const & /*name*/, int /*value*/)
		{
		}
		void SetParameter(boost::shared_ptr<std::string> const & /*name*/, float /*value*/)
		{
		}
		void SetParameter(boost::shared_ptr<std::string> const & /*name*/, float4 const & /*value*/)
		{
		}
		void SetParameter(boost::shared_ptr<std::string> const & /*name*/, float4x4 const & /*value*/)
		{
		}
		void SetParameter(boost::shared_ptr<std::string> const & /*name*/, SamplerPtr const & /*value*/)
		{
		}
		void SetParameter(boost::shared_ptr<std::string> const & /*name*/, std::vector<bool> const & /*value*/)
		{
		}
		void SetParameter(boost::shared_ptr<std::string> const & /*name*/, std::vector<int> const & /*value*/)
		{
		}
		void SetParameter(boost::shared_ptr<std::string> const & /*name*/, std::vector<float> const & /*value*/)
		{
		}
		void SetParameter(boost::shared_ptr<std::string> const & /*name*/, std::vector<float4> const & /*value*/)
		{
		}
		void SetParameter(boost::shared_ptr<std::string> const & /*name*/, std::vector<float4x4> const & /*value*/)
		{
		}
	};

	ShaderObjectPtr ShaderObject::NullObject()
	{
		static ShaderObjectPtr obj(new NullShaderObject);
		return obj;
	}
}
