#include <geany++/ui.hpp>

namespace Geany
{

	template< class T >
	static T* wrap(GtkWidget *wid, bool take_copy=true)
	{
		return dynamic_cast<T*>(Glib::wrap(wid, take_copy));
	}

	UI::UI(GeanyMainWidgets *w)
		: editor_menu(wrap<Gtk::Menu>(w->editor_menu)),
		  msgwin_notebook(wrap<Gtk::Notebook>(w->message_window_notebook)),
		  notebook(wrap<Gtk::Notebook>(w->notebook)),
		  progressbar(wrap<Gtk::ProgressBar>(w->progressbar)),
		  project_menu(wrap<Gtk::Menu>(w->project_menu)),
		  sidebar_notebook(wrap<Gtk::Notebook>(w->sidebar_notebook)),
		  toolbar(wrap<Gtk::Toolbar>(w->toolbar)),
		  tools_menu(wrap<Gtk::Menu>(w->tools_menu)),
		  window(wrap<Gtk::Window>(w->window))
	{
	}

	UI::~UI()
	{
		/* Causes bad stuff to happen
		delete editor_menu;
		delete msgwin_notebook;
		delete notebook;
		delete progressbar;
		delete project_menu;
		delete sidebar_notebook;
		delete toolbar;
		delete tools_menu;
		delete window;
		*/
	}

}
