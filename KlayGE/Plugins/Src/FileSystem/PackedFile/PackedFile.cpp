// PackedFile.cpp
// KlayGE 打包文件虚拟适配器类 实现文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 2.0.0
// 初次建立 (2003.9.8)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>

#include <cassert>

#include <KlayGE/DiskFile/DiskFile.hpp>
#include <KlayGE/MemFile/MemFile.hpp>
#include <KlayGE/PackedFile/PackedFile.hpp>

namespace KlayGE
{
	PackedFile::PackedFile()
	{
	}

	PackedFile::PackedFile(std::string const & pathName)
	{
		this->Open(pathName);
	}

	bool PackedFile::Open(std::string const & pathName)
	{
		std::string::size_type const offset(pathName.rfind(".pkt/"));
		std::string const pktName(pathName.substr(0, offset + 4));
		std::string const fileName(pathName.substr(offset + 5));

		boost::shared_ptr<DiskFile> pktFile(new DiskFile);
		if (!pktFile->Open(pktName, VFile::OM_Read))
		{
			return false;
		}
		else
		{
			pktFile_ = pktFile;
		}

		try
		{
			unPkt_.Open(pktFile_);

			this->Close();

			unPkt_.LocateFile(fileName);

			file_ = VFilePtr(new MemFile);

			std::vector<U8> data(unPkt_.CurFileSize());
			unPkt_.ReadCurFile(&data[0]);
			file_->Write(&data[0], data.size());
			file_->Rewind();
		}
		catch (...)
		{
			return false;
		}

		return true;
	}

	void PackedFile::Close()
	{
		if (file_)
		{
			file_->Close();
		}
	}

	size_t PackedFile::Length()
	{
		return file_->Length();
	}

	size_t PackedFile::Read(void* data, size_t count)
	{
		assert(data != NULL);

		return file_->Read(data, count);
	}

	size_t PackedFile::Seek(size_t off, SeekMode from)
	{
		return file_->Seek(off, from);
	}

	size_t PackedFile::Tell()
	{
		return file_->Tell();
	}


	PackedFileResIdentifier::PackedFileResIdentifier(std::string const & fileName)
		: fileName_(fileName)
	{
	}

	VFilePtr PackedFileResIdentifier::Load()
	{
		return VFilePtr(new PackedFile(fileName_));
	};
}
