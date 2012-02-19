// ShaderObject.cpp
// KlayGE shader对象类 实现文件
// Ver 3.5.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://www.klayge.org
//
// 3.5.0
// 初次建立 (2006.11.2)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>

#include <string>
#include <vector>

#include <KlayGE/ShaderObject.hpp>

namespace KlayGE
{
	class NullShaderObject : public ShaderObject
	{
	public:
		bool AttachNativeShader(ShaderType /*type*/, RenderEffect const & /*effect*/, std::vector<uint32_t> const & /*shader_desc_ids*/,
			std::vector<uint8_t> const & /*native_shader_block*/)
		{
			is_validate_ = true;
			return true;
		}

		void ExtractNativeShader(ShaderType /*type*/, RenderEffect const & /*effect*/, std::vector<uint8_t>& native_shader_block)
		{
			native_shader_block.clear();
		}

		void AttachShader(ShaderType /*type*/, RenderEffect const & /*effect*/, std::vector<uint32_t> const & /*shader_desc_ids*/)
		{
			is_validate_ = true;
		}

		void AttachShader(ShaderType /*type*/, RenderEffect const & /*effect*/, ShaderObjectPtr const & /*shared_so*/)
		{
			is_validate_ = true;
		}

		void LinkShaders(RenderEffect const & /*effect*/)
		{
			is_validate_ = true;
		}

		ShaderObjectPtr Clone(RenderEffect const & /*effect*/)
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
		static ShaderObjectPtr obj = MakeSharedPtr<NullShaderObject>();
		return obj;
	}
}
