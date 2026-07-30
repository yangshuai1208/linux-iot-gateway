#include "log_manager.h"
#include "protocol_parser.h"

#include <iostream>
#include <string>
#include <vector>

int main()
{
    LogManager logger("gateway_cpp_demo.log");
    ProtocolParser parser;

    logger.setConsoleEnabled(true)
    .setAutoFlush(true)
   .setMaxCachedLogs(5);

    if(!logger.isFileOpen())
    {
        std::cerr<<"[ERROR]Logger initialization failed"
        <<'\n';

        return 1;
    }
    const std::vector<std::string>testInputs{
        "OPEN",
        "grab\r\n",
        "JUMP",
        "\tSTOP\t"};

    for(const std::string &rawInput:testInputs)
    {
        const Command command=
        parser.parse(rawInput);

        if(!parser.isValid(command))
        {
            logger.log(
            LogLevel::Warning,
            "Rejected invalid command");

            continue;
        }
        const std::string commandName=parser.commandToString(command);

        const std::string executorCommand=parser.toExecutorCommand(command);

        logger.log(LogLevel::Info,"Accepted command "+commandName+"->"+executorCommand);
    }
    std::cout<<"[INFO]Log statistics:"
            <<'\n';

    std::cout<<"INFO:"
            <<logger.getLogCount(LogLevel::Info)
            <<'\n';

    std::cout<<"WARNING:"
            <<logger.getLogCount(LogLevel::Warning)
            <<'\n';

    std::cout<<"ERROR:"
            <<logger.getLogCount(LogLevel::Error)
            <<'\n';
    
    const std::vector<std::string>&recentLogs=logger.getRecentLogs();

    std::cout<<"[INFO]Recent cached logs:"
            <<'\n';
        
    for(const std::string&line:recentLogs)
    {
        std::cout<<" "
                <<line
                <<'\n';
    }
    return 0;
}