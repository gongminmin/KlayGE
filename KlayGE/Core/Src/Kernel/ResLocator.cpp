#include <KlayGE/KlayGE.hpp>

#include <KlayGE/PackedFile/PackedFile.hpp>
#include <KlayGE/DiskFile/DiskFile.hpp>

#include <KlayGE/ResLocator.hpp>

namespace KlayGE
{
	class NullResIdentifier : public ResIdentifier
	{
	public:
		VFilePtr Load()
			{ return VFile::NullObject(); }
	};

	ResIdentifierPtr ResIdentifier::NullObject()
	{
		static ResIdentifierPtr obj(new NullResIdentifier);
		return obj;
	}

	ResLocator& ResLocator::Instance()
	{
		static ResLocator resLocator;
		return resLocator;
	}

	ResLocator::ResLocator()
	{
		pathes_.push_back("");
	}

	void ResLocator::AddPath(std::string const & path)
	{
		if (path[path.length() - 1] != '/')
		{
			pathes_.push_back(path + '/');
		}
		else
		{
			pathes_.push_back(path);
		}
	}

	ResIdentifierPtr ResLocator::Locate(std::string const & name)
	{
		for (size_t i = 0; i < pathes_.size(); ++ i)
		{
			std::string const resName(pathes_[i] + name);

			boost::shared_ptr<DiskFile> diskFile(new DiskFile(resName, VFile::OM_Read));

			if (!diskFile->Fail())
			{
				return ResIdentifierPtr(new DiskFileResIdentifier(resName));
			}
			else
			{
				boost::shared_ptr<PackedFile> packedFile(new PackedFile(resName));

				if (!packedFile->Fail())
				{
					return ResIdentifierPtr(new PackedFileResIdentifier(resName));
				}
			}
		}

		return ResIdentifier::NullObject();
	}
}
