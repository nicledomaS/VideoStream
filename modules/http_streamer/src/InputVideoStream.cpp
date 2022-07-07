#include "InputVideoStream.h"
#include "InputFormat.h"
#include "DecoderImpl.h"
#include "HWDecoder.h"
#include "EncoderImpl.h"
#include "InputConfig.h"

namespace video_streamer
{

namespace
{

std::unique_ptr<Decoder> createDecoder(AVFormatContext* formatContext, const InputConfig& inputConfig)
{
    if(inputConfig.hwName.empty())
    {
        return std::make_unique<DecoderImpl>(formatContext);
    }
    
    return std::make_unique<HWDecoder>(formatContext, inputConfig.hwName, inputConfig.hwId);
}

} // anonymous

InputVideoStream::InputVideoStream(const InputConfig& inputConfig) noexcept
    : m_format(std::make_unique<InputFormat>(inputConfig.url, inputConfig.inputFormatName, inputConfig.options)),
    m_decoder(createDecoder(m_format->getAVFormatContext(), inputConfig))
{
}

InputVideoStream::~InputVideoStream()
{
}

void InputVideoStream::run(const std::atomic_bool& stopper)
{
	while(!stopper)
    {
        auto packet = m_format->getPacket();
        auto frame = m_decoder->decode(std::move(packet));

        if (frame && m_notify)
        {
            m_notify(std::move(frame));
        }
    }
}

std::shared_ptr<Encoder> InputVideoStream::GetFrameEncoder()
{
    if(!m_frameEncoder)
	{
        m_frameEncoder = std::make_shared<EncoderImpl>("mjpeg", m_decoder->getAVCodecContext());
		if(!m_frameEncoder)
		{
			throw 1;
		}
	}
	
	return m_frameEncoder;
}

} // video_streamer