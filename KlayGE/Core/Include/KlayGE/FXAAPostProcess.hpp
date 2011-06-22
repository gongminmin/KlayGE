#ifndef _FXAAPOSTPROCESS_HPP
#define _FXAAPOSTPROCESS_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/PostProcess.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API FXAAPostProcess : public PostProcess
	{
	public:
		FXAAPostProcess();

		void InputPin(uint32_t index, TexturePtr const & tex);
		using PostProcess::InputPin;

		void ShowEdge(bool se);

	private:
		RenderTechniquePtr fxaa_tech_;
		RenderTechniquePtr show_edge_tech_;
	};
}

#endif		// _FXAAPOSTPROCESS_HPP

