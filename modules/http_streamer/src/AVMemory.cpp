#include "AVMemory.h"
#include "FFmpeg.h"

namespace video_streamer
{

namespace
{

void freeAVFormatContext(AVFormatContext* formatContext)
{
    avformat_free_context(formatContext);
}

void freeAVCodecContext(AVCodecContext* codecContext)
{
    avcodec_free_context(&codecContext);
}

void freeAVPacket(AVPacket* packet)
{
    av_packet_unref(packet);
    av_packet_free(&packet);
}

void freeAVFrame(AVFrame* frame)
{
    av_frame_unref(frame);
    av_frame_free(&frame);
}

void freeAVBufferRef(AVBufferRef* bufferRef)
{
    av_buffer_unref(&bufferRef);
}

} // anonymous

AVUniquePtr<AVFormatContext> makeAVFormatContext()
{
    return makeAVUniquePtr<AVFormatContext>(&avformat_alloc_context, &freeAVFormatContext);
}

AVUniquePtr<AVCodecContext> makeAVCodecContext(const AVCodec *codec)
{
    return makeAVUniquePtr<AVCodecContext>(
        std::bind(&avcodec_alloc_context3, codec), &freeAVCodecContext);
}

AVUniquePtr<AVPacket> makeAVPacket()
{
    return makeAVUniquePtr<AVPacket>(&av_packet_alloc, &freeAVPacket);
}

AVUniquePtr<AVFrame> makeAVFrame()
{
    return makeAVUniquePtr<AVFrame>(&av_frame_alloc, &freeAVFrame);
}

AVUniquePtr<AVBufferRef> makeHWFrameCtx(AVBufferRef* bufferRef)
{
    return makeAVUniquePtr<AVBufferRef>(
        std::bind(&av_hwframe_ctx_alloc, bufferRef), &freeAVBufferRef);
}

} // video_streamer