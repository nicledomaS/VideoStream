#pragma once

#include <toml.hpp>

#include <string>

enum class InputType
{
    Unknown,
    WebCamera,
    IpCamera
};

InputType fromString(const std::string& value);

struct Input
{
    std::string id;
    InputType type;
    std::string url;
    std::string hwName;
    int hwId;

    void from_toml(const toml::value& value);
};