/**
 * @file SceneComponent.hpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KlayGE
 * For the latest info, see http://www.klayge.org
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You may alternatively use this source under the terms of
 * the KlayGE Proprietary License (KPL). You can obtained such a license
 * from http://www.klayge.org/licensing/.
 */

#ifndef KLAYGE_CORE_SCENE_COMPONENT_HPP
#define KLAYGE_CORE_SCENE_COMPONENT_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#include <boost/type_index.hpp>
#include <boost/type_index/runtime_cast.hpp>

#include <KlayGE/Signal.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API SceneComponent : boost::noncopyable
	{
	public:
#if defined(KLAYGE_COMPILER_CLANGCL)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#endif
		BOOST_TYPE_INDEX_REGISTER_RUNTIME_CLASS(BOOST_TYPE_INDEX_NO_BASE_CLASS)
#if defined(KLAYGE_COMPILER_CLANGCL)
#pragma clang diagnostic pop
#endif

		virtual ~SceneComponent() noexcept;

		virtual SceneComponentPtr Clone() const = 0;

		template <typename T>
		bool IsOfType() const
		{
			return (boost::typeindex::runtime_cast<T const*>(this) != nullptr);
		}

		virtual void BindSceneNode(SceneNode* node);
		SceneNode* BoundSceneNode() const;

		using UpdateEvent = Signal::Signal<void(SceneComponent&, float, float)>;
		UpdateEvent& OnSubThreadUpdate()
		{
			return sub_thread_update_event_;
		}
		UpdateEvent& OnMainThreadUpdate()
		{
			return main_thread_update_event_;
		}

		virtual void SubThreadUpdate(float app_time, float elapsed_time);
		virtual void MainThreadUpdate(float app_time, float elapsed_time);

		bool Enabled() const;
		void Enabled(bool enabled);

	protected:
		SceneNode* node_ = nullptr;
		bool enabled_ = true;

		UpdateEvent sub_thread_update_event_;
		UpdateEvent main_thread_update_event_;
	};
}

#endif		// KLAYGE_CORE_SCENE_COMPONENT_HPP
