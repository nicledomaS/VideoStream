#include "StreamController.h"
#include "HttpStreamServer.h"
#include "FFmpegInput.h"
#include "FFmpeg.h"
#include "Input.h"

#include <toml.hpp>

#include <gsl/gsl>

#include <boost/program_options.hpp>

#include <chrono>
#include <thread>
#include <iostream>

namespace program_opt = boost::program_options;

constexpr auto Help = "help";
constexpr auto Config = "config";
constexpr auto HelpDescription = "produce help message";
constexpr auto ConfigDescription = "set config file";
constexpr auto OptionDescription = "Allowed options";

int main (int argc, char** argv)
{
    try
    {
        program_opt::options_description desc(OptionDescription);
        desc.add_options()
        (Help, HelpDescription)
        (Config, program_opt::value<std::string>(), ConfigDescription);

        program_opt::variables_map varMap;
        program_opt::store(program_opt::parse_command_line(argc, argv, desc), varMap);
        program_opt::notify(varMap);

        if (varMap.count(Help) || !varMap.count(Config)) {
            std::cout << desc << "\n";
            return 0;
        }

        std::cout << "Start program" << std::endl;

        auto config = varMap[Config].as<std::string>();
        auto data = toml::parse(config);
        const auto& http_streamer = toml::find(data, "http_streamer");
        const auto url = toml::find<std::string>(http_streamer, "url");
        const auto cert = toml::find<std::string>(http_streamer, "cert");
        const auto key = toml::find<std::string>(http_streamer, "key");
        av_log_set_level(AV_LOG_TRACE);
        avdevice_register_all();

        auto streamController = std::make_shared<video_streamer::StreamController>();
        auto streamServer = std::make_unique<video_streamer::HttpStreamServer>(url, cert, key, streamController);

        const auto& inputs = toml::find(data, "inputs");
        const auto items = toml::find<std::vector<Input>>(inputs, "items");

        for(const auto& input : items)
        {
            std::cout << input.id << " " << input.url << std::endl;
            auto inputStream = video_streamer::CreateFFmpegInput(input.id, input.url, input.hwName, input.hwId);
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