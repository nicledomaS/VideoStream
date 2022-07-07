#include "InputFormat.h"
#include "FFmpeg.h"

#include <gsl/gsl>

#include <iostream>

namespace video_streamer
{

namespace
{

inline AVInputFormat* getAVInputFormat(const std::string& inputFormatName)
{
    return inputFormatName.empty() ? nullptr : av_find_input_format(inputFormatName.c_str());
}

inline AVDictionary* prepareOptions(const std::map<std::string, std::string>& options)
{
    AVDictionary* dictionary = nullptr;
    
    for(const auto& option : options)
    {
        av_dict_set(&dictionary, option.first.c_str(), option.second.c_str(), 0);
    }
		
    return dictionary;
}

AVUniquePtr<AVFormatContext> createAVFormatContext(const std::string& url,
        const std::string& inputFormatName,
        const std::map<std::string, std::string>& options)
{
    auto formatContext = makeAVFormatContext();
    auto* formatContextPtr = formatContext.get();

    auto* inputFormat = getAVInputFormat(inputFormatName);
    auto* dict = prepareOptions(options);

    if (avformat_open_input(&formatContextPtr, url.c_str(), inputFormat, &dict) != 0)
    {
        throw std::runtime_error("Error while open input");
    }
   
    if (avformat_find_stream_info(formatContextPtr, nullptr) < 0)
    {
        throw std::runtime_error("Error while find stream info");
    }

    return formatContext;
}

} // anonymous

InputFormat::InputFormat(
    const std::string& url,
    const std::string& inputFormatName,
    const std::map<std::string, std::string>& options)
    : m_formatContext(createAVFormatContext(url, inputFormatName, options))
{
}

AVUniquePtr<AVPacket> InputFormat::getPacket()
{
    auto packet = makeAVPacket();
    av_read_frame(m_formatContext.get(), packet.get());

    return packet;
}

AVFormatContext* InputFormat::getAVFormatContext() const
{
    return m_formatContext.get();
}

} // video_streamer