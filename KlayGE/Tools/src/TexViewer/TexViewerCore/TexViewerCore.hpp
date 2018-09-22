#ifndef _TEX_VIEWER_CORE_HPP
#define _TEX_VIEWER_CORE_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Texture.hpp>

#ifdef KLAYGE_MTL_EDITOR_CORE_SOURCE		// Build dll
#define KLAYGE_MTL_EDITOR_CORE_API KLAYGE_SYMBOL_EXPORT
#else							// Use dll
#define KLAYGE_MTL_EDITOR_CORE_API KLAYGE_SYMBOL_IMPORT
#endif

namespace KlayGE
{
	class KLAYGE_MTL_EDITOR_CORE_API TexViewerCore : public App3DFramework
	{
	public:
		explicit TexViewerCore(void* native_wnd);

		void Resize(uint32_t width, uint32_t height);

		void OpenTexture(std::string const & name);
		void SaveAsTexture(std::string const & name);

		Texture::TextureType TextureType() const;
		uint32_t ArraySize() const;
		uint32_t NumMipmaps() const;
		uint32_t Width(uint32_t mipmap) const;
		uint32_t Height(uint32_t mipmap) const;
		uint32_t Depth(uint32_t mipmap) const;
		ElementFormat Format() const;

		void ArrayIndex(uint32_t array_index);
		void Face(uint32_t face);
		void DepthIndex(uint32_t depth_index);
		void MipmapLevel(uint32_t mipmap);
		void Stops(float value);
		void OffsetAndZoom(float x, float y, float zoom);
		void ColorMask(bool r, bool g, bool b, bool a);

		float4 RetrieveColor(uint32_t x, uint32_t y);

	private:
		virtual void OnCreate() override;
		virtual void OnDestroy() override;
		virtual void OnResize(uint32_t width, uint32_t height) override;
		virtual void DoUpdateOverlay() override;
		virtual uint32_t DoUpdate(uint32_t pass) override;

		void UpdateDisplayTexture();
		void UpdateQuadMatrix();

	private:
		FontPtr font_;

		SceneNodePtr quad_so_;
		RenderablePtr quad_;
		TexturePtr texture_original_;
		TexturePtr texture_display_;
		std::vector<float4> texels_;

		std::string last_file_path_;

		uint32_t active_array_index_;
		uint32_t active_face_;
		uint32_t active_depth_index_;
		uint32_t active_mipmap_level_;
		float offset_x_;
		float offset_y_;
		float zoom_;
		float stops_;
	};
}

#endif		// _TEX_VIEWER_CORE_HPP
