#include "protocol_parser.h"

#include <iostream>
#include <string>
#include <vector>

int main()
{
    ProtocolParser parser;

    const std::vector<std::string>testInputs{
        "OPEN",
        "grab",
        "release",
        "\tStop\t",
        "JUMP",
        ""};

    for(const std::string&rawInput:testInputs)
    {
        const Command command=parser.parse(rawInput);
        std::cout
        <<"input:\""
        <<rawInput
        <<"\""
        <<"|command:"
        <<parser.commandToString(command)
        <<"|executor:"
        <<parser.toExecutorCommand(command)
        <<"|valid:"
        <<(parser.isValid(command)
                ?"true"
                    :"false")
        <<'\n';

    }
    return 0;
}