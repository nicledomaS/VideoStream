#pragma once

#include <memory>
#include <functional>

struct AVFormatContext;
struct AVCodecContext;
struct AVCodec;
struct AVPacket;
struct AVFrame;
struct AVBufferRef;

namespace video_streamer
{

template <typename T>
using AVUniquePtr = std::unique_ptr<T, std::function<void(T*)>>;

template <typename T, typename AllocateFunc, typename Deleter>
AVUniquePtr<T> makeAVUniquePtr(AllocateFunc allocateFunc, Deleter deleter)
{
    AVUniquePtr<T> ptr(nullptr, deleter);
    ptr.reset(allocateFunc());
    return ptr;
}

AVUniquePtr<AVFormatContext> makeAVFormatContext();
AVUniquePtr<AVCodecContext> makeAVCodecContext(const AVCodec *codec);
AVUniquePtr<AVPacket> makeAVPacket();
AVUniquePtr<AVFrame> makeAVFrame();
AVUniquePtr<AVBufferRef> makeHWFrameCtx(AVBufferRef* bufferRef);

} // video_streamer