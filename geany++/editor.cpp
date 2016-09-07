#include <geany++/editor.hpp>
#include <geany++/geany_p.hpp>

#ifdef HAVE_CONFIG_H
#include <geany++/config.h>
#endif

namespace Geany
{

	const std::string WORD_CHARS(GEANY_WORDCHARS);

	Document *Editor::document() const
	{
		return g_proxy->documents.lookup(m_ed->document);
	}

}
