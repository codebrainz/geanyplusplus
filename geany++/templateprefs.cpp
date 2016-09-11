#include <geany++/templateprefs.hpp>
#include <geany++/geany.hpp>

namespace Geany
{

	namespace TemplatePrefs
	{
		Glib::ustring company()
		{
			return Geany::data->template_prefs->company;
		}

		Glib::ustring developer()
		{
			return Geany::data->template_prefs->developer;
		}

		Glib::ustring initials()
		{
			return Geany::data->template_prefs->initials;
		}

		Glib::ustring email()
		{
			return Geany::data->template_prefs->mail;
		}

		Glib::ustring initial_version()
		{
			return Geany::data->template_prefs->version;
		}
	}

}
