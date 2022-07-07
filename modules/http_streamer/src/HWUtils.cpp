#include "HWUtils.h"
#include "AVMemory.h"
#include "FFmpeg.h"

namespace video_streamer
{

AVBufferRef* createHwContext(const std::string& hwName, const std::string& id)
{
    AVHWDeviceType deviceType = av_hwdevice_find_type_by_name(hwName.c_str());
    if (deviceType == AVHWDeviceType::AV_HWDEVICE_TYPE_NONE)
    {
        return nullptr;
    }

    AVBufferRef* hw_device_ctx = nullptr;
    if(av_hwdevice_ctx_create(&hw_device_ctx, deviceType, id.c_str(), nullptr, 0) < 0)
    {
        av_log(NULL, AV_LOG_INFO, "createHwContext error\n");
    }

    return hw_device_ctx;
}

void setHWFrameCtx(AVCodecContext *ctx, AVBufferRef *hw_device_ctx)
{
    auto hwFramesRef = makeHWFrameCtx(hw_device_ctx);
    if (!hwFramesRef) {
        // fprintf(stderr, "Failed to create VAAPI frame context.\n");
        return ;
    }

    auto* frames_ctx = reinterpret_cast<AVHWFramesContext*>(hwFramesRef->data);
    frames_ctx->format = ctx->pix_fmt;
    frames_ctx->sw_format = AV_PIX_FMT_NV12;
    frames_ctx->width = ctx->width;
    frames_ctx->height = ctx->height;
    frames_ctx->initial_pool_size = 20;

    enum AVPixelFormat *formats;
  
    auto ret = av_hwframe_transfer_get_formats(hwFramesRef.get(),
                                               AV_HWFRAME_TRANSFER_DIRECTION_TO,
                                               &formats, 0);

    for(int i = 0; i < ret; ++i)
    {
        av_log(nullptr, AV_LOG_INFO, "makeEncoderContext encoder pix fmt: %s\n", av_get_pix_fmt_name(formats[i]));
    }

    if (av_hwframe_ctx_init(hwFramesRef.get()) < 0) {
        return ;
    }
    ctx->hw_frames_ctx = av_buffer_ref(hwFramesRef.get());
}

bool isHWPixFmt(const AVPixelFormat pixFmt)
{
    switch (pixFmt) {
    case AVPixelFormat::AV_PIX_FMT_VAAPI:
    case AVPixelFormat::AV_PIX_FMT_DXVA2_VLD:
    case AVPixelFormat::AV_PIX_FMT_D3D11:
    case AVPixelFormat::AV_PIX_FMT_VDPAU:
    case AVPixelFormat::AV_PIX_FMT_VIDEOTOOLBOX:
    case AVPixelFormat::AV_PIX_FMT_CUDA:
        return true;
    default:
        return false;
    }
}

AVPixelFormat FindHWPixFmt(const AVHWDeviceType type)
{
    enum AVPixelFormat fmt;
    switch (type) {
    case AVHWDeviceType::AV_HWDEVICE_TYPE_CUDA:
        fmt = AVPixelFormat::AV_PIX_FMT_CUDA;
        break;
    case AVHWDeviceType::AV_HWDEVICE_TYPE_VAAPI:
        fmt = AVPixelFormat::AV_PIX_FMT_VAAPI;
        break;
    case AVHWDeviceType::AV_HWDEVICE_TYPE_DXVA2:
        fmt = AVPixelFormat::AV_PIX_FMT_DXVA2_VLD;
        break;
    case AVHWDeviceType::AV_HWDEVICE_TYPE_D3D11VA:
        fmt = AVPixelFormat::AV_PIX_FMT_D3D11VA_VLD;
        break;
    case AVHWDeviceType::AV_HWDEVICE_TYPE_VDPAU:
        fmt = AVPixelFormat::AV_PIX_FMT_VDPAU;
        break;
    case AVHWDeviceType::AV_HWDEVICE_TYPE_VIDEOTOOLBOX:
        fmt = AVPixelFormat::AV_PIX_FMT_VIDEOTOOLBOX;
        break;
    default:
        fmt = AVPixelFormat::AV_PIX_FMT_NONE;
        break;
    }
    return fmt;
}

} // video_streamer