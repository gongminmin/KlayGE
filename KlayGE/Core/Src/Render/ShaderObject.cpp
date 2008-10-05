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
		void SetShader(RenderEffect& /*effect*/, ShaderType /*type*/, boost::shared_ptr<std::vector<shader_desc> > const & /*shader_descs*/,
			boost::shared_ptr<std::string> const & /*shader_text*/)
		{
			is_validate_ = true;
		}

		ShaderObjectPtr Clone(RenderEffect& /*effect*/)
		{
			return ShaderObject::NullObject();
		}

		void Bind()
		{
		}

		void Unbind()
		{
		}
	};

	ShaderObjectPtr ShaderObject::NullObject()
	{
		static ShaderObjectPtr obj(new NullShaderObject);
		return obj;
	}
}
