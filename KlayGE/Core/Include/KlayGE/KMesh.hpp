// KMesh.hpp
// KlayGE KMesh类 头文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2005-2010
// Homepage: http://klayge.sourceforge.net
//
// 3.10.0
// 支持32-bit index (2010.2.28)
//
// 3.9.0
// 多线程模型载入 (2009.12.13)
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

#pragma once

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
		KMesh(RenderModelPtr const & model, std::wstring const & name);
		virtual ~KMesh();

		virtual void BuildMeshInfo();

		virtual void OnRenderBegin();
		void SetModelMatrix(float4x4 const & model);

	private:
		float4x4 model_matrix_;

		RenderEffectParameterPtr texSampler_ep_;
		RenderEffectParameterPtr modelviewproj_ep_;
		RenderEffectParameterPtr modelIT_ep_;
	};

	template <typename T>
	struct CreateKMeshFactory
	{
		StaticMeshPtr operator()(RenderModelPtr const & model, std::wstring const & name)
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

	KLAYGE_CORE_API void LoadModel(std::string const & meshml_name, std::vector<RenderModel::Material>& mtls,
		std::vector<std::string>& mesh_names, std::vector<int32_t>& mtl_ids, std::vector<std::vector<vertex_element> >& ves,
		std::vector<uint32_t>& max_num_blends, std::vector<std::vector<std::vector<uint8_t> > >& buffs,
		std::vector<char>& is_index_16_bit, std::vector<std::vector<uint8_t> >& indices,
		std::vector<Joint>& joints, boost::shared_ptr<KeyFramesType>& kfs,
		int32_t& start_frame, int32_t& end_frame, int32_t& frame_rate);
	KLAYGE_CORE_API boost::function<RenderModelPtr()> LoadModel(std::string const & meshml_name, uint32_t access_hint,
		boost::function<RenderModelPtr (std::wstring const &)> CreateModelFactoryFunc = CreateKModelFactory<RenderModel>(),
		boost::function<StaticMeshPtr (RenderModelPtr, std::wstring const &)> CreateMeshFactoryFunc = CreateKMeshFactory<StaticMesh>());

	KLAYGE_CORE_API void SaveModel(std::string const & meshml_name, std::vector<RenderModel::Material> const & mtls,
		std::vector<std::string> const & mesh_names, std::vector<int32_t> const & mtl_ids, std::vector<std::vector<vertex_element> > const & ves,
		std::vector<std::vector<std::vector<uint8_t> > > const & buffs,
		std::vector<char> const & is_index_16_bit, std::vector<std::vector<uint8_t> > const & indices,
		std::vector<Joint> const & joints, boost::shared_ptr<KeyFramesType> const & kfs,
		int32_t start_frame, int32_t end_frame, int32_t frame_rate);
	KLAYGE_CORE_API void SaveModel(RenderModelPtr const & model, std::string const & meshml_name);
}

#endif			// _KMESH_HPP
