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
	namespace
	{
		boost::asio::io_service io_service;
		boost::asio::dir_monitor dm(io_service);
	}

	manager::manager()
	{
	}

	manager::~manager()
	{
	}

	void handler(boost::function<void(const std::string&, const boost::asio::dir_monitor_event&)> fn, 
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
		dm.async_monitor(boost::bind(&handler, fn, _1, _2));		
	}

	void register_notification_path(const std::string& name, 
		boost::function<void(const std::string&, const boost::asio::dir_monitor_event&)> fn)
	{
		dm.add_directory(name);
		dm.async_monitor(boost::bind(&handler, fn, _1, _2));
	}

	void manager::poll()
	{
		io_service.reset();
		io_service.poll();
	}
}
