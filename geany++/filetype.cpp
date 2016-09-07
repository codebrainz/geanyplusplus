#include <geany++/filetype.hpp>
#include <geany++/geany_p.hpp>

#ifdef HAVE_CONFIG_H
#include <geany++/config.h>
#endif

namespace Geany
{
	std::vector<Filetype*> g_filetypes;
	std::unordered_map<GeanyFiletype*, Filetype*> g_filetype_map;

	Filetype::Filetype(GeanyFiletype *ft)
		: m_ft(ft)
	{
	}

	Filetype *Filetype::from_file(const Glib::ustring &filename)
	{
		GeanyFiletype *ft = ::filetypes_detect_from_file(filename.c_str());
		return g_proxy->filetypes.lookup(ft);
	}

	Filetype *Filetype::from_id(GeanyFiletypeID id)
	{
		return from_id(static_cast<int>(id));
	}

	Filetype *Filetype::from_id(int idx)
	{
		GeanyFiletype *ft = ::filetypes_index(idx);
		return g_proxy->filetypes.lookup(ft);
	}

	Filetype *Filetype::from_name(const std::string &name)
	{
		GeanyFiletype *ft = ::filetypes_lookup_by_name(name.c_str());
		return g_proxy->filetypes.lookup(ft);
	}

	Filetype *Filetype::from_geany_filetype(GeanyFiletype *ft)
	{
		return g_proxy->filetypes.lookup(ft);
	}

	const std::vector<Filetype*> &Filetype::list()
	{
		return g_proxy->filetypes.list();
	}

}
