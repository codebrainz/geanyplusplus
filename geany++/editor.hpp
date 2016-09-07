#pragma once

#include <geany++/common.hpp>
#include <geany++/scintilla.hpp>
#include <string>

namespace Geany
{

	extern const std::string WORD_CHARS;

	enum class Indicator
	{
		ERROR = GEANY_INDICATOR_ERROR,
		SEARCH = GEANY_INDICATOR_SEARCH,
	};

	enum class IndentType
	{
		SPACES,
		TABS,
		BOTH,
	};

	class IndentPrefs
	{
	public:
		unsigned int width;
		IndentType type;

	private:
		IndentPrefs(const GeanyIndentPrefs *prefs)
			: width(prefs->width),
			  type(static_cast<IndentType>(prefs->type))
		{
		}
		friend class Editor;
	};

	class Document;

	class Editor : public Scintilla
	{
	public:

		GeanyEditor *get() const
		{
			return m_ed;
		}

		bool is_valid() const
		{
			return (m_ed != nullptr);
		}

		Document *document() const;

		bool auto_indent() const
		{
			return m_ed->auto_indent;
		}

		bool line_breaking() const
		{
			return m_ed->line_breaking;
		}

		bool line_wrapping() const
		{
			return m_ed->line_wrapping;
		}

		float scroll_percent() const
		{
			return m_ed->scroll_percent;
		}

		std::string eol_chars() const
		{
			return ::editor_get_eol_char(m_ed);
		}

		IndentPrefs indent_prefs() const
		{
			const GeanyIndentPrefs *prefs = ::editor_get_indent_prefs(m_ed);
			return IndentPrefs(prefs);
		}

		IndentType indent_type() const
		{
			return indent_prefs().type;
		}

		void indent_type(IndentType type)
		{
			::editor_set_indent_type(m_ed, static_cast<GeanyIndentType>(type));
		}

		unsigned int indent_width() const
		{
			return indent_prefs().width;
		}

		void indent_width(unsigned int width)
		{
			::editor_set_indent_width(m_ed, width);
		}

		std::string word_at_position(int pos=-1,
			const std::string &word_chars=WORD_CHARS) const
		{
			const char *word =
				::editor_get_word_at_pos(m_ed, pos, word_chars.c_str());
			if (word)
				return word;
			return "";
		}

		bool goto_pos(unsigned int pos, bool mark=false)
		{
			return ::editor_goto_pos(m_ed, pos, mark);
		}

		void clear_indicator(Indicator indic)
		{
			::editor_indicator_clear(m_ed, static_cast<int>(indic));
		}

		void set_indicator(Indicator indic, unsigned int line)
		{
			::editor_indicator_set_on_line(m_ed, static_cast<int>(indic), line);
		}

		void set_indicator(Indicator indic, unsigned int start, unsigned int end)
		{
			if (end < start)
				std::swap(start, end);
			::editor_indicator_set_on_range(m_ed, static_cast<int>(indic), start, end);
		}

		static std::string default_eol_chars()
		{
			return ::editor_get_eol_char(nullptr);
		}

		static IndentPrefs default_indent_prefs()
		{
			const GeanyIndentPrefs *prefs = ::editor_get_indent_prefs(nullptr);
			return IndentPrefs(prefs);
		}

	private:
		GeanyEditor *m_ed;
		Editor(GeanyEditor *ed) : Scintilla(ed->sci), m_ed(ed) {}
		friend class Document;
	};

}
