#include "StreamChannel.h"
#include "StreamController.h"
#include "HttpStreamServer.h"
#include "InputVideoStream.h"
#include "InputConfig.h"
#include "FFmpeg.h"

#include <gsl/gsl>

// #include <boost/program_options.hpp>

#include <chrono>
#include <thread>
#include <iostream>
#include <algorithm>
#include <map>
#include <vector>

// namespace program_opt = boost::program_options;

constexpr auto Help = "help";
constexpr auto Config = "config";
constexpr auto HelpDescription = "produce help message";
constexpr auto ConfigDescription = "set config file";
constexpr auto OptionDescription = "Allowed options";

int toLogLevel(const std::string& strLogLevel);

int main (int argc, char** argv)
{
    try
    {
        // program_opt::options_description desc(OptionDescription);
        // desc.add_options()
        // (Help, HelpDescription)
        // (Config, program_opt::value<std::string>(), ConfigDescription);

        // program_opt::variables_map varMap;
        // program_opt::store(program_opt::parse_command_line(argc, argv, desc), varMap);
        // program_opt::notify(varMap);

        // if (varMap.count(Help) || !varMap.count(Config)) {
        //     std::cout << desc << "\n";
        //     return 0;
        // }

        std::cout << "Start program" << std::endl;

        // auto config = varMap[Config].as<std::string>();
        auto data = toml::parse(argv[1]);
        const auto& http_streamer = toml::find(data, "http_streamer");
        const auto url = toml::find<std::string>(http_streamer, "url");
        const auto cert = toml::find<std::string>(http_streamer, "cert");
        const auto key = toml::find<std::string>(http_streamer, "key");
        auto strLogLevel = toml::find<std::string>(http_streamer, "log_level");
        std::transform(strLogLevel.begin(), strLogLevel.end(), strLogLevel.begin(), ::toupper);
        av_log_set_level(toLogLevel(strLogLevel));
        avdevice_register_all();

        auto streamChannel = std::make_shared<video_streamer::StreamChannel>();
        auto streamServer = std::make_unique<video_streamer::HttpStreamServer>(url, cert, key, streamChannel);
        auto streamController = std::make_shared<video_streamer::StreamController>(streamChannel);

        const auto& inputs = toml::find(data, "inputs");
        const auto items = toml::find<std::vector<video_streamer::InputConfig>>(inputs, "items");

        for(const auto& input : items)
        {
            std::cout << input.id << " " << input.url << std::endl;
            auto inputStream = std::make_unique<video_streamer::InputVideoStream>(input);
            streamController->addInputStream(input.id, std::move(inputStream));
        }

        while(true)
        {
            streamServer->run();
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    catch(...)
    {
        std::cerr << "Exception of unknown type!" << std::endl;
    }

    return 0;
}

int toLogLevel(const std::string& strLogLevel)
{
    static std::map<std::string, int> levels = {
        { "QUIET", AV_LOG_QUIET },
        { "PANIC", AV_LOG_PANIC },
        { "FATAL", AV_LOG_FATAL },
        { "ERROR", AV_LOG_ERROR },
        { "WARNING", AV_LOG_WARNING },
        { "INFO", AV_LOG_INFO },
        { "VERBOSE", AV_LOG_VERBOSE },
        { "DEBUG", AV_LOG_DEBUG }
    };
    
    auto it = levels.find(strLogLevel);
    return it == levels.end() ? AV_LOG_INFO : it->second;
}