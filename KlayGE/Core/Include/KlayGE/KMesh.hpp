// KMesh.hpp
// KlayGE KMesh类 头文件
// Ver 3.4.0
// 版权所有(C) 龚敏敏, 2005-2006
// Homepage: http://klayge.sourceforge.net
//
// 3.4.0
// 支持蒙皮模型的载入和保存 (2006.8.23)
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

#ifndef KLAYGE_CORE_SOURCE
#define KLAYGE_LIB_NAME KlayGE_Core
#include <KlayGE/config/auto_link.hpp>
#endif

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/Mesh.hpp>

#include <boost/function.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API KMesh : public StaticMesh
	{
	public:
		KMesh(RenderModelPtr model, std::wstring const & name);
		virtual ~KMesh();

		virtual void BuildMeshInfo();

		virtual void OnRenderBegin();
		void SetModelMatrix(float4x4 const & model);

	private:
		float4x4 model_;

		RenderEffectParameterPtr texSampler_ep_;
		RenderEffectParameterPtr modelviewproj_ep_;
		RenderEffectParameterPtr modelIT_ep_;
	};

	template <typename T>
	struct CreateKMeshFactory
	{
		StaticMeshPtr operator()(RenderModelPtr model, std::wstring const & name)
		{
			return MakeSharedPtr<T>(model, name);
		}
	};

	template <typename T>
	struct CreateKModelFactory
	{
		RenderModelPtr operator()(std::wstring const & name)
		{
			return MakeSharedPtr<T>(name);
		}
	};

#ifdef KLAYGE_PLATFORM_WINDOWS
#pragma pack(push, 1)
#endif
	struct KModelHeader
	{
		uint32_t version;
		uint8_t num_meshes;
		uint8_t num_joints;
		uint8_t num_key_frames;
		uint32_t start_frame;
		uint32_t end_frame;
		uint32_t frame_rate;
	};
#ifdef KLAYGE_PLATFORM_WINDOWS
#pragma pack(pop)
#endif

	KLAYGE_CORE_API RenderModelPtr LoadKModel(std::string const & kmodel_name, uint32_t access_hint,
		boost::function<RenderModelPtr (std::wstring const &)> CreateModelFactoryFunc = CreateKModelFactory<RenderModel>(),
		boost::function<StaticMeshPtr (RenderModelPtr, std::wstring const &)> CreateMeshFactoryFunc = CreateKMeshFactory<StaticMesh>());
	KLAYGE_CORE_API void SaveKModel(RenderModelPtr model, std::string const & kmodel_name);
}

#endif			// _KMESH_HPP
