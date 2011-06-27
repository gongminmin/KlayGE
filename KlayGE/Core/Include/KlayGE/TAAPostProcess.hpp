#ifndef _TAAPOSTPROCESS_HPP
#define _TAAPOSTPROCESS_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/PostProcess.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API TAAPostProcess : public PostProcess
	{
	public:
		TAAPostProcess();

		void InputPin(uint32_t index, TexturePtr const & tex);
		using PostProcess::InputPin;

		float4x4 const & JitterProj()
		{
			return jitter_proj_;
		}

		void Apply();

		void UpdateJitterProj(Camera const & camera, uint32_t width, uint32_t height);

	private:
		TexturePtr temporal_tex_[2];
		float4x4 jitter_proj_;
		int cur_jitter_index_;
	};
}

#endif		// _TAAPOSTPROCESS_HPP

