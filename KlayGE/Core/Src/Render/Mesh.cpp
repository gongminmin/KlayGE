#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Math.hpp>

#include <KlayGE/Mesh.hpp>

namespace KlayGE
{
	StaticMesh::StaticMesh()
		: rb_(new RenderBuffer(RenderBuffer::BT_TriangleList))
	{
	}

	StaticMesh::~StaticMesh()
	{
	}

	const std::wstring& StaticMesh::Name() const
	{
		static std::wstring name(L"Static Mesh");
		return name;
	}


	BoneMesh::~BoneMesh()
	{
	}

	const std::wstring& BoneMesh::Name() const
	{
		static std::wstring name(L"Bone Mesh");
		return name;
	}
}
