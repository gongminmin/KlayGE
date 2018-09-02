/**
 * @file ToolCommonLoader.hpp
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

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ToolCommonLoader.hpp>

#if KLAYGE_IS_DEV_PLATFORM

namespace KlayGE
{
	ToolCommonLoader::ToolCommonLoader()
	{
		std::string const tool_common_name = DLL_PREFIX "ToolCommon" DLL_SUFFIX;

		dll_loader_.Load(tool_common_name);

		DynamicConvertModel_ = reinterpret_cast<ConvertModelFunc>(dll_loader_.GetProcAddress("ConvertModel"));
		DynamicConvertTexture_ = reinterpret_cast<ConvertTextureFunc>(dll_loader_.GetProcAddress("ConvertTexture"));
	}

	ToolCommonLoader& ToolCommonLoader::Instance()
	{
		static ToolCommonLoader ret;
		return ret;
	}

	void ToolCommonLoader::ConvertModel(std::string_view input_name, std::string_view metadata_name, std::string_view output_name,
		RenderDeviceCaps const * caps)
	{
		DynamicConvertModel_(input_name, metadata_name, output_name, caps);
	}

	void ToolCommonLoader::ConvertTexture(std::string_view input_name, std::string_view metadata_name, std::string_view output_name,
		RenderDeviceCaps const * caps)
	{
		DynamicConvertTexture_(input_name, metadata_name, output_name, caps);
	}
}

#endif
