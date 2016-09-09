#pragma once

#include <geany++/geany.hpp>
#include <geany++/scintilla.hpp>
#include <algorithm>
#include <unordered_map>
#include <vector>


#define GROUP_NAME       "cpp-plugin"
#define MODULE_EXTENSION "." G_MODULE_SUFFIX
#define MODULE_SYMBOL    "geanycpp_create_plugin"
#define SPEC_EXTENSION   ".plugin"
#define MIN_API          228


#ifndef _
#define _(s) s
#endif


namespace Geany
{
	typedef IPlugin* (*PluginCreateFunc)(const PluginData&);


	struct PluginModule
	{
		std::string filename;
		Glib::Module module;
		PluginCreateFunc factory_func;
		PluginModule(const std::string &spec_filename);
		bool loaded() const;
		IPlugin *create_plugin(const PluginData &init_data);
		static bool probe(const std::string &fn, bool check_spec=true);
	};


	struct PluginSpecFile
	{
		std::string filename;
		std::string name;
		std::string description;
		std::string version;
		std::string author;
		std::string help_uri;
		bool configurable;
		PluginSpecFile(const std::string &filename);
		bool provides_help() const;
		static bool probe(const std::string &fn, bool check_module=true);
	};


	struct ProxyPlugin;

	struct PluginData
	{
		ProxyPlugin &proxy;
		GeanyPlugin *gplugin;
		PluginSpecFile spec;
		std::unique_ptr<PluginModule> module;
		std::unique_ptr<IPlugin> plugin;
		PluginConfig config;

		PluginData(ProxyPlugin &proxy, GeanyPlugin *gplugin,
			const std::string &spec_filename);
		bool is_initialized() const;
		void init();
		void cleanup();
		static PluginData *from_data(gpointer pdata);
	};


	class DocumentManager
	{
		typedef std::vector<Document*> DocList;
	public:
		const DocList &list() const
		{
			return doclist;
		}

		Document *lookup(GeanyDocument *doc) const
		{
			auto found = docmap.find(doc);
			if (found != docmap.end())
				return found->second.get();
			return nullptr;
		}

		Document *add(GeanyDocument *doc)
		{
			auto docptr = lookup(doc);
			if (docptr)
				return docptr;

			docptr = new Document(doc);
			try
			{
				docmap.emplace(doc, DocPtr(docptr));
				doclist.emplace_back(docptr);
				return docptr;
			}
			catch (...)
			{
				delete docptr;
				throw;
			}
		}

		bool remove(GeanyDocument *doc)
		{
			if (auto docptr = lookup(doc))
			{
				size_t old_size = doclist.size();
				doclist.erase(std::remove(doclist.begin(),
					doclist.end(), docptr), doclist.end());
				return (docmap.erase(doc) > 0 && old_size < doclist.size());
			}
			return false;
		}

		Document *current() const
		{
			return lookup(::document_get_current());
		}

	private:
		typedef std::unique_ptr<Document> DocPtr;
		typedef std::unordered_map<GeanyDocument*, DocPtr> DocMap;
		DocMap docmap;
		DocList doclist;
	};


	class FiletypeManager
	{
		typedef std::unique_ptr<Filetype> FtPtr;
	public:

		FiletypeManager()
		{
			reload();
		}

		size_t count() const
		{
			return ft_list.size();
		}

		const std::vector<Filetype*> &list() const
		{
			return ft_list;
		}

		Filetype *lookup(GeanyFiletype *ft)
		{
			auto found = ft_map.find(ft);
			if (found != ft_map.end())
				return found->second.get();
			return nullptr;
		}

		void reload()
		{
			ft_list.clear();
			ft_map.clear();

			for (size_t i = 0; i < Geany::data->filetypes_array->len; i++)
			{
				auto ft = static_cast<GeanyFiletype*>(
					Geany::data->filetypes_array->pdata[i]);
				auto ft_ptr = new Filetype(ft);
				try
				{
					ft_map.emplace(ft, FtPtr(ft_ptr));
					ft_list.emplace_back(ft_ptr);
				}
				catch (...)
				{
					delete ft_ptr;
					throw;
				}
			}
		}

	private:
		std::unordered_map<GeanyFiletype*, FtPtr> ft_map;
		std::vector<Filetype*> ft_list;
	};


	class PluginManager
	{
	public:

		size_t count() const
		{
			return plugin_map.size();
		}

		const std::vector<IPlugin*> &list_plugins() const
		{
			return plugin_list;
		}

		IPlugin *lookup(GeanyPlugin *p) const
		{
			auto found = plugin_map.find(p);
			if (found != plugin_map.end())
				return found->second;
			return nullptr;
		}

		void add(GeanyPlugin *p, IPlugin *ip)
		{
			plugin_map.emplace(p, ip);
			plugin_list.emplace_back(ip);
		}

		bool remove(GeanyPlugin *p)
		{
			auto ip = lookup(p);
			if (ip && plugin_map.erase(p) > 0)
			{
				plugin_list.erase(std::remove(plugin_list.begin(),
					plugin_list.end(), ip), plugin_list.end());
				return true;
			}
		}

	private:
		std::unordered_map<GeanyPlugin*, IPlugin*> plugin_map;
		std::vector<IPlugin*> plugin_list;
	};


	struct ProxyPlugin
	{
		DocumentManager documents;
		FiletypeManager filetypes;
		PluginManager plugins;
		std::unique_ptr<Project> project;

		ProxyPlugin() : project(nullptr)
		{
		}

		Project *new_project(GeanyProject *gproj)
		{
			project.reset(new Project(gproj));
			return project.get();
		}

		static ProxyPlugin *from_data(gpointer data)
		{
			return static_cast<ProxyPlugin*>(data);
		}
	};


	void init();


	extern ProxyPlugin *g_proxy;
}
