#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <boost/bind.hpp>
#include <boost/filesystem.hpp>

#include "asserts.hpp"
#include "notify.hpp"

namespace notify
{
	manager::manager()
		: dm_(io_service_)
	{
	}

	manager::~manager()
	{
	}

	void manager::register_notification_path(const std::string& name, 
		boost::function<void(const std::string&, const boost::asio::dir_monitor_event&)> fn)
	{
		dm_.add_directory(name);
		dm_.async_monitor(boost::bind(&manager::handler, this, fn, _1, _2));
	}

	void manager::handler(boost::function<void(const std::string&, const boost::asio::dir_monitor_event&)> fn, 
		const boost::system::error_code& ec, 
		const boost::asio::dir_monitor_event &ev)
	{
		if(ec) {
			std::cerr << "Error waiting for directory change: " << ec.message() << std::endl;
			return;
		}
		boost::filesystem::path p(ev.dirname + ev.filename);
		if(boost::filesystem::is_regular_file(p)) {
			fn(p.generic_string(), ev);
		}
		dm_.async_monitor(boost::bind(&manager::handler, this, fn, _1, _2));		
	}

	void manager::poll()
	{
		io_service_.reset();
		io_service_.poll();
	}
}
