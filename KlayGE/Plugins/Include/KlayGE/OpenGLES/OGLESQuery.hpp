// OGLESQuery.hpp
// KlayGE OpenGL ES�ڵ������ ʵ���ļ�
// Ver 3.0.0
// ��Ȩ����(C) ������, 2005
// Homepage: http://www.klayge.org
//
// 3.0.0
// ���ν��� (2005.9.27)
//
// �޸ļ�¼
//////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLESQUERY_HPP
#define _OGLESQUERY_HPP

#pragma once

#include <KlayGE/Query.hpp>

namespace KlayGE
{
	class OGLESConditionalRender final : public ConditionalRender
	{
	public:
		OGLESConditionalRender();
		~OGLESConditionalRender() override;

		void Begin() override;
		void End() override;

		void BeginConditionalRender() override;
		void EndConditionalRender() override;

		bool AnySamplesPassed() override;

	private:
		GLuint query_;
	};

	class OGLESTimerQuery final : public TimerQuery
	{
	public:
		OGLESTimerQuery();
		~OGLESTimerQuery() override;

		void Begin() override;
		void End() override;

		double TimeElapsed() override;

	private:
		GLuint query_;
	};

	class OGLESSOStatisticsQuery final : public SOStatisticsQuery
	{
	public:
		OGLESSOStatisticsQuery();
		~OGLESSOStatisticsQuery() override;

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
