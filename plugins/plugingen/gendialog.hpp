#pragma once

#include <gtkmm.h>
#include <string>

struct GenProcessorSettings;

class GenDialog
{
public:
	GenDialog();
	~GenDialog();
	bool run(GenProcessorSettings &settings);

private:
	Gtk::Dialog *dialog;
	Gtk::Entry *ent_name;
	Gtk::Entry *ent_ident;
	Gtk::Entry *ent_desc;
	Gtk::Entry *ent_vers;
	Gtk::Entry *ent_author;
	Gtk::Entry *ent_email;
	Gtk::Entry *ent_support_uri;
	Gtk::Entry *ent_bug_uri;
	Gtk::Entry *ent_basedir;
	Gtk::Entry *ent_templatedir;
	Gtk::CheckButton *chk_init_repo;
	Gtk::CheckButton *chk_init_commit;
	Gtk::CheckButton *chk_use_author;
	Gtk::CheckButton *chk_gen_project;

	void regenerate_ident();
	bool validate();
	void update_settings(GenProcessorSettings &settings);
};
