// OGLQuery.hpp
// KlayGE OpenGL�ڵ������ ʵ���ļ�
// Ver 3.0.0
// ��Ȩ����(C) ������, 2005
// Homepage: http://www.klayge.org
//
// 3.0.0
// ���ν��� (2005.9.27)
//
// �޸ļ�¼
//////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLQUERY_HPP
#define _OGLQUERY_HPP

#pragma once

#include <KlayGE/Query.hpp>

namespace KlayGE
{
	class OGLOcclusionQuery final : public OcclusionQuery
	{
	public:
		OGLOcclusionQuery();
		~OGLOcclusionQuery() override;

		void Begin() override;
		void End() override;

		uint64_t SamplesPassed() override;

	private:
		GLuint query_;
	};

	class OGLConditionalRender final : public ConditionalRender
	{
	public:
		OGLConditionalRender();
		~OGLConditionalRender() override;

		void Begin() override;
		void End() override;

		void BeginConditionalRender() override;
		void EndConditionalRender() override;

		bool AnySamplesPassed() override;

	private:
		GLuint query_;
	};

	class OGLTimerQuery final : public TimerQuery
	{
	public:
		OGLTimerQuery();
		~OGLTimerQuery() override;

		void Begin() override;
		void End() override;

		double TimeElapsed() override;

	private:
		GLuint query_;
	};

	class OGLSOStatisticsQuery final : public SOStatisticsQuery
	{
	public:
		OGLSOStatisticsQuery();
		~OGLSOStatisticsQuery() override;

		void Begin() override;
		void End() override;

		uint64_t NumPrimitivesWritten() override;
		uint64_t PrimitivesGenerated() override;

	private:
		GLuint primitive_written_query_;
		GLuint primitive_generated_query_;
	};
}

#endif		// _OGLQUERY_HPP
