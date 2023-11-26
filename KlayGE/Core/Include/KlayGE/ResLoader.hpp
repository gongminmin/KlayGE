/**
 * @file ResLoader.hpp
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

#pragma once

#include <memory>
#include <string>
#include <string_view>

#include <KFL/ResIdentifier.hpp>
#include <KFL/Noncopyable.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API ResLoadingDesc
	{
		KLAYGE_NONCOPYABLE(ResLoadingDesc);

	public:
		ResLoadingDesc() noexcept;
		virtual ~ResLoadingDesc() noexcept;

		virtual uint64_t Type() const = 0;

		virtual bool StateLess() const = 0;

		virtual std::shared_ptr<void> CreateResource()
		{
			return std::shared_ptr<void>();
		}
		virtual void SubThreadStage() = 0;
		virtual void MainThreadStage() = 0;

		virtual bool HasSubThreadStage() const = 0;

		virtual bool Match(ResLoadingDesc const & rhs) const = 0;
		virtual void CopyDataFrom(ResLoadingDesc const & rhs) = 0;
		virtual std::shared_ptr<void> CloneResourceFrom(std::shared_ptr<void> const & resource) = 0;

		virtual std::shared_ptr<void> Resource() const = 0;
	};

	using ResLoadingDescPtr = std::shared_ptr<ResLoadingDesc>;

	class KLAYGE_CORE_API ResLoader final
	{
		KLAYGE_NONCOPYABLE(ResLoader);

	public:
		ResLoader();
		~ResLoader();

		static ResLoader& Instance();
		static void Destroy() noexcept;

		void Suspend();
		void Resume();

		void AddPath(std::string_view phy_path);
		void DelPath(std::string_view phy_path);
		bool IsInPath(std::string_view phy_path);
		std::string const& LocalFolder() const noexcept;

		void Mount(std::string_view virtual_path, std::string_view phy_path);
		void Unmount(std::string_view virtual_path, std::string_view phy_path);

		ResIdentifierPtr Open(std::string_view name);
		std::string Locate(std::string_view name);
		uint64_t Timestamp(std::string_view name);
		std::string AbsPath(std::string_view path);

		std::shared_ptr<void> SyncQuery(ResLoadingDescPtr const & res_desc);
		std::shared_ptr<void> ASyncQuery(ResLoadingDescPtr const & res_desc);
		void Unload(std::shared_ptr<void> const & res);

		template <typename T>
		std::shared_ptr<T> SyncQueryT(ResLoadingDescPtr const & res_desc)
		{
			return std::static_pointer_cast<T>(this->SyncQuery(res_desc));
		}

		template <typename T>
		std::shared_ptr<T> ASyncQueryT(ResLoadingDescPtr const & res_desc)
		{
			return std::static_pointer_cast<T>(this->ASyncQuery(res_desc));
		}

		template <typename T>
		void Unload(std::shared_ptr<T> const & res)
		{
			this->Unload(std::static_pointer_cast<void>(res));
		}

		void Update();

		uint32_t NumLoadingResources() const noexcept;

	private:
		class Impl;
		std::unique_ptr<Impl> pimpl_;
	};
}
