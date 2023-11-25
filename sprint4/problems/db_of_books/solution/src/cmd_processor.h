#pragma once
// #include <boost/json.hpp>
// #include <functional>

// using CmdHandler = std::function<void(boost::json::object&)>;

// class CmdProcessor {
// public:
//     CmdProcessor() = default;
//     void AddHandler(std::string cmd, CmdHandler&& handler) { handlers_[cmd] = std::move(handler); }
//     void operator()() private : std::unordered_map<std::string, CmdHandler> handlers_;
// };