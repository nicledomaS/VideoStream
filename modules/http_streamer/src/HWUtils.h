#pragma once

#include <string>

enum AVPixelFormat;
enum AVHWDeviceType;

struct AVBufferRef;
struct AVCodecContext;

namespace video_streamer
{

AVBufferRef* createHwContext(const std::string& hwName, const std::string& id);
void setHWFrameCtx(AVCodecContext *ctx, AVBufferRef *hw_device_ctx);
bool isHWPixFmt(const AVPixelFormat pixFmt);
AVPixelFormat FindHWPixFmt(const AVHWDeviceType type);

} // video_streamer