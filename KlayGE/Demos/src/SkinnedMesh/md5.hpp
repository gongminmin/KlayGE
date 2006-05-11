#ifndef _MD5_HPP
#define _MD5_HPP

#include "Model.hpp"
#include <string>

boost::shared_ptr<MD5SkinnedModel> LoadModel(const std::string& fileName);
boost::shared_ptr<KlayGE::KeyFramesType> LoadAnim(const std::string& fileName);

#endif		// _MD5_HPP
