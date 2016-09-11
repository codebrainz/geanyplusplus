#include "gendialog.hpp"
#include "genprocessor.hpp"
#include <geany++/geany.hpp>
#include <cstdarg>

static void make_clearable_entry(Gtk::Entry *ent)
{
	ent->set_icon_from_stock(Gtk::Stock::CLEAR, Gtk::ENTRY_ICON_SECONDARY);
	ent->set_icon_sensitive(Gtk::ENTRY_ICON_SECONDARY, !ent->get_text().empty());
	ent->set_icon_activatable(true, Gtk::ENTRY_ICON_SECONDARY);

	ent->signal_icon_release().connect(
		[ent](Gtk::EntryIconPosition pos, const GdkEventButton*) {
			if (pos == Gtk::ENTRY_ICON_SECONDARY)
				ent->set_text("");
	});

	ent->signal_changed().connect([ent]() {
		ent->set_icon_sensitive(Gtk::ENTRY_ICON_SECONDARY,
			!ent->get_text().empty());
	});
}

static void make_clearable_entries(Gtk::Entry *ent, ...)
{
	va_list ap;
	va_start(ap, ent);
	while (ent != nullptr)
	{
		make_clearable_entry(ent);
		ent = va_arg(ap, Gtk::Entry*);
	}
	va_end(ap);
}

static void setup_folder_browser(Gtk::Window &parent, Gtk::Entry *ent,
	Gtk::Button *btn, const Glib::ustring &dir_type)
{
	btn->signal_clicked().connect([&parent, ent, dir_type]() {
		Gtk::FileChooserDialog dlg(parent,
			Glib::ustring::compose(_("Select %1 Directory"), dir_type),
			Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
		dlg.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
		dlg.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
		dlg.set_create_folders(true);
		if (dlg.run() == Gtk::RESPONSE_OK)
			ent->set_text(Glib::filename_to_utf8(dlg.get_filename()));
	});
}

GenDialog::GenDialog()
	: dialog(nullptr),
	  ent_name(nullptr),
	  ent_ident(nullptr),
	  ent_desc(nullptr),
	  ent_vers(nullptr),
	  ent_author(nullptr),
	  ent_email(nullptr),
	  ent_support_uri(nullptr),
	  ent_bug_uri(nullptr),
	  ent_basedir(nullptr),
	  ent_templatedir(nullptr),
	  chk_init_repo(nullptr),
	  chk_init_commit(nullptr),
	  chk_use_author(nullptr),
	  chk_gen_project(nullptr)
{
	auto builder = Gtk::Builder::create_from_file(PLUGINGEN_UI_FILE);

	builder->get_widget("dlg_main", dialog);
	builder->get_widget("ent_name", ent_name);
	builder->get_widget("ent_ident", ent_ident);
	builder->get_widget("ent_desc", ent_desc);
	builder->get_widget("ent_vers", ent_vers);
	builder->get_widget("ent_author", ent_author);
	builder->get_widget("ent_email", ent_email);
	builder->get_widget("ent_support_uri", ent_support_uri);
	builder->get_widget("ent_bug_uri", ent_bug_uri);
	builder->get_widget("ent_basedir", ent_basedir);
	builder->get_widget("ent_templatedir", ent_templatedir);
	builder->get_widget("chk_init_repo", chk_init_repo);
	builder->get_widget("chk_init_commit", chk_init_commit);
	builder->get_widget("chk_use_author", chk_use_author);
	builder->get_widget("chk_gen_project", chk_gen_project);

	dialog->set_default_size(500, -1);
	dialog->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	dialog->add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);

	std::string def_path = Glib::build_filename(
		Glib::get_home_dir(), "plugins", "untitled");
	ent_basedir->set_text(def_path);

	ent_templatedir->set_text(PLUGINGEN_TEMPLATE_DIR);

	make_clearable_entries(ent_name, ent_ident, ent_desc, ent_vers,
		ent_author, ent_email, ent_support_uri, ent_bug_uri, ent_basedir, nullptr);

	Gtk::Button *btn_browse = nullptr;
	builder->get_widget("btn_browse_basedir", btn_browse);
	setup_folder_browser(*dialog, ent_basedir, btn_browse, _("Base"));

	btn_browse = nullptr;
	builder->get_widget("btn_browse_templatedir", btn_browse);
	setup_folder_browser(*dialog, ent_templatedir, btn_browse, _("Template"));

	Gtk::Button *btn_regen = nullptr;
	builder->get_widget("btn_regen_ident", btn_regen);

	btn_regen->signal_clicked().connect(
		sigc::mem_fun(*this, &GenDialog::regenerate_ident));

	chk_init_repo->signal_toggled().connect([this]() {
			bool active = chk_init_repo->get_active();
			chk_init_commit->set_sensitive(active);
			chk_use_author->set_sensitive(active);
	});
}

GenDialog::~GenDialog()
{
	delete dialog;
}

bool GenDialog::run(GenProcessorSettings &settings)
{
	int response = 0;
	while ((response = dialog->run()) == Gtk::RESPONSE_OK)
	{
		dialog->hide();
		if (validate())
			break;
	}
	if (response == Gtk::RESPONSE_OK)
	{
		update_settings(settings);
		return true;
	}
	return false;
}

static void show_error_dialog(const Glib::ustring &msg)
{
	Gtk::MessageDialog dlg(msg, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
	dlg.set_title(_("Error"));
	dlg.run();
}

static bool validate_entry(Gtk::Entry *ent, const Glib::ustring &name)
{
	if (ent->get_text().size() == 0)
	{
		show_error_dialog(Glib::ustring::compose(
			_("The '%1' field is required."), name));
		ent->grab_focus();
		ent->select_region(0, -1);
		return false;
	}
	return true;
}

bool GenDialog::validate()
{
	if (! validate_entry(ent_name, _("Name")))
		return false;
	else if (! validate_entry(ent_ident, _("Identifier")))
		return false;
	else if (! validate_entry(ent_desc, _("Description")))
		return false;
	else if (! validate_entry(ent_vers, _("Version")))
		return false;
	else if (! validate_entry(ent_author, _("Author Name")))
		return false;
	else if (! validate_entry(ent_email, _("Author Email")))
		return false;
	else if (! validate_entry(ent_basedir, _("Base Directory")))
		return false;
	else if (! validate_entry(ent_templatedir, _("Template Directory")))
		return false;
	return true;
}

void GenDialog::regenerate_ident()
{
	Glib::ustring ident;
	Glib::ustring nm(ent_name->get_text());

	char last = ' ';
	for (size_t i=0; i < nm.size(); i++)
	{
		if (i == 0 && Glib::Ascii::isdigit(nm[i]))
			ident += '_';
		else if (nm[i] == '_' || Glib::Ascii::isalnum(nm[i]))
			ident += Glib::Ascii::tolower(nm[i]);
		else if (last != '_')
			ident += '_';
		last = nm[i];
	}

	ent_ident->set_text(ident);
}

void GenDialog::update_settings(GenProcessorSettings &settings)
{
	settings.name = ent_name->get_text();
	settings.identifier = ent_ident->get_text();
	settings.description = ent_desc->get_text();
	settings.version = ent_vers->get_text();
	settings.author_name = ent_author->get_text();
	settings.author_email = ent_email->get_text();
	settings.support_url = ent_support_uri->get_text();
	settings.bug_report_url = ent_bug_uri->get_text();
	settings.base_dir = Glib::filename_from_utf8(ent_basedir->get_text());
	settings.template_dir = Glib::filename_from_utf8(ent_templatedir->get_text());
	settings.init_git_repo = chk_init_repo->get_active();
	settings.initial_git_commit = chk_init_commit->get_active();
	settings.set_git_author = chk_use_author->get_active();
	settings.generate_project = chk_gen_project->get_active();
}
