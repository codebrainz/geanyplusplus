#include <geany++/project.hpp>
#include <geany++/geany_p.hpp>

namespace Geany
{

#ifdef GEANY_API_HAVE_PROJECT_OPEN
	Project *Project::open(const std::string &fn)
	{
		if (::project_open_file(fn.c_str()))
			return g_proxy->project.get();
		return nullptr;
	}
#endif

}
