#include <geany++/iplugin.hpp>
#include <geany++/geany_p.hpp>

namespace Geany
{

	IPlugin::IPlugin(PluginData &init_data)
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

}
