#include "InputConfig.h"

namespace video_streamer
{

void InputConfig::from_toml(const toml::value& value)
{
    id = toml::find<std::string>(value, "id");
    inputFormatName = toml::find<std::string>(value, "input_format");
    url = toml::find<std::string>(value, "url");
    hwName = toml::find<std::string>(value, "hw_name");
    hwId = toml::find<std::string>(value, "hw_id");
    options = toml::find<std::map<std::string, std::string>>(value, "options");
}

} // video_streamer