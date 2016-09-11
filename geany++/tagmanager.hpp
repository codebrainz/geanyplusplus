#pragma once

#include <geany++/common.hpp>

namespace Geany
{

	namespace TagManager
	{
#if 0
		// TODO: this doesn't make much sense here
		std::string realpath(const std::string &fn)
		{
			if (gchar *rp = ::tm_get_real_path(fn.c_str()))
			{
				std::string s(rp);
				g_free(rp);
				return s;
			}
			return "";
		}
#endif

		class Tag;

		class SourceFile
		{
		public:
			SourceFile(const std::string &fn, const std::string &name=std::string())
				: m_file(::tm_source_file_new(fn.c_str(), name.empty() ? nullptr : name.c_str())),
				  m_owned(true)
			{
			}

			~SourceFile()
			{
				if (m_owned)
					::tm_source_file_free(m_file);
			}

			bool is_valid() const
			{
				return (m_file != nullptr);
			}

			TMSourceFile *get() const
			{
				return m_file;
			}

			std::string filename() const
			{
				return m_file->file_name;
			}

			std::string short_name() const
			{
				return m_file->short_name;
			}

			size_t num_tags() const
			{
				return m_file->tags_array->len;
			}

			Tag nth_tag(size_t n) const;

		private:
			TMSourceFile *m_file;
			bool m_owned;
			friend class Tag;
			friend class Workspace;
			SourceFile(const SourceFile&);
			SourceFile& operator=(const SourceFile&);

			SourceFile(TMSourceFile *file)
				: m_file(file),
				  m_owned(false)
			{
			}
		};

		class Workspace
		{
		public:
			bool is_valid() const
			{
				return (m_ws != nullptr);
			}

			const TMWorkspace *get() const
			{
				return m_ws;
			}

			size_t num_global_tags() const
			{
				return m_ws->global_tags->len;
			}

			Tag nth_global_tag(size_t n) const;

			size_t num_source_files() const
			{
				return m_ws->source_files->len;
			}

			SourceFile nth_source_file(size_t n) const
			{
				if (n < num_source_files())
					return SourceFile(static_cast<TMSourceFile*>(m_ws->source_files->pdata[n]));
				return SourceFile(nullptr);
			}

			size_t num_tags() const
			{
				return m_ws->tags_array->len;
			}

			Tag nth_tag(size_t n) const;

			static Workspace &instance();

		private:
			const TMWorkspace *m_ws;
			Workspace(const TMWorkspace *ws) : m_ws(ws) {}
		};

		class Tag
		{
		public:
			bool is_valid() const
			{
				return (m_tag != nullptr);
			}

			TMTag *get() const
			{
				return m_tag;
			}

			char access() const
			{
				return m_tag->access;
			}

			std::string arglist() const
			{
				return m_tag->arglist;
			}

			SourceFile file() const
			{
				return SourceFile(m_tag->file);
			}

			char impl() const
			{
				return m_tag->impl;
			}

			std::string inheritance() const
			{
				return m_tag->inheritance;
			}

			unsigned long line() const
			{
				return m_tag->line;
			}

			bool local() const
			{
				return m_tag->local;
			}

			std::string name() const
			{
				return m_tag->name;
			}

			std::string scope() const
			{
				return m_tag->scope;
			}

			// FIXME: returns TMTagType but it is not in Geany API docs
			int type() const
			{
				return m_tag->type;
			}

			std::string var_type() const
			{
				return m_tag->var_type;
			}

		private:
			TMTag *m_tag;
			Tag(TMTag *tag) : m_tag(tag) {}
			friend class SourceFile;
			friend class Workspace;
		};

		inline Tag SourceFile::nth_tag(size_t n) const
		{
			if (n < num_tags())
				return Tag(static_cast<TMTag*>(m_file->tags_array->pdata[n]));
			return Tag(nullptr);
		}

		inline Tag Workspace::nth_global_tag(size_t n) const
		{
			if (n < num_global_tags())
				return Tag(static_cast<TMTag*>(m_ws->global_tags->pdata[n]));
			return Tag(nullptr);
		}

		inline Tag Workspace::nth_tag(size_t n) const
		{
			if (n < num_tags())
				return Tag(static_cast<TMTag*>(m_ws->tags_array->pdata[n]));
			return Tag(nullptr);
		}

	}

}
