// SATPostProcess.hpp
// KlayGE Summed-Area Table后期处理类 头文件
// Ver 3.7.0
// 版权所有(C) 龚敏敏, 2007
// Homepage: http://klayge.sourceforge.net
//
// 3.7.0
// 初次建立 (2006.10.10)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _SATPOSTPROCESS_HPP
#define _SATPOSTPROCESS_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#include <KlayGE/PostProcess.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API SATSeparableScanSweepPostProcess : public PostProcess
	{
	public:
		SATSeparableScanSweepPostProcess(RenderTechniquePtr tech, bool dir);

		void ChildBuffer(TexturePtr const & tex);
		void Length(int32_t length);
		void AddrOffset(float3 offset);
		void Scale(float scale);

	private:
		int32_t length_;
		bool dir_;

		RenderEffectParameterPtr child_tex_ep_;
		RenderEffectParameterPtr addr_offset_ep_;
		RenderEffectParameterPtr length_ep_;
		RenderEffectParameterPtr scale_ep_;
	};

	class KLAYGE_CORE_API SummedAreaTablePostProcess : public PostProcess
	{
	public:
		SummedAreaTablePostProcess();

		void InputPin(uint32_t index, TexturePtr const & tex);
		TexturePtr const & InputPin(uint32_t index) const;
		void Apply();

		TexturePtr SATTexture();

	private:
		std::vector<TexturePtr> inter_tex_x_up_;
		std::vector<TexturePtr> inter_tex_x_down_;
		std::vector<TexturePtr> inter_tex_y_up_;
		std::vector<TexturePtr> inter_tex_y_down_;

		SATSeparableScanSweepPostProcess scan_x_up_;
		SATSeparableScanSweepPostProcess scan_x_down_;
		SATSeparableScanSweepPostProcess scan_y_up_;
		SATSeparableScanSweepPostProcess scan_y_down_;
	};
}

#endif		// _SATPOSTPROCESS_HPP
