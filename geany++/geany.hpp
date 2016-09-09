#pragma once


#include <geany++/common.hpp>
#include <geany++/document.hpp>
#include <geany++/pluginconfig.hpp>
#include <geany++/project.hpp>
#include <memory>
#include <string>
#include <type_traits>


namespace Geany
{
	struct ProxyPlugin;

	/**
	 * Provides access to all kinds of Geany's global data.
	 */
	extern GeanyData *data;


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

	/**
	 * An opaque structure passed to the IPlugin constructor.
	 *
	 * This is passed by the plugin loader to the base IPlugin class
	 * through any subclass constructors.
	 */
	struct PluginData;


	/**
	 * Interface for Geany plugins.
	 *
	 * This interface must be implemented by each plugin.
	 *
	 * @see IConfigurablePlugin
	 */
	class IPlugin
	{
	public:

		virtual ~IPlugin()
		{
		}

		/**
		 * Get the GeanyPlugin associated with this IPlugin.
		 *
		 * @return A reference to the related GeanyPlugin.
		 */
		GeanyPlugin &geany_plugin() const;

		/**
		 * Get the plugin's module/DLL filename.
		 *
		 * @return The plugin's module filename.
		 */
		const std::string &module_filename() const;

		/**
		 * Get the plugin's specification filename.
		 *
		 * @return The plugin's specification filename.
		 */
		const std::string &plugin_filename() const;

		/**
		 * Get the name of the plugin.
		 *
		 * @return The plugin's name.
		 */
		const std::string &name() const;

		/**
		 * Get the description of the plugin.
		 *
		 * @return The plugin's description.
		 */
		const std::string &description() const;

		/**
		 * Get the version string of this plugin.
		 *
		 * @return The plugin's version string.
		 */
		const std::string &version() const;

		/**
		 * Get the author(s) of this plugin.
		 *
		 * @return The plugin's author(s).
		 */
		const std::string &author() const;

		/**
		 * Get the help URI of this plugin.
		 *
		 * @return The plugin's help URI or an empty string if the
		 * plugin doesn't provide help documentation.
		 */
		const std::string &help_uri() const;

		/**
		 * Get the plugin's configuration file object.
		 *
		 * @return The PluginConfig object for this plugin.
		 */
		PluginConfig &config();

		/**
		 * Signal emitted when a new or existing document is emitted.
		 *
		 * @note This may not get fired if the plugin overrides the
		 * virtual document_open member function and doesn't chain back
		 * up to the base class from it.
		 *
		 * A reference to the document that was opened is passed as
		 * the only argument to the callbacks.
		 */
		sigc::signal<void, Document&> signal_document_open()
		{
			return signal_document_open_;
		}

		sigc::signal<void, Project&, Glib::KeyFile> signal_project_open()
		{
			return signal_project_open_;
		}

	protected:

		/**
		 * Base class constructor.
		 *
		 * Subclasses must chain-up to this constructor, passing the
		 * PluginInitData which they themselves must take as an
		 * argument.
		 *
		 * For example:
		 *
		 * @code
		 *   MyPlugin::MyPlugin(const PluginInitData &init_data)
		 *     : IPlugin(init_data), ...init more stuff...
		 *   {
		 *   }
		 * @endcode
		 *
		 * @param init_data An opaque reference to internal data used
		 * to initialize the base class.
		 */
		IPlugin(PluginData &init_data);

		/**
		 * Gets a Gtk::Widget panel to add to the plugin prefs dialog.
		 *
		 * @note This function will only be called if the plugin's
		 * spec file specifies `configurable = true`. If it does
		 * not, even if the plugin implements this function, it will
		 * never be called.
		 *
		 * @note If the plugin's spec file does specify
		 * `configurable = true` but the plugin doesn't implement this
		 * function, or returns `nullptr` like the default implementation,
		 * Geany will think the plugin can provide some kind of prefs
		 * GUI but when the user goes to actually open it, they will
		 * not see any GUI for the plugin.
		 *
		 * @param dialog The top-level plugin prefs dialog where the
		 * panel will be added at some undefined level of nesting. This
		 * is given so that the plugin can connect to its signals, in
		 * particular, Gtk::Dialog::response.
		 *
		 * @return The new Gtk::Widget containing the plugin's pref GUI
		 * and which will be added into the plugin prefs dialog. If
		 * `nullptr` is returned, no prefs GUI will be shown.
		 */
		virtual Gtk::Widget *configure(G_GNUC_UNUSED Gtk::Dialog *dialog)
		{
			return nullptr;
		}

		/**
		 * Gets called when a new or existing document is opened.
		 *
		 * The default implementation emits the signal_document_open()
		 * signal. If subclasses override this function, they should
		 * make sure to chain-up to this base class implementation
		 * if/when they want the signal emitted.
		 *
		 * To tell whether it was a new file that was opened, check
		 * Document::realpath() to see if it's empty. If it is, it's a
		 * new document that hasn't been saved to disk yet.
		 *
		 * @param doc The document that was opened.
		 */
		virtual void document_open(Document &doc)
		{
			signal_document_open_.emit(doc);
		}

		/**
		 * Gets called when a new or existing project is opened.
		 *
		 * The default implementation emits the signal_project_open()
		 * signal. If subclasses override this function, they should
		 * make sure to chain-up to this base class implementation
		 * if/when they want the signal emitted.
		 *
		 * @param proj The project that was opened.
		 * @param kf The config file for the project.
		 */
		virtual void project_open(Project &proj, Glib::KeyFile &kf)
		{
			signal_project_open_.emit(proj, kf);
		}

	private:
		PluginData &priv;
		sigc::signal<void, Document&> signal_document_open_;
		sigc::signal<void, Project&, Glib::KeyFile> signal_project_open_;

		friend GtkWidget *subplugin_configure(GeanyPlugin*, GtkDialog*, gpointer) noexcept G_GNUC_INTERNAL;
		friend void emit_document_open(ProxyPlugin *proxy, Document *doc) noexcept G_GNUC_INTERNAL;
		friend void on_project_open(GObject*, GKeyFile *kf, gpointer pdata) noexcept G_GNUC_INTERNAL;
	};

}


/**
 * Declares a function to be exported and with C linkage.
 *
 * Use it before the return type of functions that need to be visible
 * outside the plugin DLL and which should have C linkage (ie. `extern "C"`).
 */
#define GEANYCPP_EXPORT extern "C" G_MODULE_EXPORT


/**
 * Define the DLL plugin factory function using this macro.
 *
 * This macro defines the hook function that Geany++ will find in the
 * plugin's DLL (.so, .dll, etc) and call to get a new instance of a
 * Geany::IPlugin subclass.
 *
 * @note You should call this macro in the global namespace in one and
 * only one translation unit in the plugin, after the declaration of
 * the IPlugin subclass as shown above.
 *
 * @param T The Geany::IPlugin subclass type that implements the plugin.
 *
 * @see Geany::IPlugin
 */
#define GEANYCPP_DEFINE_PLUGIN(T)                                   \
    GEANYCPP_EXPORT                                                 \
    Geany::IPlugin *geanycpp_create_plugin(Geany::PluginData &x)    \
    {                                                               \
        static_assert(std::is_base_of<Geany::IPlugin, T>::value,    \
            "the type '" #T "' must derive from 'Geany::IPlugin'"); \
        return new T(x);                                            \
    }
