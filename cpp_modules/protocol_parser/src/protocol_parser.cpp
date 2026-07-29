#include "protocol_parser.h"

#include <algorithm>
#include <cctype>

ProtocolParser::ProtocolParser()
:commandMap_{
    {"OPEN",Command::Open},
    {"GRAB",Command::Grab},
    {"RELEASE",Command::Release},
    {"STOP",Command::Stop}
}
{
}
std::string ProtocolParser::normalizeInput(
    const std::string &rawInput)const
{
    const std::size_t first=rawInput.find_first_not_of(" \t\r\n");

    if(first==std::string::npos)
    {
        return "";
    }
    const std::size_t last=rawInput.find_last_not_of(" \t\r\n");

    std::string normalized=rawInput.substr(first,last-first+1);

    std::transform(normalized.begin(),
                   normalized.end(),
                   normalized.begin(),
                   [](unsigned char character)
                {return static_cast<char>(
                  std::toupper(character) 
                );}                     );
                return normalized;
}
Command ProtocolParser::parse(const std::string &rawInput)const
{
    const std::string normalizedInput=this->normalizeInput(rawInput);
    if(normalizedInput.empty())
    {
        return  Command::Unknown;
    }

    const auto iterator=this->commandMap_.find(normalizedInput);

    if(iterator==this->commandMap_.end())
    {
        return Command::Unknown;
    }
    return iterator->second;
}
bool ProtocolParser::isValid(Command command)const
{
    return command!=Command::Unknown;
}
std::string ProtocolParser::commandToString(Command command)const
{
    switch(command)
    {
        case Command::Open:
        return "OPEN";

        case Command::Grab:
        return "GRAB";

        case Command::Release:
        return "RELEASE";

        case Command::Stop:
        return "STOP";

        case Command::Unknown:
        default:
        return "UNKNOWN";

    }
}
std::string ProtocolParser::toExecutorCommand(Command command)const
{
    switch(command)
    {
        case Command::Open:
        return "HAND_OPEN";

        case Command::Grab:
        return "HAND_GRAB";


        case Command::Release:
        return "HAND_RELEASE";

        case Command::Stop:
        return "HAND_STOP";

        case Command::Unknown:
        default: 
        return "HAND_NONE";
    }
}