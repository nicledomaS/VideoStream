#include "Input.h"

#include <map>

InputType fromString(const std::string& value)
{
    static std::map<std::string, InputType> inputTypes = {
        {"WebCamera", InputType::WebCamera},
        {"IpCamera", InputType::IpCamera}
    };

    auto it = inputTypes.find(value);
    return it == inputTypes.end() ? InputType::Unknown : it->second;
}

void Input::from_toml(const toml::value& value)
{
    id = toml::find<std::string>(value, "id");
    type = fromString(toml::find<std::string>(value, "type"));
    url = toml::find<std::string>(value, "url");
    hwName = toml::find<std::string>(value, "hw_name");
    hwId = toml::find<int>(value, "hw_id");
    return;
}