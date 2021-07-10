#include "RemotePictureClient.hpp"

#include <utility>

namespace Gaia::RemotePicture
{
    /// Reuse the connection to a Redis server.
    RemotePictureClient::RemotePictureClient(std::string picture_name, unsigned int port, const std::string &ip) :
            RemotePictureClient(picture_name,
                                std::make_shared<sw::redis::Redis>("tcp://" + ip + ":" + std::to_string(port)))
    {}

    /// Establish a connection to the Redis server.
    RemotePictureClient::RemotePictureClient(
            const std::string& picture_name,
            std::shared_ptr<sw::redis::Redis> connection) :
        Connection(std::move(connection)), PictureName(picture_name)
    {
        Subscriber = std::make_unique<sw::redis::Subscriber>(std::move(Connection->subscriber()));
        Subscriber->subscribe("remote_pictures/" + picture_name);
        Subscriber->on_message([this](const std::string& channel, const std::string& data)
        {
            this->OnReceivePictureData(data);
        });
    }

    /// Decode the picture data and trigger the event.
    void RemotePictureClient::OnReceivePictureData(const std::string& data)
    {
        std::vector<char> bytes(data.begin(), data.end());
        auto picture = cv::imdecode(bytes, cv::IMREAD_ANYCOLOR);
        LatestPicture = picture;
        OnReceive.Trigger();
    }

    /// Consume Redis channel message.
    void RemotePictureClient::Update()
    {
        Connection->publish("remote_pictures/command", "request," + PictureName);
        try
        {
            Subscriber->consume();
        }catch (sw::redis::TimeoutError& error)
        {}
    }
}