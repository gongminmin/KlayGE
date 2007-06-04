// Event.hpp
// KlayGE 事件模板 头文件
// Ver 3.4.0
// 版权所有(C) 龚敏敏, 2005-2006
// Homepage: http://klayge.sourceforge.net
//
// 3.4.0
// 改用boost::signal实现 (2006.7.19)
//
// 2.7.0
// 初次建立 (2005.6.14)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _EVENT_HPP
#define _EVENT_HPP

#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4103 4512)
#endif
#include <boost/signal.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

namespace KlayGE
{
	template <typename Sender, typename EventArg>
	class Event
	{
		typedef boost::signal<void(Sender const &, EventArg const &)> event_signal;

	public:
		typedef typename event_signal::slot_type event_handler;

	public:
		Event(Sender const & sender)
			: sender_(&sender),
				handlers_(new boost::signal<void(Sender const &, EventArg const &)>)
		{
		}

		void operator+=(event_handler const & handler)
		{
			handlers_->connect(handler);
		}

		void operator-=(event_handler const & handler)
		{
			handlers_->disconnect(handler);
		}

		void operator()(EventArg const & arg)
		{
			(*handlers_)(*sender_, arg);
		}

	private:
		Sender const * sender_;
		boost::shared_ptr<event_signal> handlers_;
	};
}

#endif			// _EVENT_HPP
