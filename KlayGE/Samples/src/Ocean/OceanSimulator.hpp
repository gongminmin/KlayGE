#ifndef _OCEAN_SIMULATOR_HPP
#define _OCEAN_SIMULATOR_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/FFT.hpp>

namespace KlayGE
{
	struct OceanParameter
	{
		// Must be power of 2.
		int dmap_dim;
		// Typical value is 10 ~ 20
		float patch_length;

		// Typical value is 5
		float time_peroid;
		uint32_t num_frames;

		// Adjust the time interval for simulation.
		float time_scale;
		// Amplitude for transverse wave. Around 1.0
		float wave_amplitude;
		// Wind speed (2 dimension).
		float2 wind_speed;
		// This value damps out the waves against the wind direction.
		// Smaller value means higher wind dependency.
		float wind_dependency;
		// The amplitude for longitudinal wave. Must be positive.
		float choppy_scale;
	};

	class OceanSimulator
	{
	public:
		OceanSimulator();

		// Update ocean wave when tick arrives.
		void Update(uint32_t frame);

		// Texture access
		TexturePtr const & DisplacementTex() const;
		TexturePtr const & GradientTex() const;

		OceanParameter const & Parameters() const;
		void Parameters(OceanParameter const & params);

	private:
		void InitHeightMap(std::vector<float2>& out_h0, std::vector<float>& out_omega);

	private:
		OceanParameter param_;

		TexturePtr displacement_tex_;
		FrameBufferPtr displacement_fb_;

		TexturePtr gradient_tex_;
		FrameBufferPtr gradient_fb_;

		TexturePtr h0_tex_;
		TexturePtr omega_tex_;

		TexturePtr out_real_tex_;
		ShaderResourceViewPtr out_real_srv_;
		TexturePtr out_imag_tex_;
		ShaderResourceViewPtr out_imag_srv_;
		FrameBufferPtr tex_fb_;

		RenderEffectPtr effect_;
		RenderTechnique* update_spectrum_tech_;
		RenderTechnique* update_displacement_tech_;
		RenderTechnique* gen_gradient_folding_tech_;
		RenderEffectParameter* time_param_;

		RenderLayoutPtr quad_layout_;

		std::unique_ptr<GpuFft> fft_;
	};
}

#endif	// _OCEAN_SIMULATOR_HPP
