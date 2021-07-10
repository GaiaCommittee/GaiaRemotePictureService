#include <iostream>
#include <boost/program_options.hpp>
#include <thread>
#include "Controller.hpp"

int main(int arguments_count, char** arguments)
{
    using namespace Gaia::RemotePicture;
    using namespace boost::program_options;

    options_description options("Options");

    options.add_options()
            ("help,?", "show help message.")
            ("host,h", value<std::string>()->default_value("127.0.0.1"),
             "ip address of the Redis server.")
            ("port,p", value<unsigned int>()->default_value(6379),
             "port of the Redis server.");
    variables_map variables;
    store(parse_command_line(arguments_count, arguments, options), variables);
    notify(variables);

    if (variables.count("help"))
    {
        std::cout << options << std::endl;
        return 0;
    }
    auto option_port = variables["port"].as<unsigned int>();
    auto option_host = variables["host"].as<std::string>();

    bool crashed;
    do
    {
        try
        {
            crashed = false;
            std::cout << "Gaia remote picture server starting..." << std::endl;
            Controller controller(option_port, option_host);
            controller.Launch();
            std::cout << "Gaia remote picture server stopped." << std::endl;
        } catch (std::exception& error)
        {
            crashed = true;
            std::cout << "Gaia remote picture server crashed:" << std::endl;
            std::cout << error.what() << std::endl;
            std::cout << "Restart in 1 seconds..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    } while (crashed);

    return 0;
}