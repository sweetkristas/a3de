#include <map>
#include <boost/filesystem.hpp>

#include "asserts.hpp"
#include "module.hpp"

namespace module
{
	using namespace boost::filesystem;

	namespace
	{
		const path modpath = "modules";

		struct mod_info
		{
			std::string name;
			std::string pretty_name;
			boost::filesystem::path base;
			std::vector<int> version;
		};

		typedef std::vector<mod_info> mod_list;
		mod_list& get_module_info()
		{
			static mod_list res;
			return res;
		}
	}

	void load_module(const std::string& modname)
	{
		get_module_info().clear();
		mod_info mi;
		mi.name = modname;
		// XXX load module config file
		mi.base = modpath / mi.name;
		get_module_info().push_back(mi);
	}

	std::string map_file(const std::string& file)
	{
		ASSERT_LOG(get_module_info().empty(), "No module loaded.");
		path fpath = get_module_info().back().base / file;
		return fpath.string();
	}
}