#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Math.hpp>

#include <algorithm>
#include <boost/mem_fn.hpp>

#include <KlayGE/Mesh.hpp>

namespace KlayGE
{
	StaticMesh::StaticMesh()
		: vb_(new VertexBuffer(VertexBuffer::BT_TriangleList))
	{
	}

	StaticMesh::~StaticMesh()
	{
	}

	std::wstring const & StaticMesh::Name() const
	{
		static std::wstring name(L"Static Mesh");
		return name;
	}

	void StaticMesh::Render()
	{
		std::for_each(children_.begin(), children_.end(), boost::mem_fn(&StaticMesh::Render));
		Renderable::Render();
	}


	BoneMesh::~BoneMesh()
	{
	}

	std::wstring const & BoneMesh::Name() const
	{
		static std::wstring name(L"Bone Mesh");
		return name;
	}
}
