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

		static Matrix4 matOldWorld, matOldProj, matOldView;
		if (vb.vertexOptions & VertexBuffer::VO_2D)
		{
			Matrix4 matWorld;
			Engine::MathInstance().Translation(matWorld, static_cast<float>(-(*activeRenderTarget_)->Width() / 2),
				static_cast<float>((*activeRenderTarget_)->Height() / 2), 0);
			matOldWorld = this->WorldMatrix();
			this->WorldMatrix(matWorld);

			Matrix4 matProj;
			Engine::MathInstance().OrthoLH(matProj, static_cast<float>((*activeRenderTarget_)->Width()),
				static_cast<float>((*activeRenderTarget_)->Height()), 0, 1);
			matOldProj = this->ProjectionMatrix();
			this->ProjectionMatrix(matProj);

			matOldView = this->ViewMatrix();
			this->ViewMatrix(Matrix4::Identity());

			float* vertices(vb.pVertices);
			for (U32 i = 0; i < vb.numVertices; ++ i)
			{
				vertices[1] = -vertices[1];

				vertices += 3;
				vertices = reinterpret_cast<float*>(reinterpret_cast<U8*>(vertices) + vb.vertexStride);
			}
		}

		this->DoRender(vb);

		if (vb.vertexOptions & VertexBuffer::VO_2D)
		{
			this->WorldMatrix(matOldWorld);
			this->ProjectionMatrix(matOldProj);
			this->ViewMatrix(matOldView);
		}
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
