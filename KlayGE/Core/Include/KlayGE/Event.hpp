// Event.hpp
// KlayGE 事件模板 头文件
// Ver 2.7.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
//
// 2.7.0
// 初次建立 (2005.6.14)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _EVENT_HPP
#define _EVENT_HPP

#include <list>
#include <boost/function.hpp>

namespace KlayGE
{
	template <typename Sender, typename EventArg>
	class Event
	{
	public:
		typedef boost::function<void (Sender&, EventArg&)>, EventHandler;

	public:
		Event(Sender& sender)
			: sender_(&sender)
		{
		}

		void operator+=(EventHandler const & handler)
		{
			handlers_.push_back(handler);
		}

		void operator()(EventArg& arg)
		{
			for (std::list<EventHandler>::iterator iter = handlers_.begin();
				iter != handlers_.end(); ++ iter)
			{
				(*iter)(*sender, arg);
			}
		}

	private:
		Sender* sender_;
		std::list<EventHandler> handlers_;
	};
}

#endif			// _EVENT_HPP
