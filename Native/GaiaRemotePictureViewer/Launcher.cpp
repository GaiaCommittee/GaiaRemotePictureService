#include <iostream>
#include <boost/program_options.hpp>
#include <chrono>
#include <GaiaRemotePictureClient/GaiaRemotePictureClient.hpp>

int main(int arguments_count, char** arguments)
{
    using namespace Gaia::RemotePicture;
    using namespace boost::program_options;

    options_description options("Options");

    options.add_options()
            ("help,?", "show help message.")
            ("host,a", value<std::string>()->default_value("127.0.0.1"),
                    "host of the remote picture server to connect to.")
            ("port,p", value<unsigned int>()->default_value(6379),
                    "port of the remote picture server to connect to.")
            ("picture,i", value<std::string>(), "name of the picture to show.")
            ("width,w", value<unsigned int>(), "width of the window to resize")
            ("height,h", value<unsigned int>(), "height of the window to resize")
            ("frequency,f", value<unsigned int>()->default_value(30),
                    "update frequency.");
    variables_map variables;
    store(parse_command_line(arguments_count, arguments, options), variables);
    notify(variables);

    if (variables.count("help"))
    {
        std::cout << options << std::endl;
        return 0;
    }

    std::string host;
    if (!variables.count("host"))
    {
        std::cout << "Input host IP: ";
        std::cin >> host;
    }
    else
    {
        host = variables["host"].as<std::string>();
    }

    std::string picture_name;
    if (!variables.count("picture"))
    {
        std::cout << "Input picture name: ";
        std::cin >> picture_name;
    }
    else
    {
        picture_name = variables["picture"].as<std::string>();
    }

    unsigned int frequency = variables["frequency"].as<unsigned int>();
    if (frequency <= 0) frequency = 1;

    RemotePictureClient client(picture_name, variables["port"].as<unsigned int>(), host);

    bool resize = false;
    unsigned int resize_width = 0;
    unsigned int resize_height = 0;
    if (variables.count("width") && variables.count("height"))
    {
        resize_width = variables["width"].as<unsigned int>();
        resize_height = variables["height"].as<unsigned int>();
        resize = true;
    }

    unsigned int received_count = 0;
    client.OnReceive.Add(Gaia::Events::Functor<void>([&received_count]
    {
        ++received_count;
    }));

//    auto last_fps_time = std::chrono::steady_clock::now();

    while (true)
    {
        auto key = cv::waitKey(static_cast<int>(1000 / frequency));

        if (key == 27) break;

        client.Update();

//        auto current_time = std::chrono::steady_clock::now();
//        if (std::chrono::duration_cast<std::chrono::seconds>(current_time - last_fps_time).count() > 1)
//        {
//            std::cout << "FPS: " << received_count << std::endl;
//            received_count = 0;
//            last_fps_time = current_time;
//        }

        if (client.LatestPicture.empty())
        {
            std::cout << "Null picture received." << std::endl;
            continue;
        }

        if (key == 's')
        {
            auto current_time_point = std::chrono::system_clock::now();
            auto global_time = std::chrono::system_clock::to_time_t(current_time_point);
            auto local_time = std::localtime(&global_time);

            std::stringstream name_builder;
            name_builder << "Save_" << local_time->tm_mon << "_" << local_time->tm_mday << "_";
            name_builder << local_time->tm_hour << "_" << local_time->tm_min << "_" << local_time->tm_sec;
            name_builder << ".png";

            cv::imwrite(name_builder.str(), client.LatestPicture);
            std::cout << "Picture saved: " << name_builder.str();
        }

        cv::Mat display_picture;
        if (resize)
        {
            cv::resize(client.LatestPicture, display_picture,
                       cv::Size(static_cast<int>(resize_width), static_cast<int>(resize_height)));
        }
        else
        {
            display_picture = client.LatestPicture;
        }
        cv::imshow(picture_name, display_picture);
    }
    return 0;
}