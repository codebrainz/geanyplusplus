#include <geany++/geany_p.hpp>
#include <geany++/utils.hpp>

#ifdef HAVE_CONFIG_H
#include <geany++/config.h>
#endif

namespace Geany
{
	//
	// Global variables
	//

	GeanyData *data = nullptr;
	UI *ui = nullptr;
	ProxyPlugin *g_proxy = nullptr;


	//
	// Helper functions
	//

	void init()
	{
		Glib::init();
		Gtk::Main::init_gtkmm_internals();
	}

	template< class T >
	static T* wrap(GtkWidget *wid, bool take_copy=true)
	{
		return dynamic_cast<T*>(Glib::wrap(wid, take_copy));
	}

	//
	// UI implementation
	//

	UI::UI(GeanyMainWidgets *w)
		: editor_menu(wrap<Gtk::Menu>(w->editor_menu)),
		  msgwin_notebook(wrap<Gtk::Notebook>(w->message_window_notebook)),
		  notebook(wrap<Gtk::Notebook>(w->notebook)),
		  progressbar(wrap<Gtk::ProgressBar>(w->progressbar)),
		  project_menu(wrap<Gtk::Menu>(w->project_menu)),
		  sidebar_notebook(wrap<Gtk::Notebook>(w->sidebar_notebook)),
		  toolbar(wrap<Gtk::Toolbar>(w->toolbar)),
		  tools_menu(wrap<Gtk::Menu>(w->tools_menu)),
		  window(wrap<Gtk::Window>(w->window))
	{
	}

	UI::~UI()
	{
		/* Causes bad stuff to happen
		delete editor_menu;
		delete msgwin_notebook;
		delete notebook;
		delete progressbar;
		delete project_menu;
		delete sidebar_notebook;
		delete toolbar;
		delete tools_menu;
		delete window;
		*/
	}

	//
	// PluginModule implementation
	//

	PluginModule::PluginModule(const std::string &spec_filename)
		: filename(replace_extension(spec_filename, MODULE_EXTENSION)),
		  module(filename/*Glib::MODULE_BIND_LOCAL*/),
		  factory_func(nullptr)
	{
		void *symbol = nullptr;
		if (module)
		{
			if (module.get_symbol(MODULE_SYMBOL, symbol))
				factory_func = (PluginCreateFunc) symbol;
		}
	}

	bool PluginModule::loaded() const
	{
		return module && factory_func;
	}

	IPlugin *PluginModule::create_plugin(const PluginData &init_data)
	{
		if (loaded())
			return factory_func(init_data);
		return nullptr;
	}

	bool PluginModule::probe(const std::string &fn, bool check_spec)
	{
		if (check_spec)
		{
			if (!PluginSpecFile::probe(replace_extension(fn, SPEC_EXTENSION), false))
				return false;
		}

		try
		{
			Glib::Module module(fn, Glib::MODULE_BIND_LOCAL);
			if (module)
			{
				void *symbol = nullptr;
				return module.get_symbol(MODULE_SYMBOL, symbol);
			}
		}
		catch (...)
		{
		}

		return false;
	}


	//
	// PluginSpecFile implementation
	//

	PluginSpecFile::PluginSpecFile(const std::string &filename)
		: filename(filename),
		  configurable(false)
	{
		Glib::KeyFile spec;
		spec.load_from_file(filename, Glib::KEY_FILE_KEEP_TRANSLATIONS);

		name = spec.get_locale_string(GROUP_NAME, "name");

		try {
			description = spec.get_locale_string(GROUP_NAME, "description");
		} catch (Glib::KeyFileError&) {
			description.clear();
		}

		try {
			version = spec.get_string(GROUP_NAME, "version");
		} catch (Glib::KeyFileError&) {
			version.clear();
		}

		try {
			author = spec.get_string(GROUP_NAME, "author");
		} catch (Glib::KeyFileError&) {
			author.clear();
		}

		try {
			help_uri = spec.get_string(GROUP_NAME, "help_uri");
		} catch (Glib::KeyFileError&) {
			help_uri.clear();
		}

		try {
			configurable = spec.get_boolean(GROUP_NAME, "configurable");
		} catch (Glib::KeyFileError&) {
			configurable = false;
		}
	}

	bool PluginSpecFile::provides_help() const
	{
		return !help_uri.empty();
	}

	bool PluginSpecFile::probe(const std::string &fn, bool check_module)
	{
		if (check_module)
		{
			if (!PluginModule::probe(replace_extension(fn, MODULE_EXTENSION), false))
				return false;
		}

		try
		{
			Glib::KeyFile spec;
			if (spec.load_from_file(fn, Glib::KEY_FILE_KEEP_TRANSLATIONS) &&
				spec.has_group(GROUP_NAME) &&
				spec.has_key(GROUP_NAME, "name"))
			{
				auto str = spec.get_locale_string(GROUP_NAME, "name");
				return !str.empty();
			}
		}
		catch (...)
		{
		}

		return false;
	}


	//
	// PluginData implementation
	//

	PluginData::PluginData(ProxyPlugin &proxy,
		GeanyPlugin *gplugin, const std::string &spec_filename)
		: proxy(proxy),
		  gplugin(gplugin),
		  spec(spec_filename),
		  config(spec_filename)
	{
	}

	bool PluginData::is_initialized() const
	{
		return (module && plugin);
	}

	void PluginData::init()
	{
		module.reset(new PluginModule(spec.filename));
		plugin.reset(module->create_plugin(*this));
	}

	void PluginData::cleanup()
	{
		plugin.reset(nullptr);
		module.reset(nullptr);
	}

	PluginData *PluginData::from_data(gpointer pdata)
	{
		return static_cast<PluginData*>(pdata);
	}


	//
	// IPlugin implementation
	//

	IPlugin::IPlugin(const PluginData &init_data)
		: priv(init_data)
	{
	}

	GeanyPlugin &IPlugin::geany_plugin() const
	{
		return *(priv.gplugin);
	}

	const std::string &IPlugin::module_filename() const
	{
		return priv.module->filename;
	}

	const std::string &IPlugin::plugin_filename() const
	{
		return priv.spec.filename;
	}

	const std::string &IPlugin::name() const
	{
		return priv.spec.name;
	}

	const std::string &IPlugin::description() const
	{
		return priv.spec.description;
	}

	const std::string &IPlugin::version() const
	{
		return priv.spec.version;
	}

	const std::string &IPlugin::author() const
	{
		return priv.spec.author;
	}

	const std::string &IPlugin::help_uri() const
	{
		return priv.spec.help_uri;
	}

	PluginConfig &IPlugin::config()
	{
		return priv.config;
	}

} // namespace Geany
