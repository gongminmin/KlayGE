#ifndef _PARTICLESYSTEMAPP_HPP
#define _PARTICLESYSTEMAPP_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/ParticleSystem.hpp>
#include <KlayGE/Timer.hpp>
#include <KlayGE/PostProcess.hpp>

#include <boost/function.hpp>

struct Particle
{
	KlayGE::float3 pos;
	KlayGE::float3 vel;
	float life;
	float birth_time;
};

class HeightImg
{
public:
	HeightImg(float start_x, float start_y, float end_x, float end_y, KlayGE::TexturePtr img, float scale)
		: start_x_(start_x), start_y_(start_y), end_x_(end_x), end_y_(end_y), scale_(scale)
	{
		hm_width_ = img->Width(0);
		hm_height_ = img->Height(0);
		heights_.resize(hm_width_ * hm_height_);

		KlayGE::uint8_t* data;
		KlayGE::uint32_t row_pitch;
		img->Map2D(0, KlayGE::TMA_Read_Only, 0, 0, hm_width_, hm_height_, reinterpret_cast<void*&>(data), row_pitch);
		for (KlayGE::uint32_t y = 0; y < hm_height_; ++ y)
		{
			memcpy(&heights_[y * hm_width_], data, hm_width_);
			data += row_pitch;
		}
		img->Unmap2D(0);
	}

	float operator()(float x, float y)
	{
		int const img_x = GetImgX(x);
		int const img_y = GetImgY(y);

		return heights_[img_y * hm_width_ + img_x] / 255.0f * scale_;
	}

	KlayGE::uint32_t HMWidth() const
	{
		return hm_width_;
	}

	KlayGE::uint32_t HMHeight() const
	{
		return hm_height_;
	}

	int GetImgX(float x) const
	{
		float normalized_x = (x - start_x_) / (end_x_ - start_x_);
		return KlayGE::MathLib::clamp(static_cast<KlayGE::uint32_t>(normalized_x * hm_width_ + 0.5f), 0UL, hm_width_ - 1);
	}

	int GetImgY(float y) const
	{
		float normalized_y = (y - start_y_) / (end_y_ - start_y_);
		return KlayGE::MathLib::clamp(static_cast<KlayGE::uint32_t>(normalized_y * hm_height_ + 0.5f), 0UL, hm_height_ - 1);
	}

private:
	float start_x_;
	float start_y_;
	float end_x_;
	float end_y_;

	std::vector<KlayGE::uint8_t> heights_;
	KlayGE::uint32_t hm_width_;
	KlayGE::uint32_t hm_height_;

	float scale_;
};

class ParticleSystemApp : public KlayGE::App3DFramework
{
public:
	ParticleSystemApp(std::string const & name, KlayGE::RenderSettings const & settings);

private:
	void InitObjects();
	void OnResize(KlayGE::uint32_t width, KlayGE::uint32_t height);
	void DoUpdate(KlayGE::uint32_t pass);
	KlayGE::uint32_t NumPasses() const;

	void InputHandler(KlayGE::InputEngine const & sender, KlayGE::InputAction const & action);

	KlayGE::FontPtr font_;

	KlayGE::SceneObjectPtr particles_;
	KlayGE::SceneObjectPtr terrain_;

	KlayGE::FirstPersonCameraController fpcController_;

	boost::shared_ptr<KlayGE::ParticleSystem<Particle> > ps_;

	KlayGE::Timer timer_;

	boost::shared_ptr<HeightImg> height_img_;

	KlayGE::TexturePtr scene_tex_;
	KlayGE::FrameBufferPtr scene_buffer_;

	KlayGE::PostProcessPtr copy_pp_;
};

#endif		// _PARTICLESYSTEMAPP_HPP
