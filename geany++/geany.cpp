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

} // namespace Geany
