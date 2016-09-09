#include <geany++/geany.hpp>

#ifdef HAVE_CONFIG_H
#include <geany++/config.h>
#endif

#include "gendialog.hpp"
#include "genprocessor.hpp"

struct GenPlugin final : public Geany::IPlugin
{
	Gtk::MenuItem item;

	GenPlugin(Geany::PluginData &init_data)
		: Geany::IPlugin(init_data),
		  item(_("Create C++ Plugin..."))
	{
		Geany::ui->tools_menu->append(item);
		item.set_tooltip_text(
			_("Generate the boilerplate for creating a Geany++ C++ plugin."));
		item.signal_activate().connect(sigc::mem_fun(*this, &GenPlugin::generate));
		item.show();
	}

	void generate()
	{
		GenDialog dialog;
		GenProcessorSettings settings;
		if (dialog.run(settings))
		{
			GenProcessor proc;
			proc.process(settings);
		}
	}

};


GEANYCPP_DEFINE_PLUGIN(GenPlugin);
