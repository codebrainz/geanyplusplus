#include <geany++/geany_p.hpp>

#ifdef HAVE_CONFIG_H
#include <geany++/config.h>
#endif

#include <memory>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <cstdlib>

//#if GEANY_API_VERSION >= 229
//#define PROX_MATCH GEANY_PROXY_MATCH
//#define PROX_IGNORE GEANY_PROXY_IGNORE
//#define PROX_RELATED GEANY_PROXY_RELATED
//#else
#define PROX_MATCH PROXY_MATCHED
#define PROX_IGNORE PROXY_IGNORED
#define PROX_RELATED (PROXY_MATCHED|PROXY_NOLOAD)
//#endif


// use these around C++ code which might throw an exception that would
// otherwise go uncaught into plain C code.
#define CXX_BLOCK_BEGIN try {
#define CXX_BLOCK_END                                                    \
	} catch (std::exception &exc) {                                      \
		g_critical(_("unhandled C++ exception caught: %s"), exc.what()); \
		abort(); \
	} catch (...) {                                                      \
		g_critical(_("unhandled unknown C++ exception caught"));         \
		abort(); \
	}


namespace Geany
{

	//
	// Sub-plugin callbacks
	//

	static gboolean subplugin_init(GeanyPlugin *gplugin, gpointer pdata) noexcept
	{
		CXX_BLOCK_BEGIN
		{
			auto data = PluginData::from_data(pdata);
			data->init();
			if (data->is_initialized())
			{
				data->proxy.plugins.add(gplugin, data->plugin.get());
				return TRUE;
			}
		}
		CXX_BLOCK_END
		return FALSE;
	}

	static void subplugin_cleanup(GeanyPlugin*, gpointer pdata) noexcept
	{
		CXX_BLOCK_BEGIN
		{
			auto data = PluginData::from_data(pdata);
			data->cleanup();
		}
		CXX_BLOCK_END
	}

	static void subplugin_help(GeanyPlugin*, gpointer pdata) noexcept
	{
		CXX_BLOCK_BEGIN
		{
			auto data = PluginData::from_data(pdata);
			utils_open_browser(data->spec.help_uri.c_str());
		}
		CXX_BLOCK_END
	}

	GtkWidget *subplugin_configure(GeanyPlugin*, GtkDialog *gdialog, gpointer pdata) noexcept
	{
		GtkWidget *prefs_panel = nullptr;
		CXX_BLOCK_BEGIN
		{
			if (auto dialog = dynamic_cast<Gtk::Dialog*>(Glib::wrap(gdialog, true)))
			{
				auto data = PluginData::from_data(pdata);
				if (auto widget = data->plugin->configure(dialog))
				{
					prefs_panel = GTK_WIDGET(g_object_ref(widget->gobj()));
					delete widget;
				}
			}
		}
		CXX_BLOCK_END
		return prefs_panel;
	}


	//
	// Proxy plugin callbacks
	//

	static gint proxy_probe(GeanyPlugin*, const gchar *filename, gpointer) noexcept
	{
		CXX_BLOCK_BEGIN
		{
			if (Glib::str_has_suffix(filename, SPEC_EXTENSION))
			{
				if (PluginSpecFile::probe(filename))
					return PROX_MATCH;
			}
			else if (Glib::str_has_suffix(filename, MODULE_EXTENSION))
			{
				if (PluginModule::probe(filename))
					return PROX_RELATED;
			}
		}
		CXX_BLOCK_END
		return PROX_IGNORE;
	}

	static gpointer proxy_load(GeanyPlugin*, GeanyPlugin *plugin,
		const gchar *filename, gpointer pdata) noexcept
	{
		CXX_BLOCK_BEGIN
		{
			auto proxy = static_cast<ProxyPlugin*>(pdata);
			g_return_val_if_fail(proxy, nullptr);
			std::unique_ptr<PluginData> data(new PluginData(*proxy, plugin, filename));

			// store pointers to the plugin info
			plugin->info->name = data->spec.name.c_str();
			plugin->info->description = data->spec.description.c_str();
			plugin->info->version = data->spec.version.c_str();
			plugin->info->author = data->spec.author.c_str();

			// store pointers to the wrapper functions
			plugin->funcs->init = subplugin_init;
			plugin->funcs->cleanup = subplugin_cleanup;
			if (data->spec.provides_help())
				plugin->funcs->help = subplugin_help;
			if (data->spec.configurable)
				plugin->funcs->configure = subplugin_configure;

			// register the subplugin with Geany
			if (GEANY_PLUGIN_REGISTER_FULL(plugin, MIN_API, data.get(), nullptr))
				return data.release();
		}
		CXX_BLOCK_END
		return NULL;
	}

	static void proxy_unload(GeanyPlugin*, GeanyPlugin *gplugin, gpointer load_data, gpointer pdata) noexcept
	{
		CXX_BLOCK_BEGIN
		{
			auto proxy = static_cast<ProxyPlugin*>(pdata);
			proxy->plugins.remove(gplugin);
			delete PluginData::from_data(load_data);
		}
		CXX_BLOCK_END
	}

	void emit_document_open(ProxyPlugin *proxy, Document *doc) noexcept
	{
		CXX_BLOCK_BEGIN
		{
			g_return_if_fail(doc && doc->is_valid());
			for (auto plugin : proxy->plugins.list_plugins())
				plugin->document_open(*doc);
		}
		CXX_BLOCK_END
	}

	static void on_document_activate(GObject*, GeanyDocument *doc, gpointer pdata) noexcept
	{
		CXX_BLOCK_BEGIN
		{
			if (auto document = ProxyPlugin::from_data(pdata)->documents.lookup(doc))
				document->signal_activate().emit();
		}
		CXX_BLOCK_END
	}

	static void on_document_before_save(GObject*, GeanyDocument *doc, gpointer pdata) noexcept
	{
		CXX_BLOCK_BEGIN
		{
			if (auto document = ProxyPlugin::from_data(pdata)->documents.lookup(doc))
				document->signal_before_save().emit();
		}
		CXX_BLOCK_END
	}

	static void on_document_close(GObject*, GeanyDocument *doc, gpointer pdata) noexcept
	{
		CXX_BLOCK_BEGIN
		{
			auto proxy = ProxyPlugin::from_data(pdata);
			if (auto document = proxy->documents.lookup(doc))
				document->signal_close().emit();
			proxy->documents.remove(doc);
		}
		CXX_BLOCK_END
	}

	static void on_document_filetype_set(GObject*, GeanyDocument *doc,
		GeanyFiletype *ft_old, gpointer pdata) noexcept
	{
		CXX_BLOCK_BEGIN
		{
			auto proxy = ProxyPlugin::from_data(pdata);
			// filetype-set may be emitted before new/open
			if (auto document = proxy->documents.add(doc))
			{
				auto ft = proxy->filetypes.lookup(ft_old);
				document->signal_filetype_set().emit(ft);
			}
		}
		CXX_BLOCK_END
	}

	static void on_document_new(GObject*, GeanyDocument *doc, gpointer pdata) noexcept
	{
		CXX_BLOCK_BEGIN
		{
			auto proxy = ProxyPlugin::from_data(pdata);
			auto document = proxy->documents.add(doc);
			emit_document_open(proxy, document);
		}
		CXX_BLOCK_END
	}

	static void on_document_open(GObject*, GeanyDocument *doc, gpointer pdata) noexcept
	{
		CXX_BLOCK_BEGIN
		{
			auto proxy = ProxyPlugin::from_data(pdata);
			auto document = proxy->documents.add(doc);
			emit_document_open(proxy, document);
		}
		CXX_BLOCK_END
	}

	static void on_document_reload(GObject*, GeanyDocument *doc, gpointer pdata) noexcept
	{
		CXX_BLOCK_BEGIN
		{
			if (auto document = ProxyPlugin::from_data(pdata)->documents.lookup(doc))
				document->signal_reload().emit();
		}
		CXX_BLOCK_END
	}

	static void on_document_save(GObject*, GeanyDocument *doc, gpointer pdata) noexcept
	{
		CXX_BLOCK_BEGIN
		{
			if (auto document = ProxyPlugin::from_data(pdata)->documents.lookup(doc))
				document->signal_save().emit();
		}
		CXX_BLOCK_END
	}

	static gboolean on_editor_notify(GObject*, GeanyEditor *editor,
		SCNotification *nt, gpointer pdata) noexcept
	{
		g_return_val_if_fail(nt, FALSE);
		CXX_BLOCK_BEGIN
		{
			Scintilla *sci = Scintilla::from_widget(editor->sci);
			if (!sci)
				return FALSE;
			switch (nt->nmhdr.code)
			{
				case SCN_STYLENEEDED: return sci->signal_style_needed().emit(*nt);
				case SCN_CHARADDED: return sci->signal_char_added().emit(*nt);
				case SCN_SAVEPOINTREACHED: return sci->signal_save_point_reached().emit(*nt);
				case SCN_SAVEPOINTLEFT: return sci->signal_save_point_left().emit(*nt);
				case SCN_MODIFYATTEMPTRO: return sci->signal_modify_attempt_ro().emit(*nt);
				case SCN_KEY: return sci->signal_key().emit(*nt);
				case SCN_DOUBLECLICK: return sci->signal_double_click().emit(*nt);
				case SCN_UPDATEUI: return sci->signal_update_ui().emit(*nt);
				case SCN_MODIFIED: return sci->signal_modified().emit(*nt);
				case SCN_MACRORECORD: return sci->signal_macro_record().emit(*nt);
				case SCN_MARGINCLICK: return sci->signal_margin_click().emit(*nt);
				case SCN_NEEDSHOWN: return sci->signal_need_shown().emit(*nt);
				case SCN_PAINTED: return sci->signal_painted().emit(*nt);
				case SCN_USERLISTSELECTION: return sci->signal_user_list_selection().emit(*nt);
				case SCN_URIDROPPED: return sci->signal_uri_dropped().emit(*nt);
				case SCN_DWELLSTART: return sci->signal_dwell_start().emit(*nt);
				case SCN_DWELLEND: return sci->signal_dwell_end().emit(*nt);
				case SCN_ZOOM: return sci->signal_zoom().emit(*nt);
				case SCN_HOTSPOTCLICK: return sci->signal_hot_spot_click().emit(*nt);
				case SCN_HOTSPOTDOUBLECLICK: return sci->signal_hot_spot_double_click().emit(*nt);
				case SCN_HOTSPOTRELEASECLICK: return sci->signal_hot_spot_release_click().emit(*nt);
				case SCN_INDICATORCLICK: return sci->signal_indicator_click().emit(*nt);
				case SCN_INDICATORRELEASE: return sci->signal_indicator_release().emit(*nt);
				case SCN_CALLTIPCLICK: return sci->signal_call_tip_click().emit(*nt);
				case SCN_AUTOCSELECTION: return sci->signal_auto_c_selection().emit(*nt);
				case SCN_AUTOCCANCELLED: return sci->signal_auto_c_cancelled().emit(*nt);
				case SCN_AUTOCCHARDELETED: return sci->signal_auto_c_char_deleted().emit(*nt);
				case SCN_FOCUSIN: return sci->signal_focus_in().emit(*nt);
				case SCN_FOCUSOUT: return sci->signal_focus_out().emit(*nt);
				case SCN_AUTOCCOMPLETED: return sci->signal_auto_c_completed().emit(*nt);
				default:
					g_warning("unrecognized Scintilla notification '%d'", nt->nmhdr.code);
					break;
			}
		}
		CXX_BLOCK_END
		return FALSE;
	}

	void on_project_open(GObject*, GKeyFile *kf, gpointer pdata) noexcept
	{
		CXX_BLOCK_BEGIN
		{
			GeanyProject *gproj = Geany::data->app->project;
			g_return_if_fail(gproj);
			auto proxy = ProxyPlugin::from_data(pdata);
			auto proj = proxy->new_project(gproj);
			Glib::KeyFile keyfile(kf);
			for (auto plugin : proxy->plugins.list_plugins())
				plugin->project_open(*proj, keyfile);
		}
		CXX_BLOCK_END
	}

	void on_project_close(GObject*, gpointer pdata) noexcept
	{
		CXX_BLOCK_BEGIN
		{
			auto proxy = ProxyPlugin::from_data(pdata);
			if (proxy->project)
				proxy->project->signal_close().emit();
			for (auto plugin : proxy->plugins.list_plugins())
				plugin->project_close();
			proxy->project = nullptr;
		}
		CXX_BLOCK_END
	}

	static void on_project_dialog_open(GObject*, GtkWidget *notebook, gpointer pdata) noexcept
	{
		CXX_BLOCK_BEGIN
		{
			auto proxy = ProxyPlugin::from_data(pdata);
			g_return_if_fail(proxy->project);
			Gtk::Notebook *nb = Glib::wrap(GTK_NOTEBOOK(notebook));
			proxy->project->signal_dialog_open().emit(nb);
		}
		CXX_BLOCK_END
	}

	static void on_project_dialog_confirmed(GObject*, GtkWidget *notebook, gpointer pdata) noexcept
	{
		CXX_BLOCK_BEGIN
		{
			auto proxy = ProxyPlugin::from_data(pdata);
			g_return_if_fail(proxy->project);
			Gtk::Notebook *nb = Glib::wrap(GTK_NOTEBOOK(notebook));
			proxy->project->signal_dialog_confirmed().emit(nb);
		}
		CXX_BLOCK_END
	}

	static void on_project_dialog_close(GObject*, GtkWidget *notebook, gpointer pdata) noexcept
	{
		CXX_BLOCK_BEGIN
		{
			auto proxy = ProxyPlugin::from_data(pdata);
			g_return_if_fail(proxy->project);
			Gtk::Notebook *nb = Glib::wrap(GTK_NOTEBOOK(notebook));
			proxy->project->signal_dialog_close().emit(nb);
		}
		CXX_BLOCK_END
	}

	gboolean proxy_init(GeanyPlugin *plugin, gpointer) noexcept
	{
		CXX_BLOCK_BEGIN
		{
			Geany::data = plugin->geany_data;
			Geany::ui = new UI(data->main_widgets);

			auto proxy = new ProxyPlugin();
			geany_plugin_set_data(plugin, proxy, nullptr);
			Geany::g_proxy = proxy;

			// todo: go through and add initial docs to docman

			if (Geany::data->app->project)
				proxy->new_project(Geany::data->app->project);

#define PSC(sig_name, cb) \
	plugin_signal_connect(plugin, NULL, sig_name, TRUE, G_CALLBACK(cb), proxy)

			PSC("document-activate", on_document_activate);
			PSC("document-before-save", on_document_before_save);
			PSC("document-close", on_document_close);
			PSC("document-filetype-set", on_document_filetype_set);
			PSC("document-new", on_document_new);
			PSC("document-open", on_document_open);
			PSC("document-reload", on_document_reload);
			PSC("document-save", on_document_save);
			PSC("editor-notify", on_editor_notify);
			PSC("project-open", on_project_open);
			PSC("project-close", on_project_close);
			PSC("project-dialog-open", on_project_dialog_open);
			PSC("project-dialog-confirmed", on_project_dialog_confirmed);
			PSC("project-dialog-close", on_project_dialog_close);

#undef PSC

			static const char *extensions[] = { "plugin", "so", NULL };

			plugin->proxy_funcs->probe = proxy_probe;
			plugin->proxy_funcs->load = proxy_load;
			plugin->proxy_funcs->unload = proxy_unload;

			return geany_plugin_register_proxy(plugin, extensions);
		}
		CXX_BLOCK_END
		return FALSE;
	}

	void proxy_cleanup(GeanyPlugin*, gpointer pdata) noexcept
	{
		delete static_cast<ProxyPlugin*>(pdata);
		delete Geany::ui;
		Geany::ui = nullptr;
		Geany::data = nullptr;
		Geany::g_proxy = nullptr;
	}

} // namespace Geany


GEANYCPP_EXPORT
void geany_load_module(GeanyPlugin *plugin)
{
	CXX_BLOCK_BEGIN
	{
		Geany::init();

		plugin->info->name = "Geany++";
		plugin->info->description = _("C++ proxy plugin");
		plugin->info->version = "0.1";
		plugin->info->author = "Matthew Brush <matt@geany.org>";

		plugin->funcs->init = Geany::proxy_init;
		plugin->funcs->cleanup = Geany::proxy_cleanup;

		GEANY_PLUGIN_REGISTER(plugin, MIN_API);
	}
	CXX_BLOCK_END
}
