#include <Magick++.h>

#include "ImageHelper.h"

using namespace std;

bool ImageHelper::Crop(const std::string& dst, const std::string& src, size_t xpos, size_t ypos, size_t xposEnd, size_t yposEnd)
{
	if (xposEnd <= xpos || yposEnd <= ypos) {
		return false;
	}

	try {
		Magick::Image img(src);
		img.crop(Magick::Geometry(xposEnd - xpos, yposEnd - ypos, xpos, ypos));
		img.write(dst);
		return true;
	} catch (const exception& e) {
		// do nothing
	}

	return false;
}
