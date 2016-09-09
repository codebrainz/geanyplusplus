#pragma once

#include <geany++/common.hpp>
#include <string>

namespace Geany
{

	static inline std::string basename_without_extension(const std::string &fn)
	{
		auto bn = Glib::path_get_basename(fn);
		auto ext_pos = bn.rfind('.');
		if (ext_pos != bn.npos)
			bn.resize(ext_pos);
		return bn;
	}

	static inline std::string replace_extension(const std::string &fn, const std::string &ext)
	{
		auto dn = Glib::path_get_dirname(fn);
		auto bn = basename_without_extension(fn) + ext;
		return Glib::build_filename(dn, bn);
	}

}
