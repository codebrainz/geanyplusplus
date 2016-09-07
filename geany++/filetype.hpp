#pragma once

#include <geany++/common.hpp>
#include <string>
#include <vector>

namespace Geany
{

	class Filetype
	{
	public:

		GeanyFiletype *get() const
		{
			return m_ft;
		}

		bool is_valid() const
		{
			return (m_ft != nullptr);
		}

		GeanyFiletypeID id() const
		{
			return m_ft->id;
		}

		std::string name() const
		{
			return m_ft->name;
		}

		Glib::ustring display_name() const
		{
			return ::filetypes_get_display_name(m_ft);
		}

		std::string title() const
		{
			return m_ft->title;
		}

		std::string extension() const
		{
			return m_ft->extension;
		}

		std::vector<std::string> patterns() const
		{
			std::vector<std::string> v;
			for (gchar **p = m_ft->pattern; *p; p++)
				v.emplace_back(*p);
			return v;
		}

		static Filetype *from_file(const Glib::ustring &filename);
		static Filetype *from_id(GeanyFiletypeID id);
		static Filetype *from_id(int idx);
		static Filetype *from_name(const std::string &name);
		static Filetype *from_geany_filetype(GeanyFiletype *ft);
		static const std::vector<Filetype*> &list();

	private:
		GeanyFiletype *m_ft;
		Filetype(GeanyFiletype *ft);
		friend class FiletypeManager;
	};

}
