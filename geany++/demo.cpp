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
		run_later();
	}

	void run_later()
	{
		Glib::signal_timeout().connect([this]() {
			this->signal_document_open().connect([](Geany::Document &doc) {
				g_debug("document '%s' opened", doc.filename().c_str());
			});
			Geany::Document::new_file("hello++.txt", nullptr,
				"Hello from the C++ demo plugin");
			return false;
		}, 250);
	}

};


GEANYCPP_DEFINE_PLUGIN(DemoPlugin);
