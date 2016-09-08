#pragma once

#include <geany++/common.hpp>

namespace Geany
{

	class Project
	{
	public:

		GeanyProject *get() const
		{
			return m_proj;
		}

		bool is_valid() const
		{
			return (m_proj != nullptr);
		}

		std::string name() const
		{
			return m_proj->name;
		}

		std::string description() const
		{
			return m_proj->description;
		}

		std::string filename() const
		{
			return m_proj->file_name;
		}

		std::string base_path() const
		{
			return m_proj->base_path;
		}

		std::vector<std::string> file_patterns() const
		{
			std::vector<std::string> pats;
			for (gchar **p=m_proj->file_patterns; *p; p++)
				pats.emplace_back(*p);
			return pats;
		}

		void write_config()
		{
			::project_write_config();
		}

		sigc::signal<void, Glib::KeyFile&> signal_save()
		{
			return signal_save_;
		}

		sigc::signal<void> signal_close()
		{
			return signal_close_;
		}

		sigc::signal<void, Gtk::Notebook*> signal_dialog_open()
		{
			return signal_dialog_open_;
		}

		sigc::signal<void, Gtk::Notebook*> signal_dialog_confirmed()
		{
			return signal_dialog_confirmed_;
		}

		sigc::signal<void, Gtk::Notebook*> signal_dialog_close()
		{
			return signal_dialog_close_;
		}

	private:
		GeanyProject *m_proj;
		sigc::signal<void, Glib::KeyFile&> signal_save_;
		sigc::signal<void> signal_close_;
		sigc::signal<void, Gtk::Notebook*> signal_dialog_open_;
		sigc::signal<void, Gtk::Notebook*> signal_dialog_confirmed_;
		sigc::signal<void, Gtk::Notebook*> signal_dialog_close_;
		friend class ProxyPlugin;
		Project(GeanyProject *proj) : m_proj(proj) {}
	};

}
