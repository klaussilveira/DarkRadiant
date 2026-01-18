#pragma once

#include "imd5anim.h"
#include <map>

#include "MD5Anim.h"

namespace md5
{

class MD5AnimationCache :
	public IAnimationCache
{
private:
	// The path => anim mapping
	typedef std::map<std::string, MD5AnimPtr> AnimationMap;
	AnimationMap _animations;

public:
	// IAnimationCache implementation
	IMD5AnimPtr getAnim(const std::string& vfsPath);

	// RegisterableModule implementation
	std::string getName() const;
	StringSet getDependencies() const;
	void shutdownModule();
};
typedef std::shared_ptr<MD5AnimationCache> MD5AnimationCachePtr;

}
