// Unpkt.cpp
// KlayGE 打包文件读取类 实现文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 2.0.0
// 初次建立 (2003.9.18)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/CommFuncs.hpp>
#include <KlayGE/Memory.hpp>
#include <KlayGE/Engine.hpp>
#include <KlayGE/Crc32.hpp>

#include <cassert>
#include <cctype>
#include <string>

#include <KlayGE/MemFile/MemFile.hpp>
#include <KlayGE/PackedFile/Pkt.hpp>

using std::vector;

namespace
{
	using namespace KlayGE;

	const U32 N(4096);				// size of ring buffer
	const U32 F(18);				// upper limit for match_length
	const U32 THRESHOLD(2);		// encode string into position and length
									//   if match_length is greater than this
	const U32 NIL(N);				// index for root of binary search trees


	static U8 textBuf[N + F - 1];	// ring buffer of size N, 
									// with extra F-1 bytes to facilitate string comparison

	// 忽略大小写比较字符串
	/////////////////////////////////////////////////////////////////////////////////
	bool IgnoreCaseCompare(const WString& lhs, const WString& rhs)
	{
		if (lhs.length() != rhs.length())
		{
			return false;
		}

		for (U32 i = 0; i < lhs.length(); ++ i)
		{
			if (std::toupper(lhs[i]) != std::toupper(rhs[i]))
			{
				break;
			}
		}

		if (i != lhs.length())
		{
			return false;
		}

		return true;
	}

	// 把目录表转化成树型结构
	/////////////////////////////////////////////////////////////////////////////////
	void Translate(tree<KlayGE::FileDes>* pOut, VFile& In)
	{
		using namespace KlayGE;

		String str;
		FileDes fd;

		U8 tag;
		U8 len;
		while (In.Tell() != In.Length())
		{
			str.clear();
			In.Read(&tag, sizeof(tag));

			switch (tag)
			{
			case DIT_File:
			case DIT_Dir:
				In.Read(&len, sizeof(len));
				str.resize(len);
				In.Read(&str[0], str.length());

				Convert(fd.fileName, str);
				In.Read(&fd.start, sizeof(U32) * 4 + sizeof(U8));
				pOut->AddChild(fd);

				if (DIT_Dir == tag)
				{
					pOut = (pOut->EndChild() - 1)->Get();
				}
				break;

			case DIT_UnDir:
				pOut = pOut->Parent();
				break;
			}
		}
	}
}

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	UnPkt::UnPkt()
			: curFile_(NULL)
	{
		curDir_ = &dirTable_;
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	UnPkt::~UnPkt()
	{
		this->Close();
	}

	// LZSS解压
	/////////////////////////////////////////////////////////////////////////////////
	void UnPkt::Decode(VFile& Out, VFile& In)
	{
		U32 r(N - F);
		Engine::MemoryInstance().Set(textBuf, ' ', r);

		U32 flags(0);
		U8 c;
		for (;;)
		{
			if (0 == ((flags >>= 1) & 256))
			{
				if (In.Tell() >= In.Length())
				{
					break;
				}

				In.Read(&c, 1);

				flags = c | 0xFF00;		// uses higher byte cleverly
											// to count eight
			}
			if (flags & 1)
			{
				if (In.Tell() >= In.Length())
				{
					break;
				}

				In.Read(&c, 1);

				Out.Write(&c, 1);
				textBuf[r] = c;
				++ r;
				r &= (N - 1);
			}
			else
			{
				if (In.Tell() >= In.Length())
				{
					break;
				}
				In.Read(&c, 1);
				U32 c1 = c;

				if (In.Tell() >= In.Length())
				{
					break;
				}
				In.Read(&c, 1);
				U32 c2 = c;

				c1 |= ((c2 & 0xF0) << 4);
				c2 = (c2 & 0x0F) + THRESHOLD;
				for (U32 k = 0; k <= c2; ++ k)
				{
					c = textBuf[(c1 + k) & (N - 1)];
					Out.Write(&c, 1);
					textBuf[r] = c;
					++ r;
					r &= (N - 1);
				}
			}
		}

		Out.Seek(0, VFile::SM_Begin);
	}

	// 打开打包文件
	/////////////////////////////////////////////////////////////////////////////////
	void UnPkt::Open(const VFile& pktFile)
	{
		Close();

		file_ = pktFile.Clone();

		file_->Read(&mag_, sizeof(mag_));
		Verify(MakeFourCC<'p', 'k', 't', ' '>::value == mag_.magic);
		Verify(3 == mag_.ver);

		file_->Seek(mag_.DTStart, VFile::SM_Begin);

		MemFile DTCom;
		DTCom.CopyFrom(*file_, mag_.DTLength);

		MemFile DT;
		DTCom.Seek(0, VFile::SM_Begin);
		Decode(DT, DTCom);

		DT.Seek(0, VFile::SM_Begin);
		Translate(&dirTable_, DT);
		curDir_ = &dirTable_;
	}

	// 关闭打包文件
	/////////////////////////////////////////////////////////////////////////////////
	void UnPkt::Close()
	{
		file_.Release();
		curFile_ = NULL;
	}

	// 设置打包文件中的当前路径
	/////////////////////////////////////////////////////////////////////////////////
	void UnPkt::Dir(const WString& dirName)
	{
		WString theDirName;
		TransPathName(theDirName, dirName);

		// 识别路径
		if (L'/' == theDirName[0])
		{
			curDir_ = &dirTable_;
		}
		if ((L'.' == theDirName[0]) && (L'/' == theDirName[1]))
		{
			theDirName = dirName.substr(2, dirName.length() - 2);
		}
		if ((L'.' == theDirName[0]) && (L'.' == theDirName[1]) && (L'/' == theDirName[2]))
		{
			if (curDir_ != NULL)
			{
				curDir_ = curDir_->Parent();
			}
		}

		for (;;)
		{
			WString::size_type nPos = theDirName.find_first_of(L'/');
			WString curDir;
			if (nPos == WString::npos)
			{
				curDir = theDirName;
				theDirName.clear();
			}
			else
			{
				curDir	= theDirName.substr(0, nPos);
				theDirName = theDirName.substr(nPos + 1, theDirName.length());
			}

			if (curDir.empty())
			{
				break;
			}

			// 查找目录
			FileIterator iter = curDir_->BeginChild();
			while ((iter != curDir_->EndChild())
						&& !IgnoreCaseCompare((*iter)->RootData().fileName, curDir))
			{
				++ iter;
			}
			if (iter == curDir_->EndChild())
			{
				THR(E_FAIL);
			}
			curDir_ = iter->Get();
		}
	}

	// 在打包文件中定位文件
	/////////////////////////////////////////////////////////////////////////////////
	void UnPkt::LocateFile(const WString& pathName)
	{
		WString thePathName;
		TransPathName(thePathName, pathName);

		WString::size_type nPos = thePathName.find_last_of(L'/');

		WString dir(thePathName, 0, nPos == WString::npos ? 0 : nPos);
		WString name(thePathName, nPos + 1, thePathName.length() - 1 - nPos);

		this->Dir(dir);

		// 查找文件
		FileIterator iter = curDir_->BeginChild();
		while ((iter != curDir_->EndChild())
						&& !IgnoreCaseCompare((*iter)->RootData().fileName, name))
		{
			++ iter;
		}
		if (iter == curDir_->EndChild())
		{
			THR(E_FAIL);
		}

		this->LocateFile(iter);
	}

	void UnPkt::LocateFile(FileIterator iter)
	{
		curFile_ = &((*iter)->RootData());
	}

	// 获取当前文件(解压过的)的字节数
	/////////////////////////////////////////////////////////////////////////////////
	size_t UnPkt::CurFileSize() const
	{
		assert(curFile_ != NULL);

		return curFile_->DeComLength;
	}

	// 读取当前文件(解压过的)
	/////////////////////////////////////////////////////////////////////////////////
	bool UnPkt::ReadCurFile(void* data)
	{
		assert(data != NULL);
		assert(curFile_ != NULL);

		file_->Seek(mag_.FIStart + curFile_->start, VFile::SM_Begin);

		if (curFile_->attr & FA_UnCompressed)
		{
			file_->Read(data, this->CurFileSize());
		}
		else
		{
			MemFile chunk;
			chunk.CopyFrom(*file_, this->CurFileCompressedSize());

			MemFile out;
			chunk.Rewind();
			Decode(out, chunk);

			out.Read(data, out.Length());
		}

		if (Crc32::CrcMem(static_cast<U8*>(data), this->CurFileSize()) != curFile_->crc32)
		{
			return false;	// CRC32 错误
		}

		return true;
	}

	// 获取当前文件(没解压过的)的字节数
	/////////////////////////////////////////////////////////////////////////////////
	size_t UnPkt::CurFileCompressedSize() const
	{
		assert(curFile_ != NULL);

		return curFile_->length;
	}

	// 读取当前文件(没解压过的)
	/////////////////////////////////////////////////////////////////////////////////
	void UnPkt::ReadCurFileCompressed(void* data)
	{
		assert(data != NULL);
		assert(curFile_ != NULL);

		file_->Seek(mag_.FIStart + curFile_->start, VFile::SM_Begin);
		file_->Read(data, this->CurFileCompressedSize());
	}

	// 获取当前目录下的第一个文件的迭代子
	/////////////////////////////////////////////////////////////////////////////////
	UnPkt::FileIterator UnPkt::BeginFile()
	{
		return curDir_->BeginChild();
	}

	// 获取当前目录下的最后一个文件的下一个迭代子
	/////////////////////////////////////////////////////////////////////////////////
	UnPkt::FileIterator UnPkt::EndFile()
	{
		return curDir_->EndChild();
	}
}