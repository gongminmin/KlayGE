#ifndef _SSVOPOSTPROCESS_HPP
#define _SSVOPOSTPROCESS_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/PostProcess.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API SSVOPostProcess : public PostProcess
	{
	public:
		SSVOPostProcess();

		void OnRenderBegin();

	private:
		RenderEffectParameterPtr proj_param_;
		RenderEffectParameterPtr inv_proj_param_;
		RenderEffectParameterPtr depth_near_far_invfar_param_;
	};
}

#endif		// _SSVOPOSTPROCESS_HPP
