#pragma once

#include "InputStream.h"

#include <thread>
#include <atomic>
#include <functional>
#include <memory>

namespace video_streamer
{

class Worker
{
public:
    explicit Worker(std::unique_ptr<InputStream> inputStream)
        : m_thread([inputStream = std::move(inputStream)](const std::atomic_bool& stopper){
            inputStream->run(stopper);
        },
        std::ref(m_stopper))
    {
    }

    ~Worker()
    {
        m_stopper = true;
        if(m_thread.joinable())
        {
            m_thread.join();
        }
    }

private:
    std::atomic_bool m_stopper = false;
    std::thread m_thread;
};

} // video_streamer