// Pkt.cpp
// KlayGE 文件打包工具
// Ver 1.3.8.1
// 版权所有(C) 龚敏敏, 2002
// Homepage: http://www.enginedev.com
//
// 1.3.8.1
// 重写了打包工具 (2002.10.24)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/LZSS/LZSS.hpp>

#include <iostream>
#include <fstream>

using namespace std;
using namespace KlayGE;

int main(int argc, char* argv[])
{
	if (4 != argc)
	{
		cout << "使用方法:" << endl
			<< "压缩整个目录: Pkt p 目录名 文件名.pkt" << endl;
		return 1;
	}

	cout << "正在压缩，请稍候..." << endl;

	std::ofstream ofs(argv[3], std::ios_base::binary);

	Pkt pkt;
	pkt.Pack(argv[2], ofs);

	cout << "压缩成功" << endl;

	return 0;
}
