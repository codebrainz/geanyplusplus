#pragma once

#include <geany++/geany.hpp>

struct GenProcessorSettings
{
	Glib::ustring name;
	Glib::ustring identifier;
	Glib::ustring description;
	Glib::ustring version;
	Glib::ustring author_name;
	Glib::ustring author_email;
	Glib::ustring support_url;
	Glib::ustring bug_report_url;
	std::string base_dir;
	std::string template_dir;
	bool init_git_repo;
	bool initial_git_commit;
	bool set_git_author;
	bool generate_project;
};


class GenProcessor
{
public:
	bool process(const GenProcessorSettings &settings);
};
