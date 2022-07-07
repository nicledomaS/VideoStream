#pragma once

#include "Format.h"

#include <string>
#include <map>

namespace video_streamer
{

class InputFormat : public Format
{
public:
    InputFormat(
        const std::string& url,
        const std::string& inputFormatName,
        const std::map<std::string, std::string>& options);

    AVUniquePtr<AVPacket> getPacket() override;
    AVFormatContext* getAVFormatContext() const override;

private:
    AVUniquePtr<AVFormatContext> m_formatContext;
};

} // video_streamer