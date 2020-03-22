/**
 * @file PlatformDefinition.hpp
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

#ifndef KLAYGE_TOOLS_TOOL_COMMON_PLATFORM_DEFINITION_HPP
#define KLAYGE_TOOLS_TOOL_COMMON_PLATFORM_DEFINITION_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KFL/CXX17/string_view.hpp>
#include <KlayGE/RenderDeviceCaps.hpp>

#include <string>

#include <KlayGE/DevHelper/DevHelper.hpp>

namespace KlayGE
{
	struct KLAYGE_DEV_HELPER_API PlatformDefinition final
	{
		PlatformDefinition();
		explicit PlatformDefinition(std::string_view platform);

		std::string platform;
		uint8_t major_version;
		uint8_t minor_version;

		bool requires_flipping;
		uint32_t native_shader_fourcc;
		uint32_t native_shader_version;

		RenderDeviceCaps device_caps{};

		bool frag_depth_support;
	};
}

#endif		// KLAYGE_TOOLS_TOOL_COMMON_PLATFORM_DEFINITION_HPP
