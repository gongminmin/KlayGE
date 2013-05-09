/**
 * @file NV3DVision.hpp
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

#ifndef _KLAYGE_NV3DVISION_HPP
#define _KLAYGE_NV3DVISION_HPP

#pragma once

// Stereo Blit defines
#define NVSTEREO_IMAGE_SIGNATURE 0x4433564E		//NV3D

struct NVSTEREOIMAGEHEADER
{
	unsigned int dwSignature;
	unsigned int dwWidth;
	unsigned int dwHeight;
	unsigned int dwBPP;
	unsigned int dwFlags;
};

// Flags in the dwFlags fiels of the _Nv_Stereo_Image_Header structure above
#define     SIH_SWAP_EYES               0x00000001
#define     SIH_SCALE_TO_FIT            0x00000002

#endif		// _KLAYGE_NV3DVISION_HPP
