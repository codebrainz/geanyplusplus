#include <geany++/document.hpp>
#include <geany++/geany_p.hpp>

#ifdef HAVE_CONFIG_H
#include <geany++/config.h>
#endif

namespace Geany
{

	Document::Document(GeanyDocument *doc)
		: m_doc(doc),
		  m_ed(DOC_VALID(doc) ? new Editor(doc->editor) : nullptr)
	{
	}

	Document *Document::current()
	{
		return g_proxy->documents.current();
	}

	Filetype *Document::filetype() const
	{
		return g_proxy->filetypes.lookup(m_doc->file_type);
	}

	const std::vector<Document*> &Document::list()
	{
		return g_proxy->documents.list();
	}

	Document *Document::new_file(const std::string &filename,
		Filetype *filetype, const std::string &init_text)
	{
		GeanyDocument *doc = ::document_new_file(
			!filename.empty() ? filename.c_str() : nullptr,
			filetype ? filetype->get() : nullptr,
			!init_text.empty() ? init_text.c_str() : nullptr);
		return g_proxy->documents.add(doc);
	}

	Document *Document::open(const std::string &filename, bool ro,
		Filetype *filetype, const std::string &encoding)
	{
		GeanyDocument *doc = ::document_open_file(
			filename.c_str(), ro,
			filetype ? filetype->get() : nullptr,
			!encoding.empty() ? encoding.c_str() : nullptr);
		return g_proxy->documents.add(doc);
	}

}
