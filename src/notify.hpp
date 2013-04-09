#pragma once

#include <boost/asio.hpp>
#include "dir_monitor.hpp"

namespace notify
{
	class manager
	{
	public:
		manager();
		~manager();
		void poll();
		void register_notification_path(const std::string& path, 
			boost::function<void(const std::string&, const boost::asio::dir_monitor_event&)> fn);
	private:
		void manager::handler(boost::function<void(const std::string&, const boost::asio::dir_monitor_event&)> fn, 
			const boost::system::error_code& e, 
			const boost::asio::dir_monitor_event &ev);

		boost::asio::io_service io_service_;
		boost::asio::dir_monitor dm_;
	};
}
