#include "Controller.hpp"

namespace Gaia::RemotePicture
{
    /// Construct and connect to the Redis server.
    Controller::Controller(unsigned int port, const std::string &ip)
    {
        sw::redis::ConnectionOptions connection_options;
        connection_options.socket_timeout = std::chrono::milliseconds(100);
        connection_options.host = ip;
        connection_options.port = static_cast<int>(port);
        connection_options.type = sw::redis::ConnectionType::TCP;

        Connection = std::make_shared<sw::redis::Redis>(connection_options);
        Subscriber = std::make_unique<sw::redis::Subscriber>(std::move(Connection->subscriber()));

        Logger = std::make_unique<LogService::LogClient>(Connection);
        Logger->Author = "RemotePictureServer";

        Subscriber->subscribe("remote_pictures/command");
        Subscriber->on_message([this](const std::string& channel, const std::string& value){
            this->OnCommand(value);
        });
    }

    /// Launch the server.
    void Controller::Launch()
    {
        if (LifeFlag) return;
        LifeFlag = true;

        Logger->RecordMilestone("Remote picture server launched.");

        while (LifeFlag)
        {
            try
            {
                Subscriber->consume();
            }catch (sw::redis::TimeoutError& error)
            {}
        }
    }

    /// Handle command.
    void Controller::OnCommand(const std::string &text)
    {
        auto comma_index = text.find(',');
        std::string command, argument;
        if (comma_index == std::string::npos)
        {
            command = text;
        }
        else
        {
            command = text.substr(0, comma_index);
            argument = text.substr(comma_index + 1);
        }
        if (command == "shutdown") {
            LifeFlag = false;
            Logger->RecordMilestone("Shutdown command received.");
        } else if (command == "request") {
            PublishPicture(argument);
        } else if (command == "close") {
            ClosePicture(argument);
        } else if (command == "open") {
            OpenPicture(argument);
        }
    }

    /// PublishPicture a picture into the corresponding Redis channel.
    void Controller::PublishPicture(const std::string &picture_name)
    {
        std::shared_lock query_lock(ReadersMutex);
        auto finder = Readers.find(picture_name);
        query_lock.unlock();
        if (finder == Readers.end())
        {
            if (!OpenPicture(picture_name))
            {
                Logger->RecordMessage("Due to the failure of open picture, publish operation canceled.");
                return;
            }
        }
        query_lock.lock();
        finder = Readers.find(picture_name);
        auto picture = finder->second->Read();
        query_lock.unlock();
        std::vector<unsigned char> picture_code;
        cv::imencode(".jpg", picture, picture_code, {cv::IMWRITE_JPEG_QUALITY, 80});
        Connection->publish("remote_pictures/" + picture_name,
                            std::string_view(
                                    reinterpret_cast<char*>(picture_code.data()), picture_code.size()));
    }

    /// Open a picture.
    bool Controller::OpenPicture(const std::string &picture_name)
    {
        std::unique_lock lock(ReadersMutex);
        auto finder = Readers.find(picture_name);
        if (finder == Readers.end())
        {
            Logger->RecordMessage("Picture " + picture_name + " is required to open.");
            try
            {
                Readers.emplace(picture_name, std::make_shared<SharedPicture::PictureReader>(picture_name));
            } catch (std::exception& error)
            {
                Logger->RecordError("Failed to open picture " + picture_name + " :" + error.what());
                return false;
            }
            Logger->RecordMessage("Picture " + picture_name + " is opened.");
        }
        return true;
    }

    /// Close a opened picture.
    void Controller::ClosePicture(const std::string &picture_name)
    {
        std::unique_lock lock(ReadersMutex);
        auto finder = Readers.find(picture_name);
        if (finder != Readers.end())
        {
            Readers.erase(finder);
            Logger->RecordMessage("Picture " + picture_name + " is closed.");
        }
        else
        {
            Logger->RecordWarning("Can not find the picture to close: " + picture_name);
        }
    }
}