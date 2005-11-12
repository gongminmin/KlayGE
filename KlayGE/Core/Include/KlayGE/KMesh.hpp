// KMesh.hpp
// KlayGE KMesh类 头文件
// Ver 2.7.1
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
//
// 2.7.1
// LoadKMesh可以使用自定义类 (2005.7.13)
//
// 2.7.0
// 初次建立 (2005.6.17)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _KMESH_HPP
#define _KMESH_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/Mesh.hpp>

#include <boost/function.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_Core_d.lib")
#else
	#pragma comment(lib, "KlayGE_Core.lib")
#endif

namespace KlayGE
{
	class KMesh : public StaticMesh
	{
	public:
		KMesh(std::wstring const & name, TexturePtr tex);
		virtual ~KMesh();

		virtual void OnRenderBegin();
		void SetModelMatrix(Matrix4 const & model);

	private:
		SamplerPtr sampler_;
		Matrix4 model_;
	};

	template <typename T>
	struct CreateKMeshFactory
	{
		KMeshPtr operator()(std::wstring const & name, TexturePtr tex)
		{
			return KMeshPtr(new T(name, tex));
		}
	};

	RenderModelPtr LoadKMesh(std::string const & kmeshName,
		boost::function<KMeshPtr (std::wstring const &, TexturePtr)> CreateFactoryFunc = CreateKMeshFactory<KMesh>());
}

#endif			// _KMESH_HPP
