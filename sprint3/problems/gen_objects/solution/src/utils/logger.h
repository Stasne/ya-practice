#pragma once
#include <boost/date_time.hpp>
#include <boost/json.hpp>
#include <boost/log/core.hpp>         // для logging::core
#include <boost/log/expressions.hpp>  // для выражения, задающего фильтр
#include <boost/log/trivial.hpp>      // для BOOST_LOG_TRIVIAL
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <filesystem>
BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "TimeStamp", boost::posix_time::ptime)
BOOST_LOG_ATTRIBUTE_KEYWORD(additional_data, "AdditionalData", boost::json::value)

class Logger {
public:
    Logger();
    static void init(std::filesystem::path path);
    static void Message(std::string_view msg) { BOOST_LOG_TRIVIAL(info) << msg; }
    static void Log(const boost::json::value& json, std::string_view msg = "") {
        BOOST_LOG_TRIVIAL(info) << boost::log::add_value(additional_data, json) << msg;
    }
    static void Log(const boost::json::object& json, std::string_view msg = "") {
        BOOST_LOG_TRIVIAL(info) << boost::log::add_value(additional_data, boost::json::value(json)) << msg;
    }
};