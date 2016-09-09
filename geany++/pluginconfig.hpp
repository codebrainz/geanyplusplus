#pragma once

#include <geany++/common.hpp>
#include <string>
#include <vector>

namespace Geany
{

	class PluginConfig
	{
	public:

		const std::string &filename() const
		{
			return m_fn;
		}

		Glib::KeyFile &keyfile()
		{
			return m_kf;
		}

		const std::string &current_group()
		{
			return m_group;
		}

		bool load();
		bool save();
		void save_later();

		void begin_group(const std::string &group);
		void end_group();

		template< class T >
		T get(const std::string &key);

		template< class T >
		void set(const std::string &key, T value);

	private:
		std::string m_fn;
		std::string m_group;
		Glib::KeyFile m_kf;
		std::vector<std::string> m_group_stack;
		PluginConfig(const std::string &plugin_filename);
		PluginConfig(const PluginConfig&);
		PluginConfig &operator=(const PluginConfig&);
		~PluginConfig();
		void update_current_group();
		friend class PluginData;
	};

	template<>
	inline std::string PluginConfig::get<std::string>(const std::string &key)
	{
		return m_kf.get_string(m_group, key);
	}

	template<>
	inline std::vector<std::string> PluginConfig::get<std::vector<std::string>>(const std::string &key)
	{
		return m_kf.get_string_list(m_group, key);
	}

	template<>
	inline bool PluginConfig::get<bool>(const std::string &key)
	{
		return m_kf.get_boolean(m_group, key);
	}

	template<>
	inline int PluginConfig::get<int> (const std::string &key)
	{
		return m_kf.get_integer(m_group, key);
	}

	template<>
	inline int64_t PluginConfig::get<int64_t>(const std::string &key)
	{
		return m_kf.get_int64(m_group, key);
	}

	template<>
	inline uint64_t PluginConfig::get<uint64_t>(const std::string &key)
	{
		return m_kf.get_uint64(m_group, key);
	}

	template<>
	inline double PluginConfig::get<double>(const std::string &key)
	{
		return m_kf.get_double(m_group, key);
	}

	template<>
	inline void PluginConfig::set<const std::string&>(const std::string &key, const std::string &value)
	{
		m_kf.set_string(m_group, key, value);
	}

	template<>
	inline void PluginConfig::set<const std::vector<std::string>&>(const std::string &key, const std::vector<std::string> &value)
	{
		m_kf.set_string_list(m_group, key, value);
	}

	template<>
	inline void PluginConfig::set<bool>(const std::string &key, bool value)
	{
		m_kf.set_boolean(m_group, key, value);
	}

	template<>
	inline void PluginConfig::set<int>(const std::string &key, int value)
	{
		m_kf.set_integer(m_group, key, value);
	}

	template<>
	inline void PluginConfig::set<int64_t>(const std::string &key, int64_t value)
	{
		m_kf.set_int64(m_group, key, value);
	}

	template<>
	inline void PluginConfig::set<uint64_t>(const std::string &key, uint64_t value)
	{
		m_kf.set_uint64(m_group, key, value);
	}

	template<>
	inline void PluginConfig::set<double>(const std::string &key, double value)
	{
		m_kf.set_double(m_group, key, value);
	}

}
