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
#include <KFL/Util.hpp>
#include <KlayGE/RenderEffect.hpp>

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

		virtual bool StreamIn(ResIdentifierPtr const & res, ShaderType type, RenderEffect const & effect,
			std::vector<uint32_t> const & shader_desc_ids) KLAYGE_OVERRIDE
		{
			UNREF_PARAM(res);
			UNREF_PARAM(type);
			UNREF_PARAM(effect);
			UNREF_PARAM(shader_desc_ids);

			return true;
		}

		virtual void StreamOut(std::ostream& os, ShaderType type) KLAYGE_OVERRIDE
		{
			UNREF_PARAM(os);
			UNREF_PARAM(type);
		}

		void AttachShader(ShaderType /*type*/, RenderEffect const & /*effect*/,
			RenderTechnique const & /*tech*/, RenderPass const & /*pass*/, std::vector<uint32_t> const & /*shader_desc_ids*/)
		{
			is_validate_ = true;
		}

		void AttachShader(ShaderType /*type*/, RenderEffect const & /*effect*/,
			RenderTechnique const & /*tech*/, RenderPass const & /*pass*/, ShaderObjectPtr const & /*shared_so*/)
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

	ShaderObject::ShaderObject()
		: has_discard_(false), has_tessellation_(false),
			cs_block_size_x_(0), cs_block_size_y_(0), cs_block_size_z_(0)
	{
	}
}
