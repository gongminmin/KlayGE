// SATPostProcess.hpp
// KlayGE Summed-Area Table后期处理类 头文件
// Ver 3.7.0
// 版权所有(C) 龚敏敏, 2007
// Homepage: http://www.klayge.org
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
		SATSeparableScanSweepPostProcess(RenderEffectPtr const & effect, RenderTechnique* tech);

		void ChildBuffer(TexturePtr const & tex);
		void Length(int32_t length);
		void AddrOffset(float3 const & offset);
		void Scale(float scale);

	private:
		int32_t length_;

		RenderEffectParameter* child_tex_ep_;
		RenderEffectParameter* addr_offset_ep_;
		RenderEffectParameter* length_ep_;
		RenderEffectParameter* scale_ep_;
	};

	class KLAYGE_CORE_API SATPostProcess : public PostProcessChain
	{
	public:
		SATPostProcess();

		virtual void InputPin(uint32_t index, TexturePtr const & tex);
		using PostProcessChain::InputPin;
	};


	class KLAYGE_CORE_API SATPostProcessCS : public PostProcessChain
	{
	public:
		SATPostProcessCS();

		virtual void InputPin(uint32_t index, TexturePtr const & tex);
		using PostProcessChain::InputPin;
	};
}

#endif		// _SATPOSTPROCESS_HPP
