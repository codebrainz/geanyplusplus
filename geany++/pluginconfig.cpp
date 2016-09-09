#include <geany++/pluginconfig.hpp>
#include <geany++/geany.hpp>
#include <geany++/utils.hpp>

#define DEFAULT_GROUP "settings"

namespace Geany
{

	static inline std::string make_cfg_name(const std::string &plugin_filename)
	{
		return Glib::build_filename(Geany::data->app->configdir,
			"plugins", basename_without_extension(plugin_filename),
			"config.ini");
	}

	PluginConfig::PluginConfig(const std::string &plugin_filename)
		: m_fn(make_cfg_name(plugin_filename))
	{
		load();
		m_group_stack.emplace_back(DEFAULT_GROUP);
		update_current_group();
	}

	PluginConfig::~PluginConfig()
	{
		try
		{
			if (!m_kf.get_groups().empty())
				save();
		}
		catch (...) {}
	}

	bool PluginConfig::load()
	{
		try
		{
			return m_kf.load_from_file(m_fn,
				Glib::KEY_FILE_KEEP_COMMENTS | Glib::KEY_FILE_KEEP_TRANSLATIONS);
		}
		catch (Glib::FileError&)
		{
			return false;
		}
		catch (Glib::KeyFileError&)
		{
			return false;
		}
	}

	bool PluginConfig::save()
	{
		return Glib::KeyFile::save_to_file(m_fn);
	}

	void PluginConfig::save_later()
	{
		Glib::signal_idle().connect([]() { save(); });
	}

	void PluginConfig::begin_group(const std::string &group)
	{
		m_group_stack.emplace_back(group);
		update_current_group();
	}

	void PluginConfig::end_group()
	{
		if (!m_group.empty())
		{
			m_group_stack.pop_back();
			update_current_group();
		}
	}

	void PluginConfig::update_current_group()
	{
		if (!m_group_stack.empty())
		{
			m_group.clear();
			for (size_t i = 0, last = m_group_stack.size() - 1; i < m_group_stack.size(); i++)
			{
				m_group += m_group_stack[i];
				if (i != last)
					m_group += "/";
			}
		}
		else
		{
			m_group = DEFAULT_GROUP;
		}
	}

}
