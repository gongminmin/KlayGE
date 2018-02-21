/**
* @file TexCompressionETC.cpp
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
#include <KFL/CXX17/iterator.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KFL/Color.hpp>
#include <KlayGE/Texture.hpp>

#include <vector>
#include <cstring>
#include <boost/assert.hpp>

#include <KlayGE/TexCompressionETC.hpp>
#include "../Base/TableGen/Tables.hpp"

namespace
{
	using namespace KlayGE;

	static uint8_t const selector_index_to_etc1[] = { 3, 2, 0, 1 };

	// color8_to_etc_block_config_0_255[color][table_index] = Supplies for each 8-bit color value a list of packed ETC1 diff/intensity table/selectors/packed_colors that map to that color.
	// To pack: diff | (inten << 1) | (selector << 4) | (packed_c << 8)
	static uint16_t const color8_to_etc_block_config_0_255[2][33] =
	{
		{ 0x0000, 0x0010, 0x0002, 0x0012, 0x0004, 0x0014, 0x0006, 0x0016, 0x0008, 0x0018, 0x000A, 0x001A, 0x000C,
			0x001C, 0x000E, 0x001E, 0x0001, 0x0011, 0x0003, 0x0013, 0x0005, 0x0015, 0x0007, 0x0017, 0x0009, 0x0019,
			0x000B, 0x001B, 0x000D, 0x001D, 0x000F, 0x001F, 0xFFFF },
		{ 0x0F20, 0x0F30, 0x0E32, 0x0F22, 0x0E34, 0x0F24, 0x0D36, 0x0F26, 0x0C38, 0x0E28, 0x0B3A, 0x0E2A, 0x093C,
			0x0E2C, 0x053E, 0x0D2E, 0x1E31, 0x1F21, 0x1D33, 0x1F23, 0x1C35, 0x1E25, 0x1A37, 0x1E27, 0x1839, 0x1D29,
			0x163B, 0x1C2B, 0x133D, 0x1B2D, 0x093F, 0x1A2F, 0xFFFF },
	};

	static uint16_t const color8_to_etc_block_config_1_to_254[254][12] =
	{
		{ 0x021C, 0x0D0D, 0xFFFF },
		{ 0x0020, 0x0021, 0x0A0B, 0x061F, 0xFFFF },
		{ 0x0113, 0x0217, 0xFFFF },
		{ 0x0116, 0x031E, 0x0B0E, 0x0405, 0xFFFF },
		{ 0x0022, 0x0204, 0x050A, 0x0023, 0xFFFF },
		{ 0x0111, 0x0319, 0x0809, 0x170F, 0xFFFF },
		{ 0x0303, 0x0215, 0x0607, 0xFFFF },
		{ 0x0030, 0x0114, 0x0408, 0x0031, 0x0201, 0x051D, 0xFFFF },
		{ 0x0100, 0x0024, 0x0306, 0x0025, 0x041B, 0x0E0D, 0xFFFF },
		{ 0x021A, 0x0121, 0x0B0B, 0x071F, 0xFFFF },
		{ 0x0213, 0x0317, 0xFFFF },
		{ 0x0112, 0x0505, 0xFFFF },
		{ 0x0026, 0x070C, 0x0123, 0x0027, 0xFFFF },
		{ 0x0211, 0x0909, 0xFFFF },
		{ 0x0110, 0x0315, 0x0707, 0x0419, 0x180F, 0xFFFF },
		{ 0x0218, 0x0131, 0x0301, 0x0403, 0x061D, 0xFFFF },
		{ 0x0032, 0x0202, 0x0033, 0x0125, 0x051B, 0x0F0D, 0xFFFF },
		{ 0x0028, 0x031C, 0x0221, 0x0029, 0xFFFF },
		{ 0x0120, 0x0313, 0x0C0B, 0x081F, 0xFFFF },
		{ 0x0605, 0x0417, 0xFFFF },
		{ 0x0216, 0x041E, 0x0C0E, 0x0223, 0x0127, 0xFFFF },
		{ 0x0122, 0x0304, 0x060A, 0x0311, 0x0A09, 0xFFFF },
		{ 0x0519, 0x190F, 0xFFFF },
		{ 0x002A, 0x0231, 0x0503, 0x0415, 0x0807, 0x002B, 0x071D, 0xFFFF },
		{ 0x0130, 0x0214, 0x0508, 0x0401, 0x0133, 0x0225, 0x061B, 0xFFFF },
		{ 0x0200, 0x0124, 0x0406, 0x0321, 0x0129, 0x100D, 0xFFFF },
		{ 0x031A, 0x0D0B, 0x091F, 0xFFFF },
		{ 0x0413, 0x0705, 0x0517, 0xFFFF },
		{ 0x0212, 0x0034, 0x0323, 0x0035, 0x0227, 0xFFFF },
		{ 0x0126, 0x080C, 0x0B09, 0xFFFF },
		{ 0x0411, 0x0619, 0x1A0F, 0xFFFF },
		{ 0x0210, 0x0331, 0x0603, 0x0515, 0x0907, 0x012B, 0xFFFF },
		{ 0x0318, 0x002C, 0x0501, 0x0233, 0x0325, 0x071B, 0x002D, 0x081D, 0xFFFF },
		{ 0x0132, 0x0302, 0x0229, 0x110D, 0xFFFF },
		{ 0x0128, 0x041C, 0x0421, 0x0E0B, 0x0A1F, 0xFFFF },
		{ 0x0220, 0x0513, 0x0617, 0xFFFF },
		{ 0x0135, 0x0805, 0x0327, 0xFFFF },
		{ 0x0316, 0x051E, 0x0D0E, 0x0423, 0xFFFF },
		{ 0x0222, 0x0404, 0x070A, 0x0511, 0x0719, 0x0C09, 0x1B0F, 0xFFFF },
		{ 0x0703, 0x0615, 0x0A07, 0x022B, 0xFFFF },
		{ 0x012A, 0x0431, 0x0601, 0x0333, 0x012D, 0x091D, 0xFFFF },
		{ 0x0230, 0x0314, 0x0036, 0x0608, 0x0425, 0x0037, 0x0329, 0x081B, 0x120D, 0xFFFF },
		{ 0x0300, 0x0224, 0x0506, 0x0521, 0x0F0B, 0x0B1F, 0xFFFF },
		{ 0x041A, 0x0613, 0x0717, 0xFFFF },
		{ 0x0235, 0x0905, 0xFFFF },
		{ 0x0312, 0x0134, 0x0523, 0x0427, 0xFFFF },
		{ 0x0226, 0x090C, 0x002E, 0x0611, 0x0D09, 0x002F, 0xFFFF },
		{ 0x0715, 0x0B07, 0x0819, 0x032B, 0x1C0F, 0xFFFF },
		{ 0x0310, 0x0531, 0x0701, 0x0803, 0x022D, 0x0A1D, 0xFFFF },
		{ 0x0418, 0x012C, 0x0433, 0x0525, 0x0137, 0x091B, 0x130D, 0xFFFF },
		{ 0x0232, 0x0402, 0x0621, 0x0429, 0xFFFF },
		{ 0x0228, 0x051C, 0x0713, 0x100B, 0x0C1F, 0xFFFF },
		{ 0x0320, 0x0335, 0x0A05, 0x0817, 0xFFFF },
		{ 0x0623, 0x0527, 0xFFFF },
		{ 0x0416, 0x061E, 0x0E0E, 0x0711, 0x0E09, 0x012F, 0xFFFF },
		{ 0x0322, 0x0504, 0x080A, 0x0919, 0x1D0F, 0xFFFF },
		{ 0x0631, 0x0903, 0x0815, 0x0C07, 0x042B, 0x032D, 0x0B1D, 0xFFFF },
		{ 0x022A, 0x0801, 0x0533, 0x0625, 0x0237, 0x0A1B, 0xFFFF },
		{ 0x0330, 0x0414, 0x0136, 0x0708, 0x0721, 0x0529, 0x140D, 0xFFFF },
		{ 0x0400, 0x0324, 0x0606, 0x0038, 0x0039, 0x110B, 0x0D1F, 0xFFFF },
		{ 0x051A, 0x0813, 0x0B05, 0x0917, 0xFFFF },
		{ 0x0723, 0x0435, 0x0627, 0xFFFF },
		{ 0x0412, 0x0234, 0x0F09, 0x022F, 0xFFFF },
		{ 0x0326, 0x0A0C, 0x012E, 0x0811, 0x0A19, 0x1E0F, 0xFFFF },
		{ 0x0731, 0x0A03, 0x0915, 0x0D07, 0x052B, 0xFFFF },
		{ 0x0410, 0x0901, 0x0633, 0x0725, 0x0337, 0x0B1B, 0x042D, 0x0C1D, 0xFFFF },
		{ 0x0518, 0x022C, 0x0629, 0x150D, 0xFFFF },
		{ 0x0332, 0x0502, 0x0821, 0x0139, 0x120B, 0x0E1F, 0xFFFF },
		{ 0x0328, 0x061C, 0x0913, 0x0A17, 0xFFFF },
		{ 0x0420, 0x0535, 0x0C05, 0x0727, 0xFFFF },
		{ 0x0823, 0x032F, 0xFFFF },
		{ 0x0516, 0x071E, 0x0F0E, 0x0911, 0x0B19, 0x1009, 0x1F0F, 0xFFFF },
		{ 0x0422, 0x0604, 0x090A, 0x0B03, 0x0A15, 0x0E07, 0x062B, 0xFFFF },
		{ 0x0831, 0x0A01, 0x0733, 0x052D, 0x0D1D, 0xFFFF },
		{ 0x032A, 0x0825, 0x0437, 0x0729, 0x0C1B, 0x160D, 0xFFFF },
		{ 0x0430, 0x0514, 0x0236, 0x0808, 0x0921, 0x0239, 0x130B, 0x0F1F, 0xFFFF },
		{ 0x0500, 0x0424, 0x0706, 0x0138, 0x0A13, 0x0B17, 0xFFFF },
		{ 0x061A, 0x0635, 0x0D05, 0xFFFF },
		{ 0x0923, 0x0827, 0xFFFF },
		{ 0x0512, 0x0334, 0x003A, 0x0A11, 0x1109, 0x003B, 0x042F, 0xFFFF },
		{ 0x0426, 0x0B0C, 0x022E, 0x0B15, 0x0F07, 0x0C19, 0x072B, 0xFFFF },
		{ 0x0931, 0x0B01, 0x0C03, 0x062D, 0x0E1D, 0xFFFF },
		{ 0x0510, 0x0833, 0x0925, 0x0537, 0x0D1B, 0x170D, 0xFFFF },
		{ 0x0618, 0x032C, 0x0A21, 0x0339, 0x0829, 0xFFFF },
		{ 0x0432, 0x0602, 0x0B13, 0x140B, 0x101F, 0xFFFF },
		{ 0x0428, 0x071C, 0x0735, 0x0E05, 0x0C17, 0xFFFF },
		{ 0x0520, 0x0A23, 0x0927, 0xFFFF },
		{ 0x0B11, 0x1209, 0x013B, 0x052F, 0xFFFF },
		{ 0x0616, 0x081E, 0x0D19, 0xFFFF },
		{ 0x0522, 0x0704, 0x0A0A, 0x0A31, 0x0D03, 0x0C15, 0x1007, 0x082B, 0x072D, 0x0F1D, 0xFFFF },
		{ 0x0C01, 0x0933, 0x0A25, 0x0637, 0x0E1B, 0xFFFF },
		{ 0x042A, 0x0B21, 0x0929, 0x180D, 0xFFFF },
		{ 0x0530, 0x0614, 0x0336, 0x0908, 0x0439, 0x150B, 0x111F, 0xFFFF },
		{ 0x0600, 0x0524, 0x0806, 0x0238, 0x0C13, 0x0F05, 0x0D17, 0xFFFF },
		{ 0x071A, 0x0B23, 0x0835, 0x0A27, 0xFFFF },
		{ 0x1309, 0x023B, 0x062F, 0xFFFF },
		{ 0x0612, 0x0434, 0x013A, 0x0C11, 0x0E19, 0xFFFF },
		{ 0x0526, 0x0C0C, 0x032E, 0x0B31, 0x0E03, 0x0D15, 0x1107, 0x092B, 0xFFFF },
		{ 0x0D01, 0x0A33, 0x0B25, 0x0737, 0x0F1B, 0x082D, 0x101D, 0xFFFF },
		{ 0x0610, 0x0A29, 0x190D, 0xFFFF },
		{ 0x0718, 0x042C, 0x0C21, 0x0539, 0x160B, 0x121F, 0xFFFF },
		{ 0x0532, 0x0702, 0x0D13, 0x0E17, 0xFFFF },
		{ 0x0528, 0x081C, 0x0935, 0x1005, 0x0B27, 0xFFFF },
		{ 0x0620, 0x0C23, 0x033B, 0x072F, 0xFFFF },
		{ 0x0D11, 0x0F19, 0x1409, 0xFFFF },
		{ 0x0716, 0x003C, 0x091E, 0x0F03, 0x0E15, 0x1207, 0x0A2B, 0x003D, 0xFFFF },
		{ 0x0622, 0x0804, 0x0B0A, 0x0C31, 0x0E01, 0x0B33, 0x092D, 0x111D, 0xFFFF },
		{ 0x0C25, 0x0837, 0x0B29, 0x101B, 0x1A0D, 0xFFFF },
		{ 0x052A, 0x0D21, 0x0639, 0x170B, 0x131F, 0xFFFF },
		{ 0x0630, 0x0714, 0x0436, 0x0A08, 0x0E13, 0x0F17, 0xFFFF },
		{ 0x0700, 0x0624, 0x0906, 0x0338, 0x0A35, 0x1105, 0xFFFF },
		{ 0x081A, 0x0D23, 0x0C27, 0xFFFF },
		{ 0x0E11, 0x1509, 0x043B, 0x082F, 0xFFFF },
		{ 0x0712, 0x0534, 0x023A, 0x0F15, 0x1307, 0x1019, 0x0B2B, 0x013D, 0xFFFF },
		{ 0x0626, 0x0D0C, 0x042E, 0x0D31, 0x0F01, 0x1003, 0x0A2D, 0x121D, 0xFFFF },
		{ 0x0C33, 0x0D25, 0x0937, 0x111B, 0x1B0D, 0xFFFF },
		{ 0x0710, 0x0E21, 0x0739, 0x0C29, 0xFFFF },
		{ 0x0818, 0x052C, 0x0F13, 0x180B, 0x141F, 0xFFFF },
		{ 0x0632, 0x0802, 0x0B35, 0x1205, 0x1017, 0xFFFF },
		{ 0x0628, 0x091C, 0x0E23, 0x0D27, 0xFFFF },
		{ 0x0720, 0x0F11, 0x1609, 0x053B, 0x092F, 0xFFFF },
		{ 0x1119, 0x023D, 0xFFFF },
		{ 0x0816, 0x013C, 0x0A1E, 0x0E31, 0x1103, 0x1015, 0x1407, 0x0C2B, 0x0B2D, 0x131D, 0xFFFF },
		{ 0x0722, 0x0904, 0x0C0A, 0x1001, 0x0D33, 0x0E25, 0x0A37, 0x121B, 0xFFFF },
		{ 0x0F21, 0x0D29, 0x1C0D, 0xFFFF },
		{ 0x062A, 0x0839, 0x190B, 0x151F, 0xFFFF },
		{ 0x0730, 0x0814, 0x0536, 0x0B08, 0x1013, 0x1305, 0x1117, 0xFFFF },
		{ 0x0800, 0x0724, 0x0A06, 0x0438, 0x0F23, 0x0C35, 0x0E27, 0xFFFF },
		{ 0x091A, 0x1709, 0x063B, 0x0A2F, 0xFFFF },
		{ 0x1011, 0x1219, 0x033D, 0xFFFF },
		{ 0x0812, 0x0634, 0x033A, 0x0F31, 0x1203, 0x1115, 0x1507, 0x0D2B, 0xFFFF },
		{ 0x0726, 0x0E0C, 0x052E, 0x1101, 0x0E33, 0x0F25, 0x0B37, 0x131B, 0x0C2D, 0x141D, 0xFFFF },
		{ 0x0E29, 0x1D0D, 0xFFFF },
		{ 0x0810, 0x1021, 0x0939, 0x1A0B, 0x161F, 0xFFFF },
		{ 0x0918, 0x062C, 0x1113, 0x1217, 0xFFFF },
		{ 0x0732, 0x0902, 0x0D35, 0x1405, 0x0F27, 0xFFFF },
		{ 0x0728, 0x0A1C, 0x1023, 0x073B, 0x0B2F, 0xFFFF },
		{ 0x0820, 0x1111, 0x1319, 0x1809, 0xFFFF },
		{ 0x1303, 0x1215, 0x1607, 0x0E2B, 0x043D, 0xFFFF },
		{ 0x0916, 0x023C, 0x0B1E, 0x1031, 0x1201, 0x0F33, 0x0D2D, 0x151D, 0xFFFF },
		{ 0x0822, 0x0A04, 0x0D0A, 0x1025, 0x0C37, 0x0F29, 0x141B, 0x1E0D, 0xFFFF },
		{ 0x1121, 0x0A39, 0x1B0B, 0x171F, 0xFFFF },
		{ 0x072A, 0x1213, 0x1317, 0xFFFF },
		{ 0x0830, 0x0914, 0x0636, 0x0C08, 0x0E35, 0x1505, 0xFFFF },
		{ 0x0900, 0x0824, 0x0B06, 0x0538, 0x1123, 0x1027, 0xFFFF },
		{ 0x0A1A, 0x1211, 0x1909, 0x083B, 0x0C2F, 0xFFFF },
		{ 0x1315, 0x1707, 0x1419, 0x0F2B, 0x053D, 0xFFFF },
		{ 0x0912, 0x0734, 0x043A, 0x1131, 0x1301, 0x1403, 0x0E2D, 0x161D, 0xFFFF },
		{ 0x0826, 0x0F0C, 0x062E, 0x1033, 0x1125, 0x0D37, 0x151B, 0x1F0D, 0xFFFF },
		{ 0x1221, 0x0B39, 0x1029, 0xFFFF },
		{ 0x0910, 0x1313, 0x1C0B, 0x181F, 0xFFFF },
		{ 0x0A18, 0x072C, 0x0F35, 0x1605, 0x1417, 0xFFFF },
		{ 0x0832, 0x0A02, 0x1223, 0x1127, 0xFFFF },
		{ 0x0828, 0x0B1C, 0x1311, 0x1A09, 0x093B, 0x0D2F, 0xFFFF },
		{ 0x0920, 0x1519, 0x063D, 0xFFFF },
		{ 0x1231, 0x1503, 0x1415, 0x1807, 0x102B, 0x0F2D, 0x171D, 0xFFFF },
		{ 0x0A16, 0x033C, 0x0C1E, 0x1401, 0x1133, 0x1225, 0x0E37, 0x161B, 0xFFFF },
		{ 0x0922, 0x0B04, 0x0E0A, 0x1321, 0x1129, 0xFFFF },
		{ 0x0C39, 0x1D0B, 0x191F, 0xFFFF },
		{ 0x082A, 0x1413, 0x1705, 0x1517, 0xFFFF },
		{ 0x0930, 0x0A14, 0x0736, 0x0D08, 0x1323, 0x1035, 0x1227, 0xFFFF },
		{ 0x0A00, 0x0924, 0x0C06, 0x0638, 0x1B09, 0x0A3B, 0x0E2F, 0xFFFF },
		{ 0x0B1A, 0x1411, 0x1619, 0x073D, 0xFFFF },
		{ 0x1331, 0x1603, 0x1515, 0x1907, 0x112B, 0xFFFF },
		{ 0x0A12, 0x0834, 0x053A, 0x1501, 0x1233, 0x1325, 0x0F37, 0x171B, 0x102D, 0x181D, 0xFFFF },
		{ 0x0926, 0x072E, 0x1229, 0xFFFF },
		{ 0x1421, 0x0D39, 0x1E0B, 0x1A1F, 0xFFFF },
		{ 0x0A10, 0x1513, 0x1617, 0xFFFF },
		{ 0x0B18, 0x082C, 0x1135, 0x1805, 0x1327, 0xFFFF },
		{ 0x0932, 0x0B02, 0x1423, 0x0B3B, 0x0F2F, 0xFFFF },
		{ 0x0928, 0x0C1C, 0x1511, 0x1719, 0x1C09, 0xFFFF },
		{ 0x0A20, 0x1703, 0x1615, 0x1A07, 0x122B, 0x083D, 0xFFFF },
		{ 0x1431, 0x1601, 0x1333, 0x112D, 0x191D, 0xFFFF },
		{ 0x0B16, 0x043C, 0x0D1E, 0x1425, 0x1037, 0x1329, 0x181B, 0xFFFF },
		{ 0x0A22, 0x0C04, 0x0F0A, 0x1521, 0x0E39, 0x1F0B, 0x1B1F, 0xFFFF },
		{ 0x1613, 0x1717, 0xFFFF },
		{ 0x092A, 0x1235, 0x1905, 0xFFFF },
		{ 0x0A30, 0x0B14, 0x0836, 0x0E08, 0x1523, 0x1427, 0xFFFF },
		{ 0x0B00, 0x0A24, 0x0D06, 0x0738, 0x1611, 0x1D09, 0x0C3B, 0x102F, 0xFFFF },
		{ 0x0C1A, 0x1715, 0x1B07, 0x1819, 0x132B, 0x093D, 0xFFFF },
		{ 0x1531, 0x1701, 0x1803, 0x122D, 0x1A1D, 0xFFFF },
		{ 0x0B12, 0x0934, 0x063A, 0x1433, 0x1525, 0x1137, 0x191B, 0xFFFF },
		{ 0x0A26, 0x003E, 0x082E, 0x1621, 0x0F39, 0x1429, 0x003F, 0xFFFF },
		{ 0x1713, 0x1C1F, 0xFFFF },
		{ 0x0B10, 0x1335, 0x1A05, 0x1817, 0xFFFF },
		{ 0x0C18, 0x092C, 0x1623, 0x1527, 0xFFFF },
		{ 0x0A32, 0x0C02, 0x1711, 0x1E09, 0x0D3B, 0x112F, 0xFFFF },
		{ 0x0A28, 0x0D1C, 0x1919, 0x0A3D, 0xFFFF },
		{ 0x0B20, 0x1631, 0x1903, 0x1815, 0x1C07, 0x142B, 0x132D, 0x1B1D, 0xFFFF },
		{ 0x1801, 0x1533, 0x1625, 0x1237, 0x1A1B, 0xFFFF },
		{ 0x0C16, 0x053C, 0x0E1E, 0x1721, 0x1529, 0x013F, 0xFFFF },
		{ 0x0B22, 0x0D04, 0x1039, 0x1D1F, 0xFFFF },
		{ 0x1813, 0x1B05, 0x1917, 0xFFFF },
		{ 0x0A2A, 0x1723, 0x1435, 0x1627, 0xFFFF },
		{ 0x0B30, 0x0C14, 0x0936, 0x0F08, 0x1F09, 0x0E3B, 0x122F, 0xFFFF },
		{ 0x0C00, 0x0B24, 0x0E06, 0x0838, 0x1811, 0x1A19, 0x0B3D, 0xFFFF },
		{ 0x0D1A, 0x1731, 0x1A03, 0x1915, 0x1D07, 0x152B, 0xFFFF },
		{ 0x1901, 0x1633, 0x1725, 0x1337, 0x1B1B, 0x142D, 0x1C1D, 0xFFFF },
		{ 0x0C12, 0x0A34, 0x073A, 0x1629, 0x023F, 0xFFFF },
		{ 0x0B26, 0x013E, 0x092E, 0x1821, 0x1139, 0x1E1F, 0xFFFF },
		{ 0x1913, 0x1A17, 0xFFFF },
		{ 0x0C10, 0x1535, 0x1C05, 0x1727, 0xFFFF },
		{ 0x0D18, 0x0A2C, 0x1823, 0x0F3B, 0x132F, 0xFFFF },
		{ 0x0B32, 0x0D02, 0x1911, 0x1B19, 0xFFFF },
		{ 0x0B28, 0x0E1C, 0x1B03, 0x1A15, 0x1E07, 0x162B, 0x0C3D, 0xFFFF },
		{ 0x0C20, 0x1831, 0x1A01, 0x1733, 0x152D, 0x1D1D, 0xFFFF },
		{ 0x1825, 0x1437, 0x1729, 0x1C1B, 0x033F, 0xFFFF },
		{ 0x0D16, 0x063C, 0x0F1E, 0x1921, 0x1239, 0x1F1F, 0xFFFF },
		{ 0x0C22, 0x0E04, 0x1A13, 0x1B17, 0xFFFF },
		{ 0x1635, 0x1D05, 0xFFFF },
		{ 0x0B2A, 0x1923, 0x1827, 0xFFFF },
		{ 0x0C30, 0x0D14, 0x0A36, 0x1A11, 0x103B, 0x142F, 0xFFFF },
		{ 0x0D00, 0x0C24, 0x0F06, 0x0938, 0x1B15, 0x1F07, 0x1C19, 0x172B, 0x0D3D, 0xFFFF },
		{ 0x0E1A, 0x1931, 0x1B01, 0x1C03, 0x162D, 0x1E1D, 0xFFFF }, 
		{ 0x1833, 0x1925, 0x1537, 0x1D1B, 0xFFFF },
		{ 0x0D12, 0x0B34, 0x083A, 0x1A21, 0x1339, 0x1829, 0x043F, 0xFFFF },
		{ 0x0C26, 0x023E, 0x0A2E, 0x1B13, 0xFFFF },
		{ 0x1735, 0x1E05, 0x1C17, 0xFFFF },
		{ 0x0D10, 0x1A23, 0x1927, 0xFFFF },
		{ 0x0E18, 0x0B2C, 0x1B11, 0x113B, 0x152F, 0xFFFF },
		{ 0x0C32, 0x0E02, 0x1D19, 0x0E3D, 0xFFFF },
		{ 0x0C28, 0x0F1C, 0x1A31, 0x1D03, 0x1C15, 0x182B, 0x172D, 0x1F1D, 0xFFFF },
		{ 0x0D20, 0x1C01, 0x1933, 0x1A25, 0x1637, 0x1E1B, 0xFFFF },
		{ 0x1B21, 0x1929, 0x053F, 0xFFFF },
		{ 0x0E16, 0x073C, 0x1439, 0xFFFF },
		{ 0x0D22, 0x0F04, 0x1C13, 0x1F05, 0x1D17, 0xFFFF },
		{ 0x1B23, 0x1835, 0x1A27, 0xFFFF },
		{ 0x0C2A, 0x123B, 0x162F, 0xFFFF },
		{ 0x0D30, 0x0E14, 0x0B36, 0x1C11, 0x1E19, 0x0F3D, 0xFFFF },
		{ 0x0E00, 0x0D24, 0x0A38, 0x1B31, 0x1E03, 0x1D15, 0x192B, 0xFFFF },
		{ 0x0F1A, 0x1D01, 0x1A33, 0x1B25, 0x1737, 0x1F1B, 0x182D, 0xFFFF },
		{ 0x1A29, 0x063F, 0xFFFF },
		{ 0x0E12, 0x0C34, 0x093A, 0x1C21, 0x1539, 0xFFFF },
		{ 0x0D26, 0x033E, 0x0B2E, 0x1D13, 0x1E17, 0xFFFF },
		{ 0x1935, 0x1B27, 0xFFFF },
		{ 0x0E10, 0x1C23, 0x133B, 0x172F, 0xFFFF },
		{ 0x0F18, 0x0C2C, 0x1D11, 0x1F19, 0xFFFF },
		{ 0x0D32, 0x0F02, 0x1F03, 0x1E15, 0x1A2B, 0x103D, 0xFFFF },
		{ 0x0D28, 0x1C31, 0x1E01, 0x1B33, 0x192D, 0xFFFF },
		{ 0x0E20, 0x1C25, 0x1837, 0x1B29, 0x073F, 0xFFFF },
		{ 0x1D21, 0x1639, 0xFFFF },
		{ 0x0F16, 0x083C, 0x1E13, 0x1F17, 0xFFFF },
		{ 0x0E22, 0x1A35, 0xFFFF },
		{ 0x1D23, 0x1C27, 0xFFFF },
		{ 0x0D2A, 0x1E11, 0x143B, 0x182F, 0xFFFF },
		{ 0x0E30, 0x0F14, 0x0C36, 0x1F15, 0x1B2B, 0x113D, 0xFFFF },
		{ 0x0F00, 0x0E24, 0x0B38, 0x1D31, 0x1F01, 0x1A2D, 0xFFFF },
		{ 0x1C33, 0x1D25, 0x1937, 0xFFFF },
		{ 0x1E21, 0x1739, 0x1C29, 0x083F, 0xFFFF },
		{ 0x0F12, 0x0D34, 0x0A3A, 0x1F13, 0xFFFF },
		{ 0x0E26, 0x043E, 0x0C2E, 0x1B35, 0xFFFF },
		{ 0x1E23, 0x1D27, 0xFFFF },
		{ 0x0F10, 0x1F11, 0x153B, 0x192F, 0xFFFF },
		{ 0x0D2C, 0x123D, 0xFFFF }
	};

	// Returns pointer to sorted array.
	template<typename T, typename Q>
	T* IndirectRadixSort(uint32_t num_indices, T* indices0, T* indices1, Q const * keys, uint32_t key_ofs, uint32_t key_size, bool init_indices)
	{
		BOOST_ASSERT((key_ofs >= 0) && (key_ofs < sizeof(T)));
		BOOST_ASSERT((key_size >= 1) && (key_size <= 4));

		if (init_indices)
		{
			T* p = indices0;
			T* q = indices0 + (num_indices & ~T(1));
			uint32_t i;
			for (i = 0; p != q; p += 2, i += 2)
			{
				p[0] = static_cast<T>(i + 0);
				p[1] = static_cast<T>(i + 1);
			}

			if (num_indices & 1)
			{
				*p = static_cast<T>(i);
			}
		}

		uint32_t hist[256 * 4];

		memset(hist, 0, sizeof(hist));

#define IRS_GET_KEY(p) (*reinterpret_cast<uint32_t const *>(reinterpret_cast<uint8_t const *>(keys + *(p)) + key_ofs))
#define IRS_GET_KEY_FROM_INDEX(i) (*reinterpret_cast<uint32_t const *>(reinterpret_cast<uint8_t const *>(keys + (i)) + key_ofs))

		if (4 == key_size)
		{
			T* p = indices0;
			T* q = indices0 + num_indices;
			for (; p != q; ++ p)
			{
				uint32_t const key = IRS_GET_KEY(p);

				++ hist[key & 0xFF];
				++ hist[256 + ((key >> 8) & 0xFF)];
				++ hist[512 + ((key >> 16) & 0xFF)];
				++ hist[768 + ((key >> 24) & 0xFF)];
			}
		}
		else if (3 == key_size)
		{
			T* p = indices0;
			T* q = indices0 + num_indices;
			for (; p != q; p++)
			{
				uint32_t const key = IRS_GET_KEY(p);

				++ hist[key & 0xFF];
				++ hist[256 + ((key >> 8) & 0xFF)];
				++ hist[512 + ((key >> 16) & 0xFF)];
			}
		}
		else if (2 == key_size)
		{
			T* p = indices0;
			T* q = indices0 + (num_indices & ~T(1));

			for (; p != q; p += 2)
			{
				uint32_t const key0 = IRS_GET_KEY(p + 0);
				uint32_t const key1 = IRS_GET_KEY(p + 1);

				++ hist[key0 & 0xFF];
				++ hist[256 + ((key0 >> 8) & 0xFF)];

				++ hist[key1 & 0xFF];
				++ hist[256 + ((key1 >> 8) & 0xFF)];
			}

			if (num_indices & 1)
			{
				uint32_t const key = IRS_GET_KEY(p);

				++ hist[key & 0xFF];
				++ hist[256 + ((key >> 8) & 0xFF)];
			}
		}
		else
		{
			BOOST_ASSERT(1 == key_size);
			if (key_size != 1)
			{
				return nullptr;
			}

			T* p = indices0;
			T* q = indices0 + (num_indices & ~T(1));

			for (; p != q; p += 2)
			{
				uint32_t const key0 = IRS_GET_KEY(p + 0);
				uint32_t const key1 = IRS_GET_KEY(p + 1);

				++ hist[key0 & 0xFF];
				++ hist[key1 & 0xFF];
			}

			if (num_indices & 1)
			{
				uint32_t const key = IRS_GET_KEY(p);

				++ hist[key & 0xFF];
			}
		}

		T* cur_ind = indices0;
		T* new_ind = indices1;

		for (uint32_t pass = 0; pass < key_size; ++ pass)
		{
			uint32_t const * hist_ptr = &hist[pass << 8];

			uint32_t offsets[256];

			uint32_t cur_ofs = 0;
			for (uint32_t i = 0; i < 256; i += 2)
			{
				offsets[i + 0] = cur_ofs;
				cur_ofs += hist_ptr[i + 0];

				offsets[i + 1] = cur_ofs;
				cur_ofs += hist_ptr[i + 1];
			}

			uint32_t const pass_shift = pass << 3;

			T* p = cur_ind;
			T* q = cur_ind + (num_indices >> 1) * 2;

			for (; p != q; p += 2)
			{
				uint32_t index0 = p[0];
				uint32_t index1 = p[1];

				uint32_t c0 = (IRS_GET_KEY_FROM_INDEX(index0) >> pass_shift) & 0xFF;
				uint32_t c1 = (IRS_GET_KEY_FROM_INDEX(index1) >> pass_shift) & 0xFF;

				if (c0 == c1)
				{
					uint32_t dst_offset0 = offsets[c0];

					offsets[c0] = dst_offset0 + 2;

					new_ind[dst_offset0 + 0] = static_cast<T>(index0);
					new_ind[dst_offset0 + 1] = static_cast<T>(index1);
				}
				else
				{
					uint32_t dst_offset0 = offsets[c0];
					uint32_t dst_offset1 = offsets[c1];
					++ offsets[c0];
					++ offsets[c1];

					new_ind[dst_offset0] = static_cast<T>(index0);
					new_ind[dst_offset1] = static_cast<T>(index1);
				}
			}

			if (num_indices & 1)
			{
				uint32_t index = *p;
				uint32_t c = (IRS_GET_KEY_FROM_INDEX(index) >> pass_shift) & 0xFF;

				uint32_t dst_offset = offsets[c];
				offsets[c] = dst_offset + 1;

				new_ind[dst_offset] = static_cast<T>(index);
			}

			std::swap(cur_ind, new_ind);
		}

#undef IRS_GET_KEY
#undef IRS_GET_KEY_FROM_INDEX

		return cur_ind;
	}
}

namespace KlayGE
{
	using namespace TexCompressionLUT;

	TexCompressionETC1::Params::Params()
		: quality_(TCM_Quality),
			num_src_pixels_(0), src_pixels_(0),
			use_color4_(false),
			scan_delta_size_(1),
			base_color5_(0),
			constrain_against_base_color5_(false)
	{
		static int const s_default_scan_delta[] = { 0 };
		scan_deltas_ = s_default_scan_delta;
	}


	TexCompressionETC1::Results::Results()
		: error_(std::numeric_limits<uint64_t>::max()),
			block_inten_table_(0),
			selectors_(8)
	{
	}

	TexCompressionETC1::Results& TexCompressionETC1::Results::operator=(TexCompressionETC1::Results const & rhs)
	{
		if (this != &rhs)
		{
			block_color_unscaled_ = rhs.block_color_unscaled_;
			block_color4_ = rhs.block_color4_;
			block_inten_table_ = rhs.block_inten_table_;
			error_ = rhs.error_;
			BOOST_ASSERT(selectors_.size() == rhs.selectors_.size());
			selectors_ = rhs.selectors_;
		}
		return *this;
	}


	TexCompressionETC1::ETC1SolutionCoordinates::ETC1SolutionCoordinates()
		: unscaled_color_(0),
			inten_table_(0),
			color4_(false)
	{
	}

	TexCompressionETC1::ETC1SolutionCoordinates::ETC1SolutionCoordinates(int r, int g, int b, uint32_t inten_table, bool color4)
		: unscaled_color_(From4Ints(255, r, g, b)),
			inten_table_(inten_table),
			color4_(color4)
	{
	}

	TexCompressionETC1::ETC1SolutionCoordinates::ETC1SolutionCoordinates(ARGBColor32 const & c, uint32_t inten_table, bool color4)
		: unscaled_color_(c),
			inten_table_(inten_table),
			color4_(color4)
	{
	}

	TexCompressionETC1::ETC1SolutionCoordinates::ETC1SolutionCoordinates(ETC1SolutionCoordinates const & rhs)
	{
		*this = rhs;
	}

	TexCompressionETC1::ETC1SolutionCoordinates& TexCompressionETC1::ETC1SolutionCoordinates::operator=(ETC1SolutionCoordinates const & rhs)
	{
		if (this != &rhs)
		{
			unscaled_color_ = rhs.unscaled_color_;
			inten_table_ = rhs.inten_table_;
			color4_ = rhs.color4_;
		}
		return *this;
	}

	void TexCompressionETC1::ETC1SolutionCoordinates::Clear()
	{
		unscaled_color_ = ARGBColor32(0, 0, 0, 0);
		inten_table_ = 0;
		color4_ = false;
	}

	void TexCompressionETC1::ETC1SolutionCoordinates::ScaledColor(uint8_t& br, uint8_t& bg, uint8_t& bb) const
	{
		uint8_t const r = unscaled_color_.r();
		uint8_t const g = unscaled_color_.g();
		uint8_t const b = unscaled_color_.b();

		if (color4_)
		{
			br = Extend4To8Bits(r);
			bg = Extend4To8Bits(g);
			bb = Extend4To8Bits(b);
		}
		else
		{
			br = Extend5To8Bits(r);
			bg = Extend5To8Bits(g);
			bb = Extend5To8Bits(b);
		}
	}

	ARGBColor32 TexCompressionETC1::ETC1SolutionCoordinates::ScaledColor() const
	{
		uint8_t r, g, b;
		this->ScaledColor(r, g, b);
		return ARGBColor32(255, r, g, b);
	}

	void TexCompressionETC1::ETC1SolutionCoordinates::BlockColors(ARGBColor32* block_colors) const
	{
		uint8_t r, g, b;
		this->ScaledColor(r, g, b);

		int const modifiers[] = { GetModifier(inten_table_, 0), GetModifier(inten_table_, 1),
			GetModifier(inten_table_, 2), GetModifier(inten_table_, 3) };
		block_colors[0] = From4Ints(255, r + modifiers[0], g + modifiers[0], b + modifiers[0]);
		block_colors[1] = From4Ints(255, r + modifiers[1], g + modifiers[1], b + modifiers[1]);
		block_colors[2] = From4Ints(255, r + modifiers[2], g + modifiers[2], b + modifiers[2]);
		block_colors[3] = From4Ints(255, r + modifiers[3], g + modifiers[3], b + modifiers[3]);
	}


	TexCompressionETC1::PotentialSolution::PotentialSolution()
		: error_(std::numeric_limits<uint64_t>::max()), valid_(false)
	{
	}

	void TexCompressionETC1::PotentialSolution::Clear()
	{
		coords_.Clear();
		error_ = std::numeric_limits<uint64_t>::max();
		valid_ = false;
	}

	
	TexCompressionETC1::TexCompressionETC1()
	{
		block_width_ = block_height_ = 4;
		block_depth_ = 1;
		block_bytes_ = NumFormatBytes(EF_ETC1) * 4;
		decoded_fmt_ = EF_ARGB8;

		params_ = nullptr;
		result_ = nullptr;
		sorted_luma_ptr_ = nullptr;
		sorted_luma_indices_ = nullptr;
	}

	void TexCompressionETC1::EncodeBlock(void* output, void const * input, TexCompressionMethod method)
	{
		BOOST_ASSERT(output);
		BOOST_ASSERT(input);

		this->EncodeETC1BlockInternal(*static_cast<ETC1Block*>(output), static_cast<ARGBColor32 const *>(input), method);
	}

	uint64_t TexCompressionETC1::EncodeETC1BlockInternal(ETC1Block& dst_block, ARGBColor32 const * argb, TexCompressionMethod method)
	{
		BOOST_ASSERT(argb);

		// Check for solid block.
		bool uniform_block = true;
		for (int i = 1; i < 16; ++ i)
		{
			if (argb[i] != argb[0])
			{
				uniform_block = false;
				break;
			}
		}
		if (uniform_block)
		{
			return 16 * this->PackETC1UniformBlock(dst_block, argb);
		}

		uint64_t best_err = std::numeric_limits<uint64_t>::max();
		bool best_flip = false;
		bool best_use_color4 = false;

		Results best_results[2];
		Results results[3];
		ARGBColor32 subblock_pixels[8];

		Params params;
		params.quality_ = method;
		params.num_src_pixels_ = 8;
		params.src_pixels_ = subblock_pixels;

		for (uint32_t flip = 0; flip < 2; ++ flip)
		{
			for (uint32_t use_color4 = 0; use_color4 < 2; ++ use_color4)
			{
				uint64_t trial_err = 0;

				uint32_t subblock;
				for (subblock = 0; subblock < 2; ++ subblock)
				{
					if (flip)
					{
						memcpy(subblock_pixels, argb + subblock * 8, sizeof(uint32_t) * 8);
					}
					else
					{
						ARGBColor32 const * src_col = argb + subblock * 2;
						subblock_pixels[0] = src_col[0];
						subblock_pixels[1] = src_col[4];
						subblock_pixels[2] = src_col[8];
						subblock_pixels[3] = src_col[12];
						subblock_pixels[4] = src_col[1];
						subblock_pixels[5] = src_col[5];
						subblock_pixels[6] = src_col[9];
						subblock_pixels[7] = src_col[13];
					}

					results[2].error_ = std::numeric_limits<uint64_t>::max();
					if ((params.quality_ >= TCM_Balanced) && (subblock || use_color4))
					{
						bool uniform_partition = true;
						for (int r = 1; r < 8; ++ r)
						{
							if (subblock_pixels[r] != subblock_pixels[0])
							{
								uniform_partition = false;
								break;
							}
						}
						if (uniform_partition)
						{
							this->PackETC1UniformPartition(results[2], 8, subblock_pixels,
								!use_color4, (subblock && !use_color4) ? &results[0].block_color_unscaled_ : nullptr);
						}
					}

					params.use_color4_ = (use_color4 != 0);
					params.constrain_against_base_color5_ = false;

					if (!use_color4 && subblock)
					{
						params.constrain_against_base_color5_ = true;
						params.base_color5_ = results[0].block_color_unscaled_;
					}

					if (TCM_Quality == params.quality_)
					{
						static int const scan_delta_0_to_4[] = { -4, -3, -2, -1, 0, 1, 2, 3, 4 };
						params.scan_delta_size_ = static_cast<uint32_t>(std::size(scan_delta_0_to_4));
						params.scan_deltas_ = scan_delta_0_to_4;
					}
					else if (TCM_Balanced == params.quality_)
					{
						static int const scan_delta_0_to_1[] = { -1, 0, 1 };
						params.scan_delta_size_ = static_cast<uint32_t>(std::size(scan_delta_0_to_1));
						params.scan_deltas_ = scan_delta_0_to_1;
					}
					else
					{
						static int const scan_delta_0[] = { 0 };
						params.scan_delta_size_ = static_cast<uint32_t>(std::size(scan_delta_0));
						params.scan_deltas_ = scan_delta_0;
					}

					this->InitSolver(params, results[subblock]);
					if (!this->Solve())
					{
						break;
					}

					if (params.quality_ >= TCM_Balanced)
					{
						// TODO: Fix fairly arbitrary/unrefined thresholds that control how far away to scan for potentially better solutions.
						uint32_t const refinement_error_thresh0 = 3000;
						uint32_t const refinement_error_thresh1 = 6000;
						if (results[subblock].error_ > refinement_error_thresh0)
						{
							if (TCM_Balanced == params.quality_)
							{
								static int const scan_delta_2_to_3[] = { -3, -2, 2, 3 };
								params.scan_delta_size_ = static_cast<uint32_t>(std::size(scan_delta_2_to_3));
								params.scan_deltas_ = scan_delta_2_to_3;
							}
							else
							{
								static int const scan_delta_5_to_5[] = { -5, 5 };
								static int const scan_delta_5_to_8[] = { -8, -7, -6, -5, 5, 6, 7, 8 };
								if (results[subblock].error_ > refinement_error_thresh1)
								{
									params.scan_delta_size_ = static_cast<uint32_t>(std::size(scan_delta_5_to_8));
									params.scan_deltas_ = scan_delta_5_to_8;
								}
								else
								{
									params.scan_delta_size_ = static_cast<uint32_t>(std::size(scan_delta_5_to_5));
									params.scan_deltas_ = scan_delta_5_to_5;
								}
							}

							if (!this->Solve())
							{
								break;
							}
						}

						if (results[2].error_ < results[subblock].error_)
						{
							results[subblock] = results[2];
						}
					}

					trial_err += results[subblock].error_;
					if (trial_err >= best_err)
					{
						break;
					}
				}

				if (subblock < 2)
				{
					continue;
				}

				best_err = trial_err;
				best_results[0] = results[0];
				best_results[1] = results[1];
				best_flip = flip ? true : false;
				best_use_color4 = use_color4 ? true : false;
			} // use_color4
		} // flip

		int dr = best_results[1].block_color_unscaled_.r() - best_results[0].block_color_unscaled_.r();
		int dg = best_results[1].block_color_unscaled_.g() - best_results[0].block_color_unscaled_.g();
		int db = best_results[1].block_color_unscaled_.b() - best_results[0].block_color_unscaled_.b();
		BOOST_ASSERT(best_use_color4 || ((MathLib::min3(dr, dg, db) >= -4) && (MathLib::min3(dr, dg, db) <= 3)));

		if (best_use_color4)
		{
			dst_block.r = static_cast<uint8_t>((best_results[0].block_color_unscaled_.r() << 4) | best_results[1].block_color_unscaled_.r());
			dst_block.g = static_cast<uint8_t>((best_results[0].block_color_unscaled_.g() << 4) | best_results[1].block_color_unscaled_.g());
			dst_block.b = static_cast<uint8_t>((best_results[0].block_color_unscaled_.b() << 4) | best_results[1].block_color_unscaled_.b());
		}
		else
		{
			if (dr < 0)
			{
				dr += 8;
			}
			dst_block.r = static_cast<uint8_t>((best_results[0].block_color_unscaled_.r() << 3) | dr);
			if (dg < 0)
			{
				dg += 8;
			}
			dst_block.g = static_cast<uint8_t>((best_results[0].block_color_unscaled_.g() << 3) | dg);
			if (db < 0)
			{
				db += 8;
			}
			dst_block.b = static_cast<uint8_t>((best_results[0].block_color_unscaled_.b() << 3) | db);
		}

		dst_block.cw_diff_flip = static_cast<uint8_t>((best_results[1].block_inten_table_ << 2)
			| (best_results[0].block_inten_table_ << 5) | (best_use_color4 ? 0 : 2) | (best_flip ? 1 : 0));

		uint32_t selector0 = 0;
		uint32_t selector1 = 0;
		if (best_flip)
		{
			// flipped:
			// { 0, 0 }, { 1, 0 }, { 2, 0 }, { 3, 0 },               
			// { 0, 1 }, { 1, 1 }, { 2, 1 }, { 3, 1 } 
			//
			// { 0, 2 }, { 1, 2 }, { 2, 2 }, { 3, 2 },
			// { 0, 3 }, { 1, 3 }, { 2, 3 }, { 3, 3 }
			uint8_t const * selectors0 = &best_results[0].selectors_[0];
			uint8_t const * selectors1 = &best_results[1].selectors_[0];
			for (int x = 3; x >= 0; -- x)
			{
				uint32_t b;

				b = selector_index_to_etc1[selectors1[4 + x]];
				selector0 = (selector0 << 1) | (b & 1);
				selector1 = (selector1 << 1) | (b >> 1);

				b = selector_index_to_etc1[selectors1[x]];
				selector0 = (selector0 << 1) | (b & 1);
				selector1 = (selector1 << 1) | (b >> 1);

				b = selector_index_to_etc1[selectors0[4 + x]];
				selector0 = (selector0 << 1) | (b & 1);
				selector1 = (selector1 << 1) | (b >> 1);

				b = selector_index_to_etc1[selectors0[x]];
				selector0 = (selector0 << 1) | (b & 1);
				selector1 = (selector1 << 1) | (b >> 1);
			}
		}
		else
		{
			// non-flipped:
			// { 0, 0 }, { 0, 1 }, { 0, 2 }, { 0, 3 },
			// { 1, 0 }, { 1, 1 }, { 1, 2 }, { 1, 3 }
			//
			// { 2, 0 }, { 2, 1 }, { 2, 2 }, { 2, 3 },
			// { 3, 0 }, { 3, 1 }, { 3, 2 }, { 3, 3 }
			for (int subblock = 1; subblock >= 0; -- subblock)
			{
				uint8_t const * selectors = &best_results[subblock].selectors_[4];
				for (uint32_t i = 0; i < 2; ++ i)
				{
					uint32_t b;

					b = selector_index_to_etc1[selectors[3]];
					selector0 = (selector0 << 1) | (b & 1);
					selector1 = (selector1 << 1) | (b >> 1);

					b = selector_index_to_etc1[selectors[2]];
					selector0 = (selector0 << 1) | (b & 1);
					selector1 = (selector1 << 1) | (b >> 1);

					b = selector_index_to_etc1[selectors[1]];
					selector0 = (selector0 << 1) | (b & 1);
					selector1 = (selector1 << 1) | (b >> 1);

					b = selector_index_to_etc1[selectors[0]];
					selector0 = (selector0 << 1) | (b & 1);
					selector1 = (selector1 << 1) | (b >> 1);

					selectors -= 4;
				}
			}
		}

		dst_block.msb = static_cast<uint16_t>((selector1 >> 8) | ((selector1 & 0xFF) << 8));
		dst_block.lsb = static_cast<uint16_t>((selector0 >> 8) | ((selector0 & 0xFF) << 8));

		return best_err;
	}

	void TexCompressionETC1::DecodeBlock(void* output, void const * input)
	{
		BOOST_ASSERT(output);
		BOOST_ASSERT(input);

		ARGBColor32* argb = static_cast<ARGBColor32*>(output);
		ETC1Block const & etc1 = *static_cast<ETC1Block const *>(input);

		if (etc1.cw_diff_flip & 0x2)
		{
			this->DecodeETCDifferentialModeInternal(argb, etc1, false);
		}
		else
		{
			this->DecodeETCIndividualModeInternal(argb, etc1);
		}
	}

	void TexCompressionETC1::DecodeETCIndividualModeInternal(ARGBColor32* argb, ETC1Block const & etc1) const
	{
		BOOST_ASSERT(argb);

		int r1 = etc1.r >> 4;
		int r2 = etc1.r & 0xF;
		int g1 = etc1.g >> 4;
		int g2 = etc1.g & 0xF;
		int b1 = etc1.b >> 4;
		int b2 = etc1.b & 0xF;

		uint8_t base_clr[2][3];
		base_clr[0][0] = Extend4To8Bits(r1);
		base_clr[0][1] = Extend4To8Bits(g1);
		base_clr[0][2] = Extend4To8Bits(b1);
		base_clr[1][0] = Extend4To8Bits(r2);
		base_clr[1][1] = Extend4To8Bits(g2);
		base_clr[1][2] = Extend4To8Bits(b2);

		ARGBColor32 modified_clr[2][4];
		for (int sub = 0; sub < 2; ++ sub)
		{
			int const cw = (etc1.cw_diff_flip >> (2 + (!sub * 3))) & 0x7;
			for (int mod = 0; mod < 4; ++ mod)
			{
				int const modifier = GetModifier(cw, mod);
				modified_clr[sub][selector_index_to_etc1[mod]] = From4Ints(255, base_clr[sub][0] + modifier,
					base_clr[sub][1] + modifier, base_clr[sub][2] + modifier);
			}
		}

		bool const flip = etc1.cw_diff_flip & 0x1;
		for (int x = 0; x < 4; ++ x)
		{
			for (int y = 0; y < 4; ++ y)
			{
				int sub_block = ((flip ? y : x) >> 1);
				int bit_index = (x * 4 + y) ^ 0x8;
				int msb = (etc1.msb >> bit_index) & 0x1;
				int lsb = (etc1.lsb >> bit_index) & 0x1;
				int pixel_index = msb * 2 + lsb;
				argb[y * 4 + x] = modified_clr[sub_block][pixel_index];
			}
		}
	}

	void TexCompressionETC1::DecodeETCDifferentialModeInternal(ARGBColor32* argb, ETC1Block const & etc1, bool alpha) const
	{
		BOOST_ASSERT(argb);

		int const r = etc1.r >> 3;
		int const dr = etc1.r & 0x7;
		int const g = etc1.g >> 3;
		int const dg = etc1.g & 0x7;
		int const b = etc1.b >> 3;
		int const db = etc1.b & 0x7;

		uint8_t base_clr[2][3];
		base_clr[0][0] = Extend5To8Bits(r);
		base_clr[0][1] = Extend5To8Bits(g);
		base_clr[0][2] = Extend5To8Bits(b);
		base_clr[1][0] = Extend5To8Bits(r - (dr & 0x4) + (dr & 0x3));
		base_clr[1][1] = Extend5To8Bits(g - (dg & 0x4) + (dg & 0x3));
		base_clr[1][2] = Extend5To8Bits(b - (db & 0x4) + (db & 0x3));

		ARGBColor32 modified_clr[2][4];
		for (int sub = 0; sub < 2; ++ sub)
		{
			int const cw = (etc1.cw_diff_flip >> (2 + (!sub * 3))) & 0x7;
			for (int mod = 0; mod < 4; ++ mod)
			{
				int modifier;
				if (alpha)
				{
					modifier = (mod & 0x1) ? GetModifier(cw, mod) : 0;
				}
				else
				{
					modifier = GetModifier(cw, mod);
				}
				modified_clr[sub][selector_index_to_etc1[mod]] = From4Ints(255, base_clr[sub][0] + modifier,
					base_clr[sub][1] + modifier, base_clr[sub][2] + modifier);
			}
		}

		bool const flip = etc1.cw_diff_flip & 0x1;
		for (int x = 0; x < 4; ++ x)
		{
			for (int y = 0; y < 4; ++ y)
			{
				int sub_block = ((flip ? y : x) >> 1);
				int bit_index = (x * 4 + y) ^ 0x8;
				int msb = (etc1.msb >> bit_index) & 0x1;
				int lsb = (etc1.lsb >> bit_index) & 0x1;
				int pixel_index = msb * 2 + lsb;
				if (alpha && (2 == pixel_index))
				{
					argb[y * 4 + x] = ARGBColor32(0, 0, 0, 0);
				}
				else
				{
					argb[y * 4 + x] = modified_clr[sub_block][pixel_index];
				}
			}
		}
	}

	int TexCompressionETC1::GetModifier(int cw, int selector)
	{
		static int const etc1_modifier_table[8][2] =
		{
			{ 2, 8 },
			{ 5, 17 },
			{ 9, 29 },
			{ 13, 42 },
			{ 18, 60 },
			{ 24, 80 },
			{ 33, 106 },
			{ 47, 183 }
		};

		return (selector >= 2) ? etc1_modifier_table[cw][selector - 2] : -etc1_modifier_table[cw][!selector];
	}

	uint32_t TexCompressionETC1::ETC1DecodeValue(uint32_t diff, uint32_t inten, uint32_t selector, uint32_t packed_c) const
	{
#ifdef KLAYGE_DEBUG
		uint32_t const limit = diff ? 32 : 16;
		BOOST_ASSERT((diff < 2) && (inten < 8) && (selector < 4) && (packed_c < limit));
#if defined(KLAYGE_COMPILER_CLANGC2)
		KFL_UNUSED(limit);
#endif
#endif
		int c;
		if (diff)
		{
			c = Extend5To8Bits(packed_c);
		}
		else
		{
			c = Extend4To8Bits(packed_c);
		}
		c += GetModifier(inten, selector);
		c = MathLib::clamp(c, 0, 255);
		return c;
	}

	// Packs solid color blocks efficiently using a set of small precomputed tables.
	// For random 888 inputs, MSE results are better than Erricson's ETC1 packer in "slow" mode ~9.5% of the time, is slightly worse only ~.01% of the time, and is equal the rest of the time.
	uint64_t TexCompressionETC1::PackETC1UniformBlock(ETC1Block& block, ARGBColor32 const * argb) const
	{
		static uint32_t const next_comp[] = { 1, 2, 0, 1 };

		uint8_t const * color = reinterpret_cast<uint8_t const *>(argb);

		uint32_t best_err = std::numeric_limits<uint32_t>::max();
		uint32_t best_i = 0;
		int best_x = 0, best_packed_c1 = 0, best_packed_c2 = 0;

		// For each possible 8-bit value, there is a precomputed list of diff/inten/selector configurations that allow that 8-bit value to be encoded with no error.
		for (uint32_t i = 0; (i < 3) && (best_err != 0); ++ i)
		{
			uint32_t const c1 = color[next_comp[i + 0]];
			uint32_t const c2 = color[next_comp[i + 1]];

			int const delta_range = 1;
			for (int delta = -delta_range; (delta <= delta_range) && (best_err != 0); ++ delta)
			{
				int const c_plus_delta = MathLib::clamp(color[i] + delta, 0, 255);

				uint16_t const * table;
				if (0 == c_plus_delta)
				{
					table = color8_to_etc_block_config_0_255[0];
				}
				else if (255 == c_plus_delta)
				{
					table = color8_to_etc_block_config_0_255[1];
				}
				else
				{
					table = color8_to_etc_block_config_1_to_254[c_plus_delta - 1];
				}

				do
				{
					uint32_t const x = *table;
					++ table;

#ifdef KLAYGE_DEBUG
					{
						uint32_t diff = x & 1;
						uint32_t inten = (x >> 1) & 7;
						uint32_t selector = (x >> 4) & 3;
						uint32_t p0 = (x >> 8) & 255;
						BOOST_ASSERT(ETC1DecodeValue(diff, inten, selector, p0) == static_cast<uint32_t>(c_plus_delta));
#if defined(KLAYGE_COMPILER_CLANGC2)
						KFL_UNUSED(diff);
						KFL_UNUSED(inten);
						KFL_UNUSED(selector);
						KFL_UNUSED(p0);
#endif
					}
#endif

					uint32_t const row = x & 0xFF;
					BOOST_ASSERT(row < 64);
					KLAYGE_ASSUME(row < 64);
					uint16_t const * inverse_table = ETC1_INVERSE_LOOKUP[row];
					uint16_t p1 = inverse_table[c1];
					uint16_t p2 = inverse_table[c2];
					uint32_t const trial_err = MathLib::sqr(c_plus_delta - color[i])
						+ MathLib::sqr(p1 >> 8) + MathLib::sqr(p2 >> 8);
					if (trial_err < best_err)
					{
						best_err = trial_err;
						best_x = x;
						best_packed_c1 = p1 & 0xFF;
						best_packed_c2 = p2 & 0xFF;
						best_i = i;
						if (0 == best_err)
						{
							break;
						}
					}
				} while (*table != 0xFFFF);
			}
		}

		uint32_t const diff = best_x & 1;
		uint32_t const inten = (best_x >> 1) & 7;

		block.cw_diff_flip = static_cast<uint8_t>(((inten | (inten << 3)) << 2) | (diff << 1));

		uint32_t const etc1_selector = selector_index_to_etc1[(best_x >> 4) & 3];
		memset(&block.msb, (etc1_selector & 2) ? 0xFF : 0, 2);
		memset(&block.lsb, (etc1_selector & 1) ? 0xFF : 0, 2);

		uint8_t* bytes = &block.r;
		uint32_t const best_packed_c0 = (best_x >> 8) & 255;
		if (diff)
		{
			bytes[best_i] = static_cast<uint8_t>(best_packed_c0 << 3);
			bytes[next_comp[best_i + 0]] = static_cast<uint8_t>(best_packed_c1 << 3);
			bytes[next_comp[best_i + 1]] = static_cast<uint8_t>(best_packed_c2 << 3);
		}
		else
		{
			bytes[best_i] = static_cast<uint8_t>(best_packed_c0 | (best_packed_c0 << 4));
			bytes[next_comp[best_i + 0]] = static_cast<uint8_t>(best_packed_c1 | (best_packed_c1 << 4));
			bytes[next_comp[best_i + 1]] = static_cast<uint8_t>(best_packed_c2 | (best_packed_c2 << 4));
		}

		return best_err;
	}

	uint32_t TexCompressionETC1::PackETC1UniformPartition(Results& results, uint32_t num_colors, ARGBColor32 const * argb,
			bool use_diff, ARGBColor32 const * base_color5_unscaled) const
	{
		static uint32_t const next_comp[] = { 1, 2, 0, 1 };

		uint8_t const * color = reinterpret_cast<uint8_t const *>(argb);

		uint32_t best_err = std::numeric_limits<uint32_t>::max();
		uint32_t best_i = 0;
		int best_x = 0, best_packed_c1 = 0, best_packed_c2 = 0;

		// For each possible 8-bit value, there is a precomputed list of diff/inten/selector configurations that allow that 8-bit value to be encoded with no error.
		for (uint32_t i = 0; (i < 3) && (best_err != 0); ++ i)
		{
			uint32_t const c1 = color[next_comp[i + 0]];
			uint32_t const c2 = color[next_comp[i + 1]];

			int const delta_range = 1;
			for (int delta = -delta_range; (delta <= delta_range) && (best_err != 0); ++ delta)
			{
				int const c_plus_delta = MathLib::clamp(color[i] + delta, 0, 255);

				uint16_t const * table;
				if (0 == c_plus_delta)
				{
					table = color8_to_etc_block_config_0_255[0];
				}
				else if (255 == c_plus_delta)
				{
					table = color8_to_etc_block_config_0_255[1];
				}
				else
				{
					table = color8_to_etc_block_config_1_to_254[c_plus_delta - 1];
				}

				do
				{
					uint32_t const x = *table;
					++ table;
					uint32_t const diff = x & 1;
					if (static_cast<uint32_t>(use_diff) != diff)
					{
						if (0xFFFF == *table)
						{
							break;
						}
						continue;
					}

					if (diff && base_color5_unscaled)
					{
						int const p0 = (x >> 8) & 0xFF;
						int delta0 = p0 - static_cast<int>((*base_color5_unscaled)[i]);
						if ((delta0 < -4) || (delta0 > 3))
						{
							if (0xFFFF == *table)
							{
								break;
							}
							continue;
						}
					}

#ifdef KLAYGE_DEBUG
					{
						uint32_t const inten = (x >> 1) & 7;
						uint32_t const selector = (x >> 4) & 3;
						uint32_t const p0 = (x >> 8) & 0xFF;
						BOOST_ASSERT(ETC1DecodeValue(diff, inten, selector, p0) == static_cast<uint32_t>(c_plus_delta));
#if defined(KLAYGE_COMPILER_CLANGC2)
						KFL_UNUSED(inten);
						KFL_UNUSED(selector);
						KFL_UNUSED(p0);
#endif
					}
#endif

					uint32_t const row = x & 0xFF;
					BOOST_ASSERT(row < 64);
					KLAYGE_ASSUME(row < 64);
					uint16_t const * inverse_table = ETC1_INVERSE_LOOKUP[row];
					uint16_t p1 = inverse_table[c1];
					uint16_t p2 = inverse_table[c2];

					if (diff && base_color5_unscaled)
					{
						int delta1 = (p1 & 0xFF) - static_cast<int>((*base_color5_unscaled)[next_comp[i + 0]]);
						int delta2 = (p2 & 0xFF) - static_cast<int>((*base_color5_unscaled)[next_comp[i + 1]]);
						if ((delta1 < -4) || (delta1 > 3) || (delta2 < -4) || (delta2 > 3))
						{
							if (0xFFFF == *table)
							{
								break;
							}
							continue;
						}
					}

					uint32_t const trial_err = MathLib::sqr(c_plus_delta - color[i])
						+ MathLib::sqr(p1 >> 8) + MathLib::sqr(p2 >> 8);
					if (trial_err < best_err)
					{
						best_err = trial_err;
						best_x = x;
						best_packed_c1 = p1 & 0xFF;
						best_packed_c2 = p2 & 0xFF;
						best_i = i;
						if (0 == best_err)
						{
							break;
						}
					}
				} while (*table != 0xFFFF);
			}
		}

		if (std::numeric_limits<uint32_t>::max() == best_err)
		{
			return best_err;
		}

		best_err *= num_colors;

		results.block_color4_ = !(best_x & 1);
		results.block_inten_table_ = (best_x >> 1) & 7;
		results.selectors_.assign(num_colors, (best_x >> 4) & 3);

		uint32_t const best_packed_c0 = (best_x >> 8) & 255;
		results.block_color_unscaled_[best_i] = static_cast<uint8_t>(best_packed_c0);
		results.block_color_unscaled_[next_comp[best_i + 0]] = static_cast<uint8_t>(best_packed_c1);
		results.block_color_unscaled_[next_comp[best_i + 1]] = static_cast<uint8_t>(best_packed_c2);
		results.error_ = best_err;

		return best_err;
	}

	void TexCompressionETC1::InitSolver(Params const & params, Results& result)
	{
		// This version is hardcoded for 8 pixel subblocks.
		BOOST_ASSERT(8 == params.num_src_pixels_);

		params_ = &params;
		result_ = &result;

		uint32_t const N = 8;

		limit_ = params_->use_color4_ ? 15 : 31;

		avg_color_ = float3(0, 0, 0);
		for (uint32_t i = 0; i < N; ++ i)
		{
			ARGBColor32 const c = params_->src_pixels_[i];
			float3 const fc(static_cast<float>(c.r()),
				static_cast<float>(c.g()),
				static_cast<float>(c.b()));

			avg_color_ += fc;

			luma_[i] = static_cast<uint16_t>(c.r() + c.g() + c.b());
			sorted_luma_[0][i] = i;
		}
		avg_color_ /= N;

		br_ = MathLib::clamp(static_cast<int>(avg_color_[0] * limit_ / 255.0f + 0.5f), 0, limit_);
		bg_ = MathLib::clamp(static_cast<int>(avg_color_[1] * limit_ / 255.0f + 0.5f), 0, limit_);
		bb_ = MathLib::clamp(static_cast<int>(avg_color_[2] * limit_ / 255.0f + 0.5f), 0, limit_);

		if (params_->quality_ <= TCM_Balanced)
		{
			sorted_luma_indices_ = IndirectRadixSort(N, sorted_luma_[0], sorted_luma_[1], luma_, 0, sizeof(luma_[0]), false);
			sorted_luma_ptr_ = sorted_luma_[0];
			if (sorted_luma_indices_ == sorted_luma_[0])
			{
				sorted_luma_ptr_ = sorted_luma_[1];
			}

			for (uint32_t i = 0; i < N; ++ i)
			{
				sorted_luma_ptr_[i] = luma_[sorted_luma_indices_[i]];
			}
		}

		best_solution_.Clear();
	}

	bool TexCompressionETC1::Solve()
	{
		uint32_t const n = params_->num_src_pixels_;
		int const scan_delta_size = params_->scan_delta_size_;

		// Scan through a subset of the 3D lattice centered around the avg block color trying each 3D (555 or 444) lattice point as a potential block color.
		// Each time a better solution is found try to refine the current solution's block color based of the current selectors and intensity table index.
		for (int zdi = 0; zdi < scan_delta_size; ++ zdi)
		{
			int const zd = params_->scan_deltas_[zdi];
			int const mbb = bb_ + zd;
			if (mbb < 0)
			{
				continue;
			}
			else if (mbb > limit_)
			{
				break;
			}

			for (int ydi = 0; ydi < scan_delta_size; ++ ydi)
			{
				int const yd = params_->scan_deltas_[ydi];
				int const mbg = bg_ + yd;
				if (mbg < 0)
				{
					continue;
				}
				else if (mbg > limit_)
				{
					break;
				}

				for (int xdi = 0; xdi < scan_delta_size; ++ xdi)
				{
					int const xd = params_->scan_deltas_[xdi];
					int const mbr = br_ + xd;
					if (mbr < 0)
					{
						continue;
					}
					else if (mbr > limit_)
					{
						break;
					}

					ETC1SolutionCoordinates coords(mbr, mbg, mbb, 0, params_->use_color4_);
					if (TCM_Quality == params_->quality_)
					{
						if (!this->EvaluateSolution(coords, trial_solution_, best_solution_))
						{
							continue;
						}
					}
					else
					{
						if (!this->EvaluateSolutionFast(coords, trial_solution_, best_solution_))
						{
							continue;
						}
					}

					// Now we have the input block, the avg. color of the input pixels, a set of trial selector indices, and the block color+intensity index.
					// Now, for each component, attempt to refine the current solution by solving a simple linear equation. For example, for 4 colors:
					// The goal is:
					// pixel0 - (block_color+inten_table[selector0]) + pixel1 - (block_color+inten_table[selector1]) + pixel2 - (block_color+inten_table[selector2]) + pixel3 - (block_color+inten_table[selector3]) = 0
					// Rearranging this:
					// (pixel0 + pixel1 + pixel2 + pixel3) - (block_color+inten_table[selector0]) - (block_color+inten_table[selector1]) - (block_color+inten_table[selector2]) - (block_color+inten_table[selector3]) = 0
					// (pixel0 + pixel1 + pixel2 + pixel3) - block_color - inten_table[selector0] - block_color-inten_table[selector1] - block_color-inten_table[selector2] - block_color-inten_table[selector3] = 0
					// (pixel0 + pixel1 + pixel2 + pixel3) - 4*block_color - inten_table[selector0] - inten_table[selector1] - inten_table[selector2] - inten_table[selector3] = 0
					// (pixel0 + pixel1 + pixel2 + pixel3) - 4*block_color - (inten_table[selector0] + inten_table[selector1] + inten_table[selector2] + inten_table[selector3]) = 0
					// (pixel0 + pixel1 + pixel2 + pixel3)/4 - block_color - (inten_table[selector0] + inten_table[selector1] + inten_table[selector2] + inten_table[selector3])/4 = 0
					// block_color = (pixel0 + pixel1 + pixel2 + pixel3)/4 - (inten_table[selector0] + inten_table[selector1] + inten_table[selector2] + inten_table[selector3])/4
					// So what this means:
					// optimal_block_color = avg_input - avg_inten_delta
					// So the optimal block color can be computed by taking the average block color and subtracting the current average of the intensity delta.
					// Unfortunately, optimal_block_color must then be quantized to 555 or 444 so it's not always possible to improve matters using this formula.
					// Also, the above formula is for unclamped intensity deltas. The actual implementation takes into account clamping.

					uint32_t const max_refinement_trials = (TCM_Speed == params_->quality_) ? 2 : ((0 == (xd | yd | zd)) ? 4 : 2);
					for (uint32_t refinement_trial = 0; refinement_trial < max_refinement_trials; ++ refinement_trial)
					{
						uint8_t const * selectors = best_solution_.selectors_;

						int delta_sum_r = 0, delta_sum_g = 0, delta_sum_b = 0;
						ARGBColor32 const base_color = best_solution_.coords_.ScaledColor();
						for (uint32_t r = 0; r < n; ++ r)
						{
							uint32_t const s = *selectors;
							++ selectors;
							int const yd0 = GetModifier(best_solution_.coords_.inten_table_, s);
							// Compute actual delta being applied to each pixel, taking into account clamping.
							delta_sum_r += MathLib::clamp(static_cast<int>(base_color.r()) + yd0, 0, 255) - base_color.r();
							delta_sum_g += MathLib::clamp(static_cast<int>(base_color.g()) + yd0, 0, 255) - base_color.g();
							delta_sum_b += MathLib::clamp(static_cast<int>(base_color.b()) + yd0, 0, 255) - base_color.b();
						}
						if (!delta_sum_r && !delta_sum_g && !delta_sum_b)
						{
							break;
						}
						float const avg_delta_r_f = static_cast<float>(delta_sum_r) / n;
						float const avg_delta_g_f = static_cast<float>(delta_sum_g) / n;
						float const avg_delta_b_f = static_cast<float>(delta_sum_b) / n;
						int const br1 = MathLib::clamp(static_cast<int>((avg_color_[0] - avg_delta_r_f) * limit_ / 255.0f + 0.5f), 0, limit_);
						int const bg1 = MathLib::clamp(static_cast<int>((avg_color_[1] - avg_delta_g_f) * limit_ / 255.0f + 0.5f), 0, limit_);
						int const bb1 = MathLib::clamp(static_cast<int>((avg_color_[2] - avg_delta_b_f) * limit_ / 255.0f + 0.5f), 0, limit_);

						bool skip = false;

						if ((mbr == br1) && (mbg == bg1) && (mbb == bb1))
						{
							skip = true;
						}
						else if ((br1 == static_cast<int>(best_solution_.coords_.unscaled_color_.r()))
							&& (bg1 == static_cast<int>(best_solution_.coords_.unscaled_color_.g()))
							&& (bb1 == static_cast<int>(best_solution_.coords_.unscaled_color_.b())))
						{
							skip = true;
						}
						else if ((br_ == br1) && (bg_ == bg1) && (bb_ == bb1))
						{
							skip = true;
						}

						if (skip)
						{
							break;
						}

						ETC1SolutionCoordinates coords1(br1, bg1, bb1, 0, params_->use_color4_);
						if (TCM_Quality == params_->quality_)
						{
							if (!this->EvaluateSolution(coords1, trial_solution_, best_solution_))
							{
								break;
							}
						}
						else
						{
							if (!this->EvaluateSolutionFast(coords1, trial_solution_, best_solution_))
							{
								break;
							}
						}
					} // refinement_trial
				} // xdi
			} // ydi
		} // zdi

		if (!best_solution_.valid_)
		{
			result_->error_ = std::numeric_limits<uint64_t>::max();
			return false;
		}

		uint8_t const * selectors = best_solution_.selectors_;

#ifdef KLAYGE_DEBUG
		{
			ARGBColor32 block_colors[4];
			best_solution_.coords_.BlockColors(block_colors);

			ARGBColor32 const * src_pixels = params_->src_pixels_;
			uint64_t actual_err = 0;
			for (uint32_t i = 0; i < n; ++ i)
			{
				actual_err += MathLib::sqr(src_pixels[i].r() - block_colors[selectors[i]].r())
					+ MathLib::sqr(src_pixels[i].g() - block_colors[selectors[i]].g())
					+ MathLib::sqr(src_pixels[i].b() - block_colors[selectors[i]].b());
			}

			BOOST_ASSERT(actual_err == best_solution_.error_);
		}
#endif      

		result_->error_ = best_solution_.error_;

		result_->block_color_unscaled_ = best_solution_.coords_.unscaled_color_;
		result_->block_color4_ = best_solution_.coords_.color4_;

		result_->block_inten_table_ = best_solution_.coords_.inten_table_;
		result_->selectors_.assign(selectors, selectors + n);

		return true;
	}

	bool TexCompressionETC1::EvaluateSolution(ETC1SolutionCoordinates const & coords, PotentialSolution& trial_solution, PotentialSolution& best_solution)
	{
		trial_solution.valid_ = false;

		if (params_->constrain_against_base_color5_)
		{
			int const dr = coords.unscaled_color_.r() - params_->base_color5_.r();
			int const dg = coords.unscaled_color_.g() - params_->base_color5_.g();
			int const db = coords.unscaled_color_.b() - params_->base_color5_.b();

			if ((MathLib::min3(dr, dg, db) < -4) || (MathLib::max3(dr, dg, db) > 3))
			{
				return false;
			}
		}

		ARGBColor32 const base_color = coords.ScaledColor();

		uint32_t const N = 8;

		trial_solution.error_ = std::numeric_limits<uint64_t>::max();

		for (uint32_t inten_table = 0; inten_table < 8; ++ inten_table)
		{
			ARGBColor32 block_colors[4];
			for (uint32_t s = 0; s < 4; ++ s)
			{
				int const yd = GetModifier(inten_table, s);
				block_colors[s] = From4Ints(0, base_color.r() + yd, base_color.g() + yd, base_color.b() + yd);
			}

			uint64_t total_err = 0;

			ARGBColor32 const * src_pixels = params_->src_pixels_;
			for (uint32_t c = 0; c < N; ++ c)
			{
				ARGBColor32 src_pixel = *src_pixels;
				++ src_pixels;

				uint32_t best_selector_index = 0;
				uint32_t best_err = MathLib::sqr(src_pixel.r() - block_colors[0].r())
					+ MathLib::sqr(src_pixel.g() - block_colors[0].g())
					+ MathLib::sqr(src_pixel.b() - block_colors[0].b());

				uint32_t trial_err = MathLib::sqr(src_pixel.r() - block_colors[1].r())
					+ MathLib::sqr(src_pixel.g() - block_colors[1].g())
					+ MathLib::sqr(src_pixel.b() - block_colors[1].b());
				if (trial_err < best_err)
				{
					best_err = trial_err;
					best_selector_index = 1;
				}

				trial_err = MathLib::sqr(src_pixel.r() - block_colors[2].r())
					+ MathLib::sqr(src_pixel.g() - block_colors[2].g())
					+ MathLib::sqr(src_pixel.b() - block_colors[2].b());
				if (trial_err < best_err)
				{
					best_err = trial_err;
					best_selector_index = 2;
				}

				trial_err = MathLib::sqr(src_pixel.r() - block_colors[3].r())
					+ MathLib::sqr(src_pixel.g() - block_colors[3].g())
					+ MathLib::sqr(src_pixel.b() - block_colors[3].b());
				if (trial_err < best_err)
				{
					best_err = trial_err;
					best_selector_index = 3;
				}

				temp_selectors_[c] = static_cast<uint8_t>(best_selector_index);

				total_err += best_err;
				if (total_err >= trial_solution.error_)
				{
					break;
				}
			}

			if (total_err < trial_solution.error_)
			{
				trial_solution.error_ = total_err;
				trial_solution.coords_.inten_table_ = inten_table;
				memcpy(trial_solution.selectors_, temp_selectors_, 8);
				trial_solution.valid_ = true;
			}
		}
		trial_solution.coords_.unscaled_color_ = coords.unscaled_color_;
		trial_solution.coords_.color4_ = params_->use_color4_;

		bool success = false;
		if (trial_solution.error_ < best_solution.error_)
		{
			best_solution = trial_solution;
			success = true;
		}

		return success;
	}

	bool TexCompressionETC1::EvaluateSolutionFast(ETC1SolutionCoordinates const & coords, PotentialSolution& trial_solution, PotentialSolution& best_solution)
	{
		if (params_->constrain_against_base_color5_)
		{
			int const dr = coords.unscaled_color_.r() - params_->base_color5_.r();
			int const dg = coords.unscaled_color_.g() - params_->base_color5_.g();
			int const db = coords.unscaled_color_.b() - params_->base_color5_.b();

			if ((MathLib::min3(dr, dg, db) < -4) || (MathLib::max3(dr, dg, db) > 3))
			{
				trial_solution.valid_ = false;
				return false;
			}
		}

		ARGBColor32 const base_color = coords.ScaledColor();

		uint32_t const N = 8;

		trial_solution.error_ = std::numeric_limits<uint64_t>::max();

		for (int inten_table = 7; inten_table >= 0; -- inten_table)
		{
			uint32_t block_inten[4];
			ARGBColor32 block_colors[4];
			for (uint32_t s = 0; s < 4; ++ s)
			{
				int const yd = GetModifier(inten_table, s);
				ARGBColor32 const block_color = From4Ints(255, base_color.r() + yd, base_color.g() + yd,
					base_color.b() + yd);
				block_colors[s] = block_color;
				block_inten[s] = block_color.r() + block_color.g() + block_color.b();
			}

			// evaluate_solution_fast() enforces/assumesd a total ordering of the input colors along the intensity (1,1,1) axis to more quickly classify the inputs to selectors.
			// The inputs colors have been presorted along the projection onto this axis, and ETC1 block colors are always ordered along the intensity axis, so this classification is fast.
			// 0   1   2   3
			//   01  12  23
			uint32_t const block_inten_midpoints[3] = { block_inten[0] + block_inten[1],
				block_inten[1] + block_inten[2], block_inten[2] + block_inten[3] };

			uint64_t total_err = 0;
			ARGBColor32 const * src_pixels = params_->src_pixels_;
			if (sorted_luma_ptr_[N - 1] * 2 < block_inten_midpoints[0])
			{
				if (block_inten[0] > sorted_luma_ptr_[N - 1])
				{
					uint32_t const min_err = MathLib::abs(static_cast<int>(block_inten[0] - sorted_luma_ptr_[N - 1]));
					if (min_err >= trial_solution.error_)
					{
						continue;
					}
				}

				memset(&temp_selectors_[0], 0, N);

				for (uint32_t c = 0; c < N; ++ c)
				{
					total_err += MathLib::sqr(block_colors[0].r() - src_pixels[c].r())
						+ MathLib::sqr(block_colors[0].g() - src_pixels[c].g())
						+ MathLib::sqr(block_colors[0].b() - src_pixels[c].b());
				}
			}
			else if (sorted_luma_ptr_[0] * 2 >= block_inten_midpoints[2])
			{
				if (sorted_luma_ptr_[0] > block_inten[3])
				{
					uint32_t const min_err = MathLib::abs(static_cast<int>(sorted_luma_ptr_[0] - block_inten[3]));
					if (min_err >= trial_solution.error_)
					{
						continue;
					}
				}

				memset(&temp_selectors_[0], 3, N);

				for (uint32_t c = 0; c < N; ++ c)
				{
					total_err += MathLib::sqr(block_colors[3].r() - src_pixels[c].r())
						+ MathLib::sqr(block_colors[3].g() - src_pixels[c].g())
						+ MathLib::sqr(block_colors[3].b() - src_pixels[c].b());
				}
			}
			else
			{
				uint32_t cur_selector = 0;
				for (uint32_t c = 0; c < N; ++ c)
				{
					uint32_t const y = sorted_luma_ptr_[c];
					while (y * 2 >= block_inten_midpoints[cur_selector])
					{
						++ cur_selector;
						if (cur_selector > 2)
						{
							while (c < N)
							{
								uint32_t const sorted_pixel_index = sorted_luma_indices_[c];
								temp_selectors_[sorted_pixel_index] = 3;
								total_err += MathLib::sqr(block_colors[3].r() - src_pixels[sorted_pixel_index].r())
									+ MathLib::sqr(block_colors[3].g() - src_pixels[sorted_pixel_index].g())
									+ MathLib::sqr(block_colors[3].b() - src_pixels[sorted_pixel_index].b());
								++ c;
							}
							break;
						}
					}

					if (c < N)
					{
						uint32_t const sorted_pixel_index = sorted_luma_indices_[c];
						temp_selectors_[sorted_pixel_index] = static_cast<uint8_t>(cur_selector);
						total_err += MathLib::sqr(block_colors[cur_selector].r() - src_pixels[sorted_pixel_index].r())
							+ MathLib::sqr(block_colors[cur_selector].g() - src_pixels[sorted_pixel_index].g())
							+ MathLib::sqr(block_colors[cur_selector].b() - src_pixels[sorted_pixel_index].b());
					}
				}
			}

			if (total_err < trial_solution.error_)
			{
				trial_solution.error_ = total_err;
				trial_solution.coords_.inten_table_ = inten_table;
				memcpy(trial_solution.selectors_, temp_selectors_, N);
				trial_solution.valid_ = true;
				if (!total_err)
				{
					break;
				}
			}
		}
		trial_solution.coords_.unscaled_color_ = coords.unscaled_color_;
		trial_solution.coords_.color4_ = params_->use_color4_;

		bool success = false;
		if (trial_solution.error_ < best_solution.error_)
		{
			best_solution = trial_solution;
			success = true;
		}

		return success;
	}


	TexCompressionETC2RGB8::TexCompressionETC2RGB8()
	{
		block_width_ = block_height_ = 4;
		block_depth_ = 1;
		block_bytes_ = NumFormatBytes(EF_ETC2_BGR8) * 4;
		decoded_fmt_ = EF_ARGB8;

		etc1_codec_ = MakeSharedPtr<TexCompressionETC1>();
	}

	void TexCompressionETC2RGB8::EncodeBlock(void* output, void const * input, TexCompressionMethod method)
	{
		KFL_UNUSED(output);
		KFL_UNUSED(input);
		KFL_UNUSED(method);

		// TODO
	}

	void TexCompressionETC2RGB8::DecodeBlock(void* output, void const * input)
	{
		BOOST_ASSERT(output);
		BOOST_ASSERT(input);

		ARGBColor32* argb = static_cast<ARGBColor32*>(output);
		ETC2Block const & etc2 = *static_cast<ETC2Block const *>(input);

		if (etc2.etc1.cw_diff_flip & 0x2)
		{
			int const dr = etc2.etc1.r & 0x7;
			int const r = (etc2.etc1.r >> 3) - (dr & 0x4) + (dr & 0x3);
			int const dg = etc2.etc1.g & 0x7;
			int const g = (etc2.etc1.g >> 3) - (dg & 0x4) + (dg & 0x3);
			int const db = etc2.etc1.b & 0x7;
			int const b = (etc2.etc1.b >> 3) - (db & 0x4) + (db & 0x3);

			if (r & 0xFFE0)
			{
				this->DecodeETCTModeInternal(argb, etc2.etc2_t_mode, false);
			}
			else if (g & 0xFFE0)
			{
				this->DecodeETCHModeInternal(argb, etc2.etc2_h_mode, false);
			}
			else if (b & 0xFFE0)
			{
				this->DecodeETCPlanarModeInternal(argb, etc2.etc2_planar_mode);
			}
			else
			{
				etc1_codec_->DecodeETCDifferentialModeInternal(argb, etc2.etc1, false);
			}
		}
		else
		{
			etc1_codec_->DecodeETCIndividualModeInternal(argb, etc2.etc1);
		}
	}

	void TexCompressionETC2RGB8::DecodeETCTModeInternal(ARGBColor32* argb, ETC2TModeBlock const & etc2, bool alpha)
	{
		BOOST_ASSERT(argb);

		static int const distance_table[8] = { 3, 6, 11, 16, 23, 32, 41, 64 };

		int const r1 = ((etc2.r1 >> 1) & 0xC) | (etc2.r1 & 0x3);
		int const g1 = etc2.g1_b1 >> 4;
		int const b1 = etc2.g1_b1 & 0xF;
		int const r2 = etc2.r2_g2 >> 4;
		int const g2 = etc2.r2_g2 & 0xF;
		int const b2 = etc2.b2_d >> 4;
		int const da = (etc2.b2_d & 0xC) >> 1;
		int const db = etc2.b2_d & 0x1;

		int const base_clr1[] =
		{
			Extend4To8Bits(r1),
			Extend4To8Bits(g1),
			Extend4To8Bits(b1)
		};

		int const base_clr2[] =
		{
			Extend4To8Bits(r2),
			Extend4To8Bits(g2),
			Extend4To8Bits(b2)
		};

		int const distance = distance_table[da | db];
		ARGBColor32 const modified_clr[] =
		{
			From4Ints(255, base_clr1[0], base_clr1[1], base_clr1[2]),
			From4Ints(255, base_clr2[0] + distance, base_clr2[1] + distance, base_clr2[2] + distance),
			From4Ints(255, base_clr2[0], base_clr2[1], base_clr2[2]),
			From4Ints(255, base_clr2[0] - distance, base_clr2[1] - distance, base_clr2[2] - distance)
		};

		for (int x = 0; x < 4; ++ x)
		{
			for (int y = 0; y < 4; ++ y)
			{
				int bit_index = (x * 4 + y) ^ 0x8;
				int msb = (etc2.msb >> bit_index) & 0x1;
				int lsb = (etc2.lsb >> bit_index) & 0x1;
				int pixel_index = msb * 2 + lsb;
				if (alpha && (2 == pixel_index))
				{
					argb[y * 4 + x] = ARGBColor32(0, 0, 0, 0);
				}
				else
				{
					argb[y * 4 + x] = modified_clr[pixel_index];
				}
			}
		}
	}

	void TexCompressionETC2RGB8::DecodeETCHModeInternal(ARGBColor32* argb, ETC2HModeBlock const & etc2, bool alpha)
	{
		BOOST_ASSERT(argb);

		static int const distance_table[8] = { 3, 6, 11, 16, 23, 32, 41, 64 };

		int const r1 = (etc2.r1_g1 >> 3) & 0xF;
		int const g1 = ((etc2.r1_g1 & 0x7) << 1) | ((etc2.g1_b1 >> 4) & 0x1);
		int const b1 = (etc2.g1_b1 & 0x8) | ((etc2.g1_b1 & 0x3) << 1) | ((etc2.b1_r2_g2 >> 7) & 0x1);
		int const r2 = (etc2.b1_r2_g2 >> 3) & 0xF;
		int const g2 = ((etc2.b1_r2_g2 & 0x7) << 1) | ((etc2.g2_b2_d >> 7) & 0x1);
		int const b2 = (etc2.g2_b2_d >> 3) & 0xF;
		int const da = etc2.g2_b2_d & 0x04;
		int const db = etc2.g2_b2_d & 0x01;

		uint8_t const base_clr1[] =
		{
			Extend4To8Bits(r1),
			Extend4To8Bits(g1),
			Extend4To8Bits(b1)
		};

		uint8_t const base_clr2[] =
		{
			Extend4To8Bits(r2),
			Extend4To8Bits(g2),
			Extend4To8Bits(b2)
		};

		int const ordering = ARGBColor32(0, base_clr1[0], base_clr1[1], base_clr1[2]).ARGB()
			>= ARGBColor32(0, base_clr2[0], base_clr2[1], base_clr2[2]).ARGB();
		int distance = distance_table[da | (db << 1) | ordering];
		ARGBColor32 const modified_clr[] =
		{
			From4Ints(255, base_clr1[0] + distance, base_clr1[1] + distance, base_clr1[2] + distance),
			From4Ints(255, base_clr1[0] - distance, base_clr1[1] - distance, base_clr1[2] - distance),
			From4Ints(255, base_clr2[0] + distance, base_clr2[1] + distance, base_clr2[2] + distance),
			From4Ints(255, base_clr2[0] - distance, base_clr2[1] - distance, base_clr2[2] - distance)
		};

		for (int x = 0; x < 4; ++ x)
		{
			for (int y = 0; y < 4; ++ y)
			{
				int bit_index = (x * 4 + y) ^ 0x8;
				int msb = (etc2.msb >> bit_index) & 0x1;
				int lsb = (etc2.lsb >> bit_index) & 0x1;
				int pixel_index = msb * 2 + lsb;
				if (alpha && (2 == pixel_index))
				{
					argb[y * 4 + x] = ARGBColor32(0, 0, 0, 0);
				}
				else
				{
					argb[y * 4 + x] = modified_clr[pixel_index];
				}
			}
		}
	}

	void TexCompressionETC2RGB8::DecodeETCPlanarModeInternal(ARGBColor32* argb, ETC2PlanarModeBlock const & etc2)
	{
		int const ro = (etc2.ro_go >> 1) & 0x3F;
		int const go = ((etc2.ro_go & 0x1) << 6) | ((etc2.go_bo >> 1) & 0x3F);
		int const bo = ((etc2.go_bo & 0x1) << 5) | (etc2.bo & 0x18) | ((etc2.bo & 0x3) << 1) | ((etc2.bo_rh >> 7) & 0x1);
		int const rh = (((etc2.bo_rh >> 2) & 0x1F) << 1) | (etc2.bo_rh & 0x1);
		int const gh = etc2.gh_bh >> 1;
		int const bh = ((etc2.gh_bh & 0x1) << 5) | ((etc2.bh_rv >> 3) & 0x1F);
		int const rv = ((etc2.bh_rv & 0x7) << 3) | ((etc2.rv_gv >> 5) & 0x7);
		int const gv = ((etc2.rv_gv & 0x1F) << 2) | ((etc2.gv_bv >> 6) & 0x3);
		int const bv = etc2.gv_bv & 0x3F;

		int const o[] =
		{
			Extend6To8Bits(ro),
			Extend7To8Bits(go),
			Extend6To8Bits(bo)
		};

		int const h[] =
		{
			Extend6To8Bits(rh),
			Extend7To8Bits(gh),
			Extend6To8Bits(bh)
		};

		int const v[] =
		{
			Extend6To8Bits(rv),
			Extend7To8Bits(gv),
			Extend6To8Bits(bv)
		};

		for (int y = 0; y < 4; ++ y)
		{
			for (int x = 0; x < 4; ++ x)
			{
				argb[y * 4 + x] = From4Ints(255,
					(x * (h[0] - o[0]) + y * (v[0] - o[0]) + 4 * o[0] + 2) >> 2,
					(x * (h[1] - o[1]) + y * (v[1] - o[1]) + 4 * o[1] + 2) >> 2,
					(x * (h[2] - o[2]) + y * (v[2] - o[2]) + 4 * o[2] + 2) >> 2);
			}
		}
	}


	TexCompressionETC2RGB8A1::TexCompressionETC2RGB8A1()
	{
		block_width_ = block_height_ = 4;
		block_depth_ = 1;
		block_bytes_ = NumFormatBytes(EF_ETC2_A1BGR8) * 4;
		decoded_fmt_ = EF_ARGB8;

		etc1_codec_ = MakeSharedPtr<TexCompressionETC1>();
		etc2_rgb8_codec_ = MakeSharedPtr<TexCompressionETC2RGB8>();
	}

	void TexCompressionETC2RGB8A1::EncodeBlock(void* output, void const * input, TexCompressionMethod method)
	{
		KFL_UNUSED(output);
		KFL_UNUSED(input);
		KFL_UNUSED(method);

		// TODO
	}

	void TexCompressionETC2RGB8A1::DecodeBlock(void* output, void const * input)
	{
		BOOST_ASSERT(output);
		BOOST_ASSERT(input);

		ARGBColor32* argb = static_cast<ARGBColor32*>(output);
		ETC2Block const & etc2 = *static_cast<ETC2Block const *>(input);

		int const dr = etc2.etc1.r & 0x7;
		int const r = (etc2.etc1.r >> 3) - (dr & 0x4) + (dr & 0x3);
		int const dg = etc2.etc1.g & 0x7;
		int const g = (etc2.etc1.g >> 3) - (dg & 0x4) + (dg & 0x3);
		int const db = etc2.etc1.b & 0x7;
		int const b = (etc2.etc1.b >> 3) - (db & 0x4) + (db & 0x3);
		int op = etc2.etc1.cw_diff_flip & 0x2;

		if (r & 0xFFE0)
		{
			etc2_rgb8_codec_->DecodeETCTModeInternal(argb, etc2.etc2_t_mode, !op);
		}
		else if (g & 0xFFE0)
		{
			etc2_rgb8_codec_->DecodeETCHModeInternal(argb, etc2.etc2_h_mode, !op);
		}
		else if (b & 0xFFE0)
		{
			etc2_rgb8_codec_->DecodeETCPlanarModeInternal(argb, etc2.etc2_planar_mode);
		}
		else
		{
			etc1_codec_->DecodeETCDifferentialModeInternal(argb, etc2.etc1, !op);
		}
	}
}
