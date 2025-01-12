#include <boost/date_time.hpp>
#include <boost/log/core.hpp>         // для logging::core
#include <boost/log/expressions.hpp>  // для выражения, задающего фильтр
#include <boost/log/trivial.hpp>      // для BOOST_LOG_TRIVIAL
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <string_view>
using namespace std::literals;
namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

BOOST_LOG_ATTRIBUTE_KEYWORD(line_id, "LineID", unsigned int)
BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "TimeStamp", boost::posix_time::ptime)
BOOST_LOG_ATTRIBUTE_KEYWORD(file, "File", std::string)
BOOST_LOG_ATTRIBUTE_KEYWORD(line, "Line", int)

void MyFormatter(logging::record_view const& rec, logging::formatting_ostream& strm) {
    // Выводить LineID стало проще.
    strm << rec[line_id] << "[" << rec[file] << ":" << rec[line] << "] ";

    // Момент времени приходится вручную конвертировать в строку.
    // Для получения истинного значения атрибута нужно добавить
    // разыменование.
    auto ts = *rec[timestamp];
    strm << to_iso_extended_string(ts) << ": ";

    // Выводим уровень, заключая его в угловые скобки.
    strm << "<" << rec[logging::trivial::severity] << "> ";

    // Выводим само сообщение.
    strm << rec[logging::expressions::smessage];
}

void InitBoostLogFilter() {
    logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::info);
    logging::add_common_attributes();
    logging::add_file_log(keywords::file_name = "sample_%N.log", keywords::format = &MyFormatter,
                          keywords::open_mode = std::ios_base::app | std::ios_base::out,
                          // ротируем по достижению размера 10 мегабайт
                          keywords::rotation_size = 10 * 1024 * 1024,
                          // ротируем ежедневно в полдень
                          keywords::time_based_rotation = sinks::file::rotation_at_time_point(12, 0, 0));

    logging::add_console_log(std::clog, keywords::format = &MyFormatter, keywords::auto_flush = true);
}

int main() {
    InitBoostLogFilter();
    BOOST_LOG_TRIVIAL(trace) << "Сообщение уровня trace"sv;
    BOOST_LOG_TRIVIAL(debug) << "Сообщение уровня debug"sv;
    BOOST_LOG_TRIVIAL(info) << "Сообщение уровня info"sv;
    BOOST_LOG_TRIVIAL(warning) << "Сообщение уровня warning"sv;
    BOOST_LOG_TRIVIAL(error) << "Сообщение уровня error"sv;
    BOOST_LOG_TRIVIAL(fatal) << "Сообщение уровня fatal"sv;
    BOOST_LOG_TRIVIAL(info) << logging::add_value(file, __FILE__) << logging::add_value(line, __LINE__)
                            << "Something happend"sv;
}