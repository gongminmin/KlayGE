// RenderEngine.cpp
// KlayGE 渲染引擎类 实现文件
// Ver 2.0.3
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
//
// 2.0.3
// 优化了RenderEffect的设置 (2004.2.16)
// 去掉了VO_2D (2004.3.1)
//
// 2.0.0
// 初次建立(2003.10.1)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/SharePtr.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Engine.hpp>
#include <KlayGE/Viewport.hpp>
#include <KlayGE/RenderTarget.hpp>
#include <KlayGE/VertexBuffer.hpp>
#include <KlayGE/RenderEffect.hpp>

#include <KlayGE/RenderEngine.hpp>

namespace KlayGE
{
	RenderEngine::RenderEngine()
		: renderEffect_(NullRenderEffectInstance()),
			renderPasses_(1)
	{
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	RenderEngine::~RenderEngine()
	{
	}

	// 增加显示目标
	/////////////////////////////////////////////////////////////////////////////////
	RenderEngine::RenderTargetListIterator RenderEngine::AddRenderTarget(const RenderTargetPtr& target)
	{
		renderTargetList_.push_back(target);
		RenderTargetListIterator iter(renderTargetList_.end());
		-- iter;

		return iter;
	}

	// 显示目标列表的Begin迭代器
	/////////////////////////////////////////////////////////////////////////////////
	RenderEngine::RenderTargetListIterator RenderEngine::RenderTargetListBegin()
	{
		return renderTargetList_.begin();
	}

	// 显示目标列表的End迭代器
	/////////////////////////////////////////////////////////////////////////////////
	RenderEngine::RenderTargetListIterator RenderEngine::RenderTargetListEnd()
	{
		return renderTargetList_.end();
	}

	// 从显示目标列表中删除显示目标
	/////////////////////////////////////////////////////////////////////////////////
	RenderTargetPtr RenderEngine::RemoveRenderTarget(RenderTargetListIterator iter)
	{
		RenderTargetPtr ret(*iter);
		renderTargetList_.erase(iter);
		return ret;
	}

	// 设置当前显示目标
	/////////////////////////////////////////////////////////////////////////////////
	void RenderEngine::ActiveRenderTarget(RenderTargetListIterator iter)
	{
		activeRenderTarget_ = iter;
	}

	// 设置渲染状态
	/////////////////////////////////////////////////////////////////////////////////
	void RenderEngine::SetRenderEffect(const RenderEffectPtr& effect)
	{
		if (renderEffect_ != effect)
		{
			renderEffect_->End();
			renderEffect_ = (!effect) ? NullRenderEffectInstance() : effect;
			renderPasses_ = renderEffect_->Begin();
		}
	}

	// 渲染顶点缓冲区
	/////////////////////////////////////////////////////////////////////////////////
	void RenderEngine::Render(VertexBuffer& vb)
	{
		// Vertex blending: do software if required
		if ((vb.vertexOptions & VertexBuffer::VO_BlendWeights) && 
			(this->MaxVertexBlendMatrices() <= 1))
		{
			// Software blending required
			this->SoftwareVertexBlend(vb);

			vb.vertexOptions &= ~VertexBuffer::VO_BlendWeights;
		}

		this->DoRender(vb);
	}

	// 获取最大坐标数
	/////////////////////////////////////////////////////////////////////////////////
	U32 RenderEngine::MaxVertexBlendMatrices()
	{
		// TODO: implement vertex blending support in DX8 & possibly GL_ARB_VERTEX_BLEND (in subclasses)
		return 1;
	}

	void RenderEngine::SoftwareVertexBlend(VertexBuffer& vb)
	{
		// Source vector
		Vector3 sourceVec;
		// Accumulation vectors
		Vector3 accumVecPos, accumVecNorm;

		// Check buffer size
		U32 numVertReals(vb.numVertices * 3);
		tempVertexBlendBuffer.resize(numVertReals);
		if (vb.vertexOptions & VertexBuffer::VO_Normals)
		{
			tempNormalBlendBuffer.resize(numVertReals);
		}

		const MathLib& math(Engine::MathInstance());

		// Loop per vertex
		float* pVertElem(vb.pVertices);
		float* pNormElem(vb.pNormals);
		VertexBuffer::VertexBlendData* pBlend(vb.pBlendingWeights);
		for (U32 vertIdx = 0; vertIdx < numVertReals; vertIdx += 3)
		{
			// Load source vertex elements
			sourceVec = Vector3(pVertElem);
			pVertElem += 3;

			if (vb.vertexOptions & VertexBuffer::VO_Normals)
			{
				accumVecNorm = Vector3(pNormElem);
				pNormElem += 3;
			}

			// Load accumulator
			accumVecPos = Vector3::Zero();

			// Loop per blend weight 
			for (U16 blendIdx = 0; blendIdx < pBlend->weightCount; ++ blendIdx)
			{
				// Blend by multiplying source by blend matrix and scaling by weight
				// Add to accumulator
				// NB weights must be normalised!!
				if (!math.Eq(pBlend->blendWeight, 0.0f))
				{
					// Blend position
					math.TransformCoord(sourceVec, sourceVec, worldMatrices[pBlend->weightIndex]);
					accumVecPos += sourceVec * pBlend->blendWeight;

					if (vb.vertexOptions & VertexBuffer::VO_Normals)
					{
						// Blend normal
						// We should blend by inverse transform here, but because we're assuming the 3x3
						// aspect of the matrix is orthogonal (no non-uniform scaling), the inverse transpose
						// is equal to the main 3x3 matrix
						// Note because it's a normal we just extract the rotational part, saves us renormalising
						math.TransformCoord(accumVecNorm, accumVecNorm,
							worldMatrices[pBlend->weightIndex] * pBlend->blendWeight);
					}

				}

				++ pBlend;
			}

			// Stored blended vertex in temp buffer
			tempVertexBlendBuffer[vertIdx + 0] = accumVecPos.x();
			tempVertexBlendBuffer[vertIdx + 1] = accumVecPos.y();
			tempVertexBlendBuffer[vertIdx + 2] = accumVecPos.z();

			// Stored blended vertex in temp buffer
			tempNormalBlendBuffer[vertIdx + 0] = accumVecNorm.x();
			tempNormalBlendBuffer[vertIdx + 1] = accumVecNorm.y();
			tempNormalBlendBuffer[vertIdx + 2] = accumVecNorm.z();
		}

		// Re-point the render operation vertex buffer
		vb.pVertices = &tempVertexBlendBuffer[0];
		if (vb.vertexOptions & VertexBuffer::VO_Normals)
		{
			vb.pNormals = &tempNormalBlendBuffer[0];
		}
	}

	void RenderEngine::WorldMatrices(Matrix4* mats, size_t count)
	{
		worldMatrices.clear();
		for (size_t i = 0; i < count; ++ i)
		{
			worldMatrices.push_back(mats[i]);
		}
		WorldMatrix(Matrix4::Identity());
	}
}
