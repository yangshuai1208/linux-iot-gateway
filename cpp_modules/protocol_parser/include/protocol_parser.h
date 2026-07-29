#ifndef PROTOCOL_PARSER_H
#define PROTOCOL_PARSER_H

#include <map>
#include <string>

enum class Command
{
    Open=0,
    Grab,
    Release,
    Stop,
    Unknown
};
class ProtocolParser
{
    public:
    ProtocolParser();

    Command parse (const std::string &rawInput) const;

    bool isValid(Command command) const;

    std::string commandToString(Command command) const;

    std::string toExecutorCommand(Command command) const;

    private:

    std::string normalizeInput(const std::string&rawInput) const;

    std::map<std::string,Command> commandMap_;
};
#endif