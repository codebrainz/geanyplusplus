#pragma once

#include <geany++/common.hpp>

namespace Geany
{

	/**
	 * Holds pointers to various UI widget wrappers.
	 *
	 * @see Geany's GeanyMainWidgets structure for descriptions of what
	 * each of these widgets are.
	 */
	class UI
	{
	public:
		Gtk::Menu *editor_menu;          //!< The editor's right-click context menu.
		Gtk::Notebook *msgwin_notebook;  //!< The notebook in the message window.
		Gtk::Notebook *notebook;         //!< The main document notebook.
		Gtk::ProgressBar *progressbar;   //!< The progress bar that at the right of the statusbar.
		Gtk::Menu *project_menu;         //!< The Project submenu in the main menu.
		Gtk::Notebook *sidebar_notebook; //!< The notebook in the sidebar
		Gtk::Toolbar *toolbar;           //!< The main toolbar
		Gtk::Menu *tools_menu;           //!< The Tools submenu in the main menu.
		Gtk::Window *window;             //!< Geany's main top-level window.

	private:
		UI(GeanyMainWidgets *w);
		~UI();
		friend gboolean proxy_init(GeanyPlugin*, gpointer) noexcept;
		friend void proxy_cleanup(GeanyPlugin*, gpointer) noexcept;
	};

	/**
	 * Provides wrappers of common UI widgets.
	 */
	extern UI *ui;

}
