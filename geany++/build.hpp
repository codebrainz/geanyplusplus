#pragma once

#include <geany++/common.hpp>

#if GEANY_API_VERSION >= 230 && 1
#define GEANY_API_HAVE_SYMMETRIC_BUILD_FUNCS 1
#endif

namespace Geany
{

	namespace Build
	{

		enum class Source
		{
			DEFAULT = GEANY_BCS_DEF,
			SYSTEM_FILETYPE = GEANY_BCS_FT,
			USER_FILETYPE = GEANY_BCS_HOME_FT,
			PREFERENCES = GEANY_BCS_PREF,
			PROJECT_FILETYPE = GEANY_BCS_PROJ_FT,
			PROJECT = GEANY_BCS_PROJ
		};

		class Group;

		class Command
		{
		public:
			bool is_valid() const
			{
				return (m_index != size_t(-1));
			}

			bool is_for_current_source() const
			{
				return m_current;
			}

			const Group &group() const
			{
				return m_group;
			}

			size_t index() const
			{
				return m_index;
			}

			void activate() const;
			Glib::ustring label() const;
			void label(const Glib::ustring &value);
			std::string command() const;
			void command(const std::string &value);
			std::string working_dir() const;
			void working_dir(const std::string &value);

		private:
			const Group &m_group;
			size_t m_index;
			Source m_source;
			bool m_current;
			friend class Group;

			Command(const Group &group, size_t index, Source source, bool current)
				: m_group(group),
				  m_index(index),
				  m_source(source),
				  m_current(current)
			{
			}
		};

		class Group
		{
		public:

			GeanyBuildGroup type() const
			{
				return m_type;
			}

			size_t num_commands() const
			{
				return ::build_get_group_count(static_cast<GeanyBuildGroup>(m_type));
			}

			Command nth_command(size_t n) const
			{
				if (n < num_commands())
					return Command(*this, n, Source::DEFAULT, true);
				return Command(*this, size_t(-1), Source::DEFAULT, true);
			}

			Command nth_command(size_t n, Source source) const
			{
				if (n < num_commands())
					return Command(*this, n, source, false);
				return Command(*this, size_t(-1), source, false);
			}

			std::vector<Command> list_commands() const
			{
				std::vector<Command> cmds;
				for (size_t i = 0, n = num_commands(); i < n; i++)
					cmds.push_back(nth_command(i));
				return cmds;
			}

			std::vector<Command> list_commands(Source source) const
			{
				std::vector<Command> cmds;
				for (size_t i = 0, n = num_commands(); i < n; i++)
					cmds.push_back(nth_command(i, source));
				return cmds;
			}

			static Group for_filetype()
			{
				return Group(GEANY_GBG_FT);
			}

			static Group for_non_filetype()
			{
				return Group(GEANY_GBG_NON_FT);
			}

			static Group for_execute()
			{
				return Group(GEANY_GBG_EXEC);
			}

		private:
			GeanyBuildGroup m_type;
			Group(GeanyBuildGroup type) : m_type(type) {}
		};

		inline void Command::activate() const
		{
			::build_activate_menu_item(m_group.type(), m_index);
		}

		inline Glib::ustring Command::label() const
		{
			if (m_current)
			{
				return ::build_get_current_menu_item(
					m_group.type(), m_index, GEANY_BC_LABEL);
			}
#ifdef GEANY_API_HAVE_SYMMETRIC_BUILD_FUNCS
			else
			{
				return ::build_get_menu_item(static_cast<GeanyBuildSource>(m_source),
					m_group.type(), m_index, GEANY_BC_LABEL);
			}
#else
			return "";
#endif
		}

		inline void Command::label(const Glib::ustring &value)
		{
			if (m_current)
			{
#ifdef GEANY_API_HAVE_SYMMETRIC_BUILD_FUNCS
					::build_set_current_menu_item(m_group.type(),
						m_index, GEANY_BC_LABEL, value.c_str());
#endif
			}
			else
			{
				::build_set_menu_item(static_cast<GeanyBuildSource>(m_source),
					m_group.type(), m_index, GEANY_BC_LABEL, value.c_str());
			}
		}

		inline std::string Command::command() const
		{
			if (m_current)
			{
				return ::build_get_current_menu_item(m_group.type(),
					m_index, GEANY_BC_COMMAND);
			}
#ifdef GEANY_API_HAVE_SYMMETRIC_BUILD_FUNCS
			else
			{
				return ::build_get_menu_item(static_cast<GeanyBuildSource>(m_source),
					m_group.type(), m_index, GEANY_BC_COMMAND);
			}
#else
			return "";
#endif
		}

		inline void Command::command(const std::string &value)
		{
			if (m_current)
			{
#ifdef GEANY_API_HAVE_SYMMETRIC_BUILD_FUNCS
					::build_set_current_menu_item(m_group.type(),
						m_index, GEANY_BC_COMMAND, value.c_str());
#endif
			}
			else
			{
				::build_set_menu_item(static_cast<GeanyBuildSource>(m_source),
					m_group.type(), m_index, GEANY_BC_COMMAND, value.c_str());
			}
		}

		inline std::string Command::working_dir() const
		{
			if (m_current)
			{
				return ::build_get_current_menu_item(m_group.type(),
					m_index, GEANY_BC_WORKING_DIR);
			}
#ifdef GEANY_API_HAVE_SYMMETRIC_BUILD_FUNCS
			else
			{
				return ::build_get_menu_item(static_cast<GeanyBuildSource>(m_source),
					m_group.type(), m_index, GEANY_BC_WORKING_DIR);
			}
#else
			return "";
#endif
		}

		inline void Command::working_dir(const std::string &value)
		{
			if (m_current)
			{
#ifdef GEANY_API_HAVE_SYMMETRIC_BUILD_FUNCS
					::build_set_current_menu_item(m_group.type(),
						m_index, GEANY_BC_WORKING_DIR, value.c_str());
#endif
			}
			else
			{
				::build_set_menu_item(static_cast<GeanyBuildSource>(m_source),
					m_group.type(), m_index, GEANY_BC_WORKING_DIR, value.c_str());
			}
		}

	}

}
