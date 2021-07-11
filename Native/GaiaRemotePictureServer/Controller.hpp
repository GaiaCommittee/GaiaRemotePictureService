#pragma once

#include <GaiaLogClient/GaiaLogClient.hpp>
#include <memory>
#include <atomic>
#include <sw/redis++/redis++.h>
#include <shared_mutex>
#include <unordered_map>
#include <GaiaSharedPicture/GaiaSharedPicture.hpp>

namespace Gaia::RemotePicture
{
    class Controller
    {
    protected:
        /// Connection to the Redis server.
        std::shared_ptr<sw::redis::Redis> Connection;
        /// Subscriber for receiving messages.
        std::unique_ptr<sw::redis::Subscriber> Subscriber;
        /// Client for log service.
        std::unique_ptr<LogService::LogClient> Logger;

        std::atomic_bool LifeFlag {false};

        std::shared_mutex ReadersMutex;
        std::unordered_map<std::string, std::shared_ptr<SharedPicture::PictureReader>> Readers;

    public:
        /**
         * @brief Connect to the given Redis server.
         * @param port Port of the Redis server.
         * @param ip IP address of the Redis server.
         */
        explicit Controller(unsigned int port = 6379, const std::string& ip = "127.0.0.1");

        /// Handle and dispatch command.
        void OnCommand(const std::string& text);

        /// Publish a picture into the corresponding Redis channel.
        void PublishPicture(const std::string& picture_name);
        /// Create a shared picture reader and make it ready for publish.
        bool OpenPicture(const std::string& picture_name);
        /// Destroy the corresponding picture reader.
        void ClosePicture(const std::string& picture_name);
        /// Launch the message channel loop.
        void Launch();
    };
}