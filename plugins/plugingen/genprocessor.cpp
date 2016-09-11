#include "genprocessor.hpp"
#include <glib/gstdio.h>
#include <cstdlib>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

typedef std::vector<std::string> StringList;
typedef std::vector<std::pair<std::string, std::string>> ReplList;

static void list_files(StringList &list, const std::string &root)
{
	if (Glib::file_test(root, Glib::FILE_TEST_IS_DIR))
	{
		Glib::Dir dir(root);
		while (true)
		{
			std::string ent(dir.read_name());
			if (! ent.empty())
				list_files(list, Glib::build_filename(root, ent));
			else
				break;
		}
	}
	else if (Glib::str_has_suffix(root, ".in"))
	{
		if (root != "Makefile.in")
			list.emplace_back(root);
	}
}

static std::string strip_prefix(const std::string &str, const std::string &pfx)
{
	auto pos = str.find(pfx);
	if (pos == 0)
		return str.substr(pfx.size());
	return str;
}

static void transform_file_list(const StringList &in_list, StringList &out_list,
	const std::string &template_dir, const std::string &base_dir)
{
	for (auto &file : in_list)
	{
		auto base = strip_prefix(file, template_dir);
		if (Glib::str_has_suffix(base, ".in"))
			base = base.substr(0, base.size() - 3);
		out_list.emplace_back(Glib::build_filename(base_dir, base));
	}
}

static unsigned int create_directories(const StringList &list)
{
	unsigned int failures = 0;
	for (auto &file : list)
	{
		auto dir = Glib::path_get_dirname(file);
		if (g_mkdir_with_parents(dir.c_str(), 0755) != 0)
			failures++;
	}
	return failures;
}

static std::string current_year_string()
{
	auto dt = Glib::DateTime::create_now_local();
	return std::to_string(dt.get_year());
}

static void make_replacements_list(ReplList &list, const GenProcessorSettings &settings)
{
	list.emplace_back("${plugin_name}", settings.name);
	list.emplace_back("${plugin_identifier}", settings.identifier);
	list.emplace_back("${plugin_description}", settings.description);
	list.emplace_back("${plugin_version}", settings.version);
	list.emplace_back("${plugin_author_name}", settings.author_name);
	list.emplace_back("${plugin_author_email}", settings.author_email);
	list.emplace_back("${plugin_support_url}", settings.support_url);
	list.emplace_back("${plugin_bug_report_url}", settings.bug_report_url);
	list.emplace_back("${plugin_base_dir}", settings.base_dir);
	list.emplace_back("${plugin_template_dir}", settings.template_dir);
	list.emplace_back("${plugin_year}", current_year_string());

	auto src_file = Glib::build_filename(settings.base_dir, settings.identifier + ".cpp");
	list.emplace_back("${plugin_cpp_file_escaped}", Glib::uri_escape_string(src_file));
}

static void str_replace(std::string &haystack, const std::string &needle,
		const std::string &replacement)
{
	if (needle.size() > haystack.size())
		return;

	for (size_t i=0; ; i += replacement.size())
	{
		i = haystack.find(needle, i);
		if (i == haystack.npos)
			return;
		if (needle.size() == replacement.size())
			haystack.replace(i, needle.size(), replacement);
		else
		{
			haystack.erase(i, needle.size());
			haystack.insert(i, replacement);
		}
	}
}

static void perform_replacements(std::string &haystack, const ReplList &repl_list)
{
	for (auto &pair : repl_list)
		str_replace(haystack, pair.first, pair.second);
}

static void replace_file_names(StringList &out_list, const ReplList &repl_list)
{
	for (auto &fn : out_list)
		perform_replacements(fn, repl_list);
}

static void replace_file_contents(const StringList &in_list,
	const StringList &out_list, const ReplList &repl_list)
{
	g_return_if_fail(in_list.size() == out_list.size());

	for (size_t i = 0; i < in_list.size(); i++)
	{
		auto contents = Glib::file_get_contents(in_list[i]);
		perform_replacements(contents, repl_list);
		Glib::file_set_contents(out_list[i], contents);
		if (Glib::str_has_suffix(out_list[i], "autogen.sh"))
			g_chmod(out_list[i].c_str(), 0755);
	}
}

static bool post_process_options(const GenProcessorSettings &settings, const StringList &out_list)
{
	if (settings.init_git_repo)
	{
		auto old_dir = Glib::get_current_dir();
		if (g_chdir(settings.base_dir.c_str()) != 0)
			return false;
		try
		{
			if (std::system("git init") != 0)
			{
				g_chdir(old_dir.c_str());
				return false;
			}
			if (settings.set_git_author)
			{
				std::stringstream ss;
				ss << "git config user.name \"" << settings.author_name << "\"";
				if (std::system(ss.str().c_str()) != 0)
				{
					g_chdir(old_dir.c_str());
					return false;
				}
				ss = std::stringstream();
				ss << "git config user.email \"" << settings.author_email << "\"";
				if (std::system(ss.str().c_str()) != 0)
				{
					g_chdir(old_dir.c_str());
					return false;
				}
			}
			if (settings.initial_git_commit)
			{
				if (std::system("git add .") != 0)
				{
					g_chdir(old_dir.c_str());
					return false;
				}
				if (std::system("git commit -m \"Initial commit\"") != 0)
				{
					g_chdir(old_dir.c_str());
					return false;
				}
			}
			g_chdir(old_dir.c_str());
		}
		catch (...)
		{
			g_chdir(old_dir.c_str());
			throw;
		}
	}
}

static void remove_project_file(StringList &list)
{
	for (size_t i = 0; i < list.size(); i++)
	{
		if (Glib::str_has_suffix(list[i], ".geany"))
		{
			list.erase(list.begin() + i);
			return;
		}
	}
}

// TODO: move this to plugingen.cpp
static void finish_processing(const GenProcessorSettings &settings)
{
#if GEANY_API_VERSION >= 229
	if (settings.generate_project)
	{
		Gtk::MessageDialog dlg(Glib::ustring::compose(
			_("The C++ plugin '%1' was generated successfully in '%2'!\n\n"
			"Would you like to open the plugin's project now?"),
			settings.name, settings.base_dir), false, Gtk::MESSAGE_INFO,
			Gtk::BUTTONS_YES_NO, true);
		if (dlg.run() == Gtk::RESPONSE_YES)
		{
			dlg.hide();
			auto fn = Glib::build_filename(settings.base_dir, settings.identifier + ".geany");
			Geany::Project::open(fn);
		}
	}
	else
#endif
	{
		Gtk::MessageDialog dlg(Glib::ustring::compose(
			_("The C++ plugin '%1' was generated successfully in '%2'!\n\n"
			"Would you like to open the main C++ source file for the plugin?"),
			settings.name, settings.base_dir), false, Gtk::MESSAGE_INFO,
			Gtk::BUTTONS_YES_NO, true);
		if (dlg.run() == Gtk::RESPONSE_YES)
		{
			dlg.hide();
			auto fn = Glib::build_filename(settings.base_dir, settings.identifier + ".cpp");
			Geany::Document::open(fn);
		}
	}
}

// TODO: handle errors and give proper return value
bool GenProcessor::process(const GenProcessorSettings &settings)
{
	StringList in_list;
	StringList out_list;
	ReplList repl_list;
	list_files(in_list, settings.template_dir);
	transform_file_list(in_list, out_list, settings.template_dir, settings.base_dir);
	make_replacements_list(repl_list, settings);
	create_directories(out_list);
	replace_file_names(out_list, repl_list);
	if (! settings.generate_project)
		remove_project_file(out_list);
	replace_file_contents(in_list, out_list, repl_list);
	post_process_options(settings, out_list);
	finish_processing(settings);
	return true;
}
