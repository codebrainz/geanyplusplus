#pragma once

#include <geany++/common.hpp>
#include <geany++/editor.hpp>
#include <geany++/filetype.hpp>
#include <memory>
#include <string>
#include <vector>

namespace Geany
{

	class Document
	{
	public:

		GeanyDocument *get() const
		{
			return m_doc;
		}

		bool is_valid() const
		{
			return DOC_VALID(m_doc);
		}

		unsigned int id() const
		{
			return m_doc->id;
		}

		Glib::ustring filename() const
		{
			return m_doc->file_name;
		}

		std::string realpath() const
		{
			return m_doc->real_path ? m_doc->real_path : "";
		}

		Glib::ustring display_name(int length=-1) const
		{
			return ::document_get_basename_for_display(m_doc, length);
		}

		Filetype *filetype() const;

		void filetype(Filetype *ft)
		{
			if (ft)
				::document_set_filetype(m_doc, ft->get());
		}

		Editor *editor() const
		{
			return m_ed.get();
		}

		Glib::ustring encoding() const
		{
			return m_doc->encoding;
		}

		void encoding(const Glib::ustring &enc)
		{
			::document_set_encoding(m_doc, enc.c_str());
		}

		bool is_readonly() const
		{
			return m_doc->readonly;
		}

		bool has_changed() const
		{
			return m_doc->changed;
		}

		void has_changed(bool changed=true)
		{
			::document_set_text_changed(m_doc, changed);
		}

		bool has_bom() const
		{
			return m_doc->has_bom;
		}

		bool has_tags() const
		{
			return m_doc->has_tags;
		}

		bool close()
		{
			return ::document_close(m_doc);
		}

		bool save(bool force=false)
		{
			return ::document_save_file(m_doc, force);
		}

		bool save_as(const Glib::ustring &new_filename)
		{
			return ::document_save_file_as(m_doc, new_filename.c_str());
		}

		bool reload(const Glib::ustring &encoding="")
		{
			return ::document_reload_force(m_doc,
				encoding.empty() ? nullptr : encoding.c_str());
		}

		sigc::signal<void> &signal_activate() { return signal_activate_; }
		sigc::signal<void> &signal_before_save() { return signal_before_save_; }
		sigc::signal<void> &signal_save() { return signal_save_; }
		sigc::signal<void> &signal_reload() { return signal_reload_; }
		sigc::signal<void> &signal_close() { return signal_close_; }
		sigc::signal<void, Filetype*> &signal_filetype_set() { return signal_filetype_set_; }

		static Document *current();
		static const std::vector<Document*> &list();

		static Document *new_file(const std::string &filename=std::string(),
			Filetype *filetype=nullptr, const std::string &init_text=std::string());

		static Document *open(const std::string &filename, bool ro=false,
			Filetype *filetype=nullptr, const std::string &encoding=std::string());

	private:
		GeanyDocument *m_doc;
		std::unique_ptr<Editor> m_ed;
		sigc::signal<void> signal_activate_;
		sigc::signal<void> signal_before_save_;
		sigc::signal<void> signal_save_;
		sigc::signal<void> signal_reload_;
		sigc::signal<void> signal_close_;
		sigc::signal<void, Filetype*> signal_filetype_set_;

		Document(GeanyDocument *doc);

		friend class DocumentManager;
	};

}
