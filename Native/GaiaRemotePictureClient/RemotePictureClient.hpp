#pragma once

#include <string>
#include <memory>
#include <optional>
#include <sw/redis++/redis++.h>
#include <opencv2/opencv.hpp>
#include <GaiaEvents/GaiaEvents.hpp>

namespace Gaia::RemotePicture
{
    class RemotePictureClient
    {
    public:
        /**
         * @brief Reuse the connection to a Redis server.
         * @param connection The connection to a Redis server.
         */
        explicit RemotePictureClient(const std::string& picture_name, std::shared_ptr<sw::redis::Redis> connection);
        /**
         * @brief Establish a connection to the Redis server.
         * @param port Port of the Redis server.
         * @param ip IP address of the Redis server.
         */
        explicit RemotePictureClient(std::string  picture_name, unsigned int port = 6379,
                                     const std::string& ip = "127.0.0.1");

    protected:
        /// Connection to the Redis server.
        std::shared_ptr<sw::redis::Redis> Connection;
        /// Subscriber for channel messages.
        std::unique_ptr<sw::redis::Subscriber> Subscriber;

        /// Name of the bound picture.
        std::string PictureName;

        /// Decode the picture data and trigger the event.
        void OnReceivePictureData(const std::string& data);

    public:
        /// Triggered when receive a picture.
        Events::Event<void> OnReceive;

        /// Latest picture transferred.
        cv::Mat LatestPicture;

        /// Consume the channel message.
        void Update();
    };
}