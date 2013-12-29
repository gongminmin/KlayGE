#ifndef _SSGIPOSTPROCESS_HPP
#define _SSGIPOSTPROCESS_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/PostProcess.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API SSGIPostProcess : public PostProcess
	{
	public:
		SSGIPostProcess();

		void OnRenderBegin();

	private:
		RenderEffectParameterPtr proj_param_;
		RenderEffectParameterPtr inv_proj_param_;
		RenderEffectParameterPtr far_plane_param_;
	};
}

#endif		// _SSGIPOSTPROCESS_HPP
