#pragma once

#include <toml.hpp>

#include <string>
#include <map>

namespace video_streamer
{

struct InputConfig
{
    std::string id;
    std::string url;
    std::string inputFormatName;
    std::string hwName;
    std::string hwId;
    std::map<std::string, std::string> options;

    void from_toml(const toml::value& value);
};

} // video_streamer