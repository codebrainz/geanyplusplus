#include <geany++/geany.hpp>

#ifdef HAVE_CONFIG_H
#include <geany++/config.h>
#endif

struct DemoPlugin final : public Geany::IPlugin
{
	Gtk::MenuItem item;

	DemoPlugin(Geany::PluginData &init_data)
		: Geany::IPlugin(init_data),
		  item(_("Demo C++"))
	{
		Geany::ui->tools_menu->append(item);
		item.show();
		item.signal_activate().connect([]() {
			Gtk::MessageDialog("Hello from the Demo C++ plugin").run();
		});
	}

};


GEANYCPP_DEFINE_PLUGIN(DemoPlugin);
