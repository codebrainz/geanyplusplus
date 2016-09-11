#include <geany++/tagmanager.hpp>
#include <geany++/common.hpp>

namespace Geany
{

	namespace TagManager
	{

		Workspace &Workspace::instance()
		{
			static Workspace ws(nullptr);
			ws.m_ws = Geany::data->app->tm_workspace;
			return ws;
		}

	}

}
