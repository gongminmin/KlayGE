// PostProcess.hpp
// KlayGE 后期处理类 头文件
// Ver 3.3.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
//
// 3.3.0
// 初次建立 (2006.6.23)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _POSTPROCESS_HPP
#define _POSTPROCESS_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderableHelper.hpp>

namespace KlayGE
{
	class PostProcess : public RenderableHelper
	{
	public:
		explicit PostProcess(RenderTechniquePtr tech);
		virtual ~PostProcess()
		{
		}

		virtual void Source(TexturePtr const & tex);
		virtual void Destinate(RenderTargetPtr const & rt);

		virtual void Apply();

		virtual void OnRenderBegin();

	protected:
		TexturePtr src_texture_;
		RenderTargetPtr render_target_;

		GraphicsBufferPtr pos_vb_;
	};
}

#endif		// _POSTPROCESS_HPP
