// D3D9IBConverter.cpp
// KlayGE 从KlayGE::VertexBuffer转化成D3D9IB的类 实现文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 2.0.0
// 初次建立 (2003.8.15)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Memory.hpp>
#include <KlayGE/Engine.hpp>
#include <KlaygE/VertexBuffer.hpp>

#include <KlayGE/D3D9/D3D9IBConverter.hpp>

namespace KlayGE
{
	// 连接上d3dDevice
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9IBConverter::Attach(const COMPtr<IDirect3DDevice9>& d3dDevice)
	{
		d3dDevice_ = d3dDevice;
	}

	// 更新D3D9的VertexBuffer
	/////////////////////////////////////////////////////////////////////////////////
	COMPtr<IDirect3DIndexBuffer9> D3D9IBConverter::Update(const VertexBuffer& vb)
	{
		if (indicies_.count < vb.NumIndices())
		{
			d3dDevice_->SetIndices(NULL);

			IDirect3DIndexBuffer9* buffer;
			TIF(d3dDevice_->CreateIndexBuffer(vb.NumIndices() * sizeof(U16),
				D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFMT_INDEX16,
				D3DPOOL_DEFAULT, &buffer, NULL));

			indicies_.buffer = COMPtr<IDirect3DIndexBuffer9>(buffer);
			indicies_.count = vb.NumIndices();
		}

		void* data;
		TIF(indicies_.buffer->Lock(0, 0, &data, D3DLOCK_DISCARD));
		Engine::MemoryInstance().Cpy(data, &vb.indices[0], vb.NumIndices() * sizeof(U16));
		indicies_.buffer->Unlock();

		return indicies_.buffer;
	}
}
