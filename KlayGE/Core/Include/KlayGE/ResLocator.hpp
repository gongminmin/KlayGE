#ifndef _RESLOCATOR_HPP
#define _RESLOCATOR_HPP

#include <vector>
#include <string>

namespace KlayGE
{
	class ResIdentifier
	{
	public:
		virtual ~ResIdentifier()
			{ }

		static ResIdentifierPtr NullObject();

		virtual VFilePtr Load() = 0;
	};

	class ResLocator
	{
	public:
		static ResLocator& Instance();

		void AddPath(const std::string& path);

		ResIdentifierPtr Locate(const std::string& name);

	private:
		ResLocator();

		std::vector<std::string> pathes_;
	};
}

#endif			// _RESLOCATOR_HPP
