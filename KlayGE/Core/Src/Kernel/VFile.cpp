#include <KlayGE/KlayGE.hpp>

#include <KlayGE/VFile.hpp>

namespace KlayGE
{
	class NullVFile : public VFile
	{
	public:
		void Close()
			{ }

		size_t Length()
			{ return 0; }
		void Length(size_t newLen)
			{ }

		size_t Write(void const * data, size_t count)
			{ return 0; }
		size_t Read(void* data, size_t count)
			{ return 0; }
		size_t CopyFrom(VFile& src, size_t size)
			{ return 0; }

		size_t Seek(size_t offset, SeekMode from)
			{ return 0; }
		size_t Tell()
			{ return 0; }
	};

	VFilePtr VFile::NullObject()
	{
		static VFilePtr obj(new NullVFile);
		return obj;
	}
}
