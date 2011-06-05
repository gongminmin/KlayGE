// MeshExtractor.hpp
// KlayGE MeshML数据导出类 头文件
// Ver 3.11.0
// 版权所有(C) 龚敏敏, 2003-2010
// Homepage: http://www.klayge.org
//
// 3.11.0
// 初次建立 (王锐 2011.2.28)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _MESHEXTRACTOR_HPP
#define _MESHEXTRACTOR_HPP

#pragma once

#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <stdarg.h>

namespace KlayGE
{
	class MeshExtractor
	{
	public:
		template<typename Type, unsigned int Size>
		class TemplatePoint
		{
		public:
			TemplatePoint()
			{
				this->Set(NULL);
			}
			TemplatePoint(Type const * ptr)
			{
				this->Set(ptr);
			}

			TemplatePoint(size_t num, ...)
			{
				va_list argptr;
				va_start(argptr, num);
				this->Set(num, argptr);
			}

			Type& operator[](int i)
			{
				return value_[i];
			}
			Type const & operator[](int i) const
			{
				return value_[i];
			}

			TemplatePoint& operator=(TemplatePoint const & rhs)
			{
				for (size_t i = 0; i < Size; ++ i)
				{
					value_[i] = rhs.value_[i];
				}
				return *this;
			}

			bool operator==(TemplatePoint const & rhs) const
			{
				bool is_same = true;
				for (size_t i = 0; i < Size; ++ i)
				{
					if (value_[i] != rhs.value_[i])
					{
						is_same = false;
					}
				}
				return is_same;
			}

			void Set(Type const * ptr)
			{
				if (!ptr)
				{
					for (size_t i = 0; i < Size; ++ i)
					{
						value_[i] = static_cast<Type>(0);
					}
				}
				else
				{
					for (size_t i = 0; i < Size; ++ i)
					{
						value_[i] = (*ptr + i);
					}
				}
			}

			void Set(size_t num, va_list argptr)
			{
				for (size_t i = 0; i < num; ++ i)
				{
					value_[i] = va_arg(argptr, Type);
				}
				va_end(argptr);
			}

		private:
			Type value_[Size];
		};

		typedef TemplatePoint<double, 2> Point2;
		typedef TemplatePoint<double, 3> Point3;
		typedef TemplatePoint<double, 4> Point4;
		typedef Point3 TexCoord;
		typedef Point4 Color;
		typedef Point4 Quat;

	public:
		typedef std::pair<std::string, std::string> TextureSlot;
		typedef std::pair<std::string, float> JointBinding;

		struct MaterialStruct
		{
			Color ambient;
			Color diffuse;
			Color specular;
			Color emit;
			float opacity;
			float specular_level;
			float shininess;
			std::vector<TextureSlot> texture_slots;

			bool operator==(MaterialStruct const & rhs) const
			{
				return (ambient == rhs.ambient) && (diffuse == rhs.diffuse)
					&& (specular == rhs.specular) && (emit == rhs.emit)
					&& (opacity == rhs.opacity) && (shininess == rhs.shininess)
					&& (specular_level == rhs.specular_level);
			}
		};

		struct VertexStruct
		{
			Point3 position;
			Point3 normal;
			Point3 tangent;
			Point3 binormal;
			int texcoord_components;
			std::vector<TexCoord> texcoords;
			std::vector<JointBinding> binds;
		};

		struct TriangleStruct
		{
			int vertex_index[3];

			bool operator==(TriangleStruct const & rhs) const
			{
				return (vertex_index[0] == rhs.vertex_index[0])
					&& (vertex_index[1] == rhs.vertex_index[1])
					&& (vertex_index[2] == rhs.vertex_index[2]);
			}
		};

		struct MeshStruct
		{
			size_t material_id;
			std::string name;
			std::vector<VertexStruct> vertices;
			std::vector<TriangleStruct> triangles;
		};

		struct JointStruct
		{
			int joint_id;  // Will be set by WriteJointChunk() internally
			std::string parent_name;
			Point3 position;
			Quat quaternion;
		};
		typedef std::map<std::string, JointStruct> JointMap;

		struct KeyframeStruct
		{
			std::string joint;
			std::vector<Point3> positions;
			std::vector<Quat> quaternions;
		};

	public:
		MeshExtractor(std::ostream* out = 0);

		void StartFrame(int sf)
		{
			start_frame_ = sf;
		}
		int StartFrame() const
		{
			return start_frame_;
		}

		void EndFrame(int ef)
		{
			end_frame_ = ef;
		}
		int EndFrame() const
		{
			return end_frame_;
		}

		void FrameRate(int fr)
		{
			frame_rate_ = fr;
		}
		int FrameRate() const
		{
			return frame_rate_;
		}

		enum VertexExportSetting
		{
			VES_NONE = 0,
			VES_NORMAL = 0x1,
			VES_TANGENT = 0x2,
			VES_BINORMAL = 0x4,
			VES_TEXCOORD = 0x8,
			VES_ALL = 0xff
		};
		void VertexExportSettings(int ves)
		{
			vertexExportSettings_ = ves;
		}
		int VertexExportSettings() const
		{
			return vertexExportSettings_;
		}

		enum UserExportSetting
		{
			UES_NONE = 0,
			UES_COMBINE_MESHES = 0x1,
			UES_SORT_MESHES = 0x2,
			UES_ALL = 0xff
		};
		void UserExportSettings(int ues)
		{
			userExportSettings_ = ues;
		}
		int UserExportSettings() const
		{
			return userExportSettings_;
		}

		void Output(std::ostream* out)
		{
			output_stream_ = out;
		}
		std::ostream* Output()
		{
			return output_stream_;
		}

		void AddJoint(std::string const & name, JointStruct const & joint)
		{
			joints_[name] = joint;
		}
		void AddMaterial(MaterialStruct const & material)
		{
			obj_materials_.push_back(material);
		}
		void AddMesh(MeshStruct const & mesh )
		{
			obj_meshes_.push_back(mesh);
		}
		void AddKeyframe(KeyframeStruct const & kf )
		{
			keyframes_.push_back(kf);
		}

		virtual void OptimizeData();
		virtual void WriteMeshML(int modelVersion, std::string const & encoding = std::string());

		virtual void WriteJointChunk();
		virtual void WriteMaterialChunk();
		virtual void WriteMeshChunk();
		virtual void WriteKeyframeChunk();

	private:
		void OptimizeJoints();
		void OptimizeMaterials();
		void OptimizeMeshes();

		int start_frame_;
		int end_frame_;
		int frame_rate_;

		int vertexExportSettings_;
		int userExportSettings_;
		std::ostream* output_stream_;

		JointMap joints_;
		std::vector<MaterialStruct> obj_materials_;
		std::vector<MeshStruct> obj_meshes_;
		std::vector<KeyframeStruct> keyframes_;
	};
}

#endif  // _MESHEXTRACTOR_HPP
