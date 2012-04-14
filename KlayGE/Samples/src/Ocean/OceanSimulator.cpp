#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/FFT.hpp>

#include "OceanSimulator.hpp"

namespace
{
	using namespace KlayGE;

	float const HALF_SQRT_2 = 0.7071068f;
	float const GRAV_ACCEL = 9.8f;	// The acceleration of gravity, m/s^2

	// Generating gaussian random number with mean 0 and standard deviation 1.
	float Gauss()
	{
		float u1 = rand() / static_cast<float>(RAND_MAX);
		float u2 = rand() / static_cast<float>(RAND_MAX);
		if (u1 < 1e-6f)
		{
			u1 = 1e-6f;
		}
		return sqrt(-2 * log(u1)) * cos(2 * PI * u2);
	}

	// Phillips Spectrum
	// K: normalized wave vector, W: wind direction, v: wind velocity, a: amplitude constant
	float Phillips(float2 K, float2 W, float a, float dir_depend)
	{
		float v = MathLib::length(W);
		W /= v;

		// largest possible wave from constant wind of velocity v
		float l = v * v / GRAV_ACCEL;
		// damp out waves with very small length w << l
		float w = l / 1000;

		float Ksqr = MathLib::dot(K, K);
		float Kcos = MathLib::dot(K, W);
		float phillips = a * exp(-1 / (l * l * Ksqr)) / (Ksqr * Ksqr * Ksqr) * (Kcos * Kcos);

		// filter out waves moving opposite to wind
		if (Kcos < 0)
		{
			phillips *= dir_depend;
		}

		// damp out waves with very small length w << l
		return phillips * exp(-Ksqr * w * w);
	}
}

namespace KlayGE
{
	OceanSimulator::OceanSimulator()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		RenderEffectPtr effect = rf.LoadEffect("OceanSimulator.fxml");
		update_spectrum_tech_ = effect->TechniqueByName("UpdateSpectrum");
		update_displacement_tech_ = effect->TechniqueByName("UpdateDisplacement");
		gen_gradient_folding_tech_ = effect->TechniqueByName("GenGradientFolding");

		quad_layout_ = rf.MakeRenderLayout();
		quad_layout_->TopologyType(RenderLayout::TT_TriangleStrip);

		float2 xys[] =
		{
			float2(-1, +1),
			float2(+1, +1),
			float2(-1, -1),
			float2(+1, -1)
		};
		ElementInitData init_data;
		init_data.row_pitch = sizeof(xys);
		init_data.slice_pitch = 0;
		init_data.data = xys;
		GraphicsBufferPtr quad_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
		quad_layout_->BindVertexStream(quad_vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_GR32F)));

		time_param_ = effect->ParameterByName("time");

		tex_fb_ = rf.MakeFrameBuffer();
	}

	// Initialize the vector field.
	// wlen_x: width of wave tile, in meters
	// wlen_y: length of wave tile, in meters
	void OceanSimulator::InitHeightMap(std::vector<float2>& out_h0, std::vector<float>& out_omega)
	{
		float2 K;

		// initialize random generator.
		srand(0);
		
		float interval = 2 * PI / param_.time_peroid;
		for (int i = 0; i <= param_.dmap_dim; i++)
		{
			// K is wave-vector, range [-|DX/W, |DX/W], [-|DY/H, |DY/H]
			K.y() = (-param_.dmap_dim / 2.0f + i) * (2 * PI / param_.patch_length);

			for (int j = 0; j <= param_.dmap_dim; j++)
			{
				K.x() = (-param_.dmap_dim / 2.0f + j) * (2 * PI / param_.patch_length);

				float phil = ((K.x() == 0) && (K.y() == 0)) ? 0 : sqrt(Phillips(K, param_.wind_speed, param_.wave_amplitude, param_.wind_dependency)) * HALF_SQRT_2;
				out_h0[i * (param_.dmap_dim + 4) + j] = phil * float2(Gauss(), Gauss());

				// The angular frequency is following the dispersion relation:
				//            out_omega^2 = g*k
				// So the equation of Gerstner wave is:
				//            x = x0 - K/k * A * sin(dot(K, x0) - sqrt(g * k) * t), x is a 2D vector.
				//            z = A * cos(dot(K, x0) - sqrt(g * k) * t)
				// Gerstner wave shows that a point on a simple sinusoid wave is doing a uniform circular
				// motion with the center at (x0, y0, z0), radius at A, and the circular plane is parallel
				// to vector K.
				out_omega[i * (param_.dmap_dim + 4) + j] = static_cast<int>(sqrt(GRAV_ACCEL * MathLib::length(K)) / interval) * interval;
			}
		}
	}

	void OceanSimulator::Update(uint32_t frame)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();
		
		FrameBufferPtr old_fb = re.CurFrameBuffer();
		
		*time_param_ = frame * param_.time_peroid / param_.num_frames;

		re.BindFrameBuffer(tex_fb_);
		re.Render(*update_spectrum_tech_, *quad_layout_);

		fft_->Execute(out_real_tex_, out_imag_tex_, out_real_tex_, out_imag_tex_);

		re.BindFrameBuffer(displacement_fb_);
		re.Render(*update_displacement_tech_, *quad_layout_);

		re.BindFrameBuffer(gradient_fb_);
		re.Render(*gen_gradient_folding_tech_, *quad_layout_);

		re.BindFrameBuffer(old_fb);
	}

	TexturePtr const & OceanSimulator::DisplacementTex() const
	{
		return displacement_tex_;
	}

	TexturePtr const & OceanSimulator::GradientTex() const
	{
		return gradient_tex_;
	}

	OceanParameter const & OceanSimulator::Parameters() const
	{
		return param_;
	}

	void OceanSimulator::Parameters(OceanParameter const & params)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		param_ = params;

		int height_map_size = (params.dmap_dim + 4) * (params.dmap_dim + 1);
		std::vector<float2> h0_data(height_map_size);
		std::vector<float> omega_data(height_map_size);
		this->InitHeightMap(h0_data, omega_data);

		ElementInitData init_data;

		std::vector<char> zero_data(3 * params.dmap_dim * params.dmap_dim * sizeof(float) * 2, 0);

		init_data.row_pitch = static_cast<uint32_t>(h0_data.size() * sizeof(h0_data[0]));
		init_data.data = &h0_data[0];
		h0_buffer_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_GPU_Unordered | EAH_GPU_Structured, &init_data, EF_GR32F);

		init_data.row_pitch = static_cast<uint32_t>(omega_data.size() * sizeof(omega_data[0]));
		init_data.data = &omega_data[0];
		omega_buffer_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_GPU_Unordered | EAH_GPU_Structured, &init_data, EF_R32F);

		displacement_tex_ = rf.MakeTexture2D(params.dmap_dim, params.dmap_dim, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
		gradient_tex_ = rf.MakeTexture2D(params.dmap_dim, params.dmap_dim, 1, 1, EF_ABGR8, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);

		displacement_fb_ = rf.MakeFrameBuffer();
		displacement_fb_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*displacement_tex_, 0, 1, 0));
		gradient_fb_ = rf.MakeFrameBuffer();
		gradient_fb_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*gradient_tex_, 0, 1, 0));

		out_real_tex_ = rf.MakeTexture2D(param_.dmap_dim, param_.dmap_dim, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
		out_imag_tex_ = rf.MakeTexture2D(param_.dmap_dim, param_.dmap_dim, 1, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
		tex_fb_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*out_real_tex_, 0, 1, 0));
		tex_fb_->Attach(FrameBuffer::ATT_Color1, rf.Make2DRenderView(*out_imag_tex_, 0, 1, 0));

		uint32_t actual_dim = param_.dmap_dim;
		uint32_t input_width = actual_dim + 4;
		uint32_t output_width = actual_dim;
		uint32_t output_height = actual_dim;
		uint32_t dtx_offset = actual_dim * actual_dim;
		uint32_t dty_offset = actual_dim * actual_dim * 2;

		RenderEffect& effect = update_spectrum_tech_->Effect();
		*(effect.ParameterByName("actual_dim")) = actual_dim;
		*(effect.ParameterByName("in_width")) = input_width;
		*(effect.ParameterByName("out_width")) = output_width;
		*(effect.ParameterByName("out_height")) = output_height;
		*(effect.ParameterByName("dx_addr_offset")) = dtx_offset;
		*(effect.ParameterByName("dy_addr_offset")) = dty_offset;
		*(effect.ParameterByName("input_h0")) = h0_buffer_;
		*(effect.ParameterByName("input_omega")) = omega_buffer_;
		*(effect.ParameterByName("choppy_scale")) = param_.choppy_scale;
		*(effect.ParameterByName("grid_len")) = param_.dmap_dim / param_.patch_length;
		*(effect.ParameterByName("displacement_tex")) = displacement_tex_;
		*(effect.ParameterByName("dxyz_tex")) = out_real_tex_;

		fft_ = MakeSharedPtr<GpuFftPS>(params.dmap_dim, params.dmap_dim, false);
	}
}
