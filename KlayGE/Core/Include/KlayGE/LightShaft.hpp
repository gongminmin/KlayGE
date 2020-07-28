// LightShaft.hpp
// KlayGE Light shaft类 头文件
// Ver 4.2.0
// 版权所有(C) 龚敏敏, 2012
// Homepage: http://www.klayge.org
//
// 4.2.0
// 初次建立 (2012.9.23)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _LIGHTSHAFT_HPP
#define _LIGHTSHAFT_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/PostProcess.hpp>
#include <vector>

namespace KlayGE
{
	class KLAYGE_CORE_API LightShaftPostProcess final : public PostProcess
	{
	public:
		LightShaftPostProcess();

		void InputPin(uint32_t index, ShaderResourceViewPtr const& srv) override;
		using PostProcess::InputPin;

		void Apply() override;

	private:
		std::vector<PostProcessPtr> radial_blur_pps_;
		PostProcessPtr apply_pp_;

		TexturePtr blur_tex_[2];
		ShaderResourceViewPtr blur_srv_[2];
		RenderTargetViewPtr blur_rtv_[2];
	};
}

#endif
