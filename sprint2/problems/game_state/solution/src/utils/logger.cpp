#include "logger.h"

#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>

namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

void MyFormatter(logging::record_view const& rec, logging::formatting_ostream& strm) {
    auto ts = *rec[timestamp];
    boost::json::object record;

    record["timestamp"] = to_iso_extended_string(ts);

    auto msgPtr = rec[logging::expressions::smessage];
    record["message"] = (msgPtr ? *msgPtr : "");

    if (auto addData = rec[additional_data]; addData)
        record["data"] = *addData;

    strm << record;
}

void Logger::init(std::filesystem::path path) {
    logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::info);
    logging::add_common_attributes();
    // Создаем имя файла с использованием указанного пути
    std::string filename = "/tmp/" + path.filename().string() + "_%N.log";
    logging::add_file_log(keywords::file_name = filename, keywords::format = &MyFormatter,
                          keywords::open_mode = std::ios_base::app | std::ios_base::out,
                          // ротируем по достижению размера 10 мегабайт
                          keywords::rotation_size = 10 * 1024 * 1024,
                          // ротируем ежедневно в полдень
                          keywords::time_based_rotation = sinks::file::rotation_at_time_point(12, 0, 0));

    logging::add_console_log(std::cout, keywords::format = &MyFormatter, keywords::auto_flush = true);
}
