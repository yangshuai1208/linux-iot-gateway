#include <algorithm>
#include <cctype>
#include <iostream>
#include <map>
#include <string>
#include <vector>

enum class Command
{
    Open,
    Grab,
    Release,
    Stop,
    Unknown
};

std::string normalizeInput(const std::string &rawInput)
{
    const std::string whitespace = " \t\r\n";

    const std::size_t first =
        rawInput.find_first_not_of(whitespace);

    if (first == std::string::npos)
    {
        return "";
    }

    const std::size_t last =
        rawInput.find_last_not_of(whitespace);

    std::string normalized =
        rawInput.substr(
            first,
            last - first + 1);

    std::transform(
        normalized.begin(),
        normalized.end(),
        normalized.begin(),
        [](unsigned char ch)
        {
            return static_cast<char>(
                std::toupper(ch));
        });

    return normalized;
}

Command parseCommand(const std::string &rawInput)
{
    static const std::map<std::string, Command>
        commandMap{
            {"OPEN", Command::Open},
            {"GRAB", Command::Grab},
            {"RELEASE", Command::Release},
            {"STOP", Command::Stop}};

    const std::string normalized =
        normalizeInput(rawInput);

    const auto iterator =
        commandMap.find(normalized);

    if (iterator == commandMap.end())
    {
        return Command::Unknown;
    }

    return iterator->second;
}

std::string commandToString(Command command)
{
    switch (command)
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

std::string toExecutorCommand(Command command)
{
    switch (command)
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

int main()
{
    const std::vector<std::string> testInputs{
        "OPEN",
        " grab ",
        "\tSTOP\r\n",
        "release",
        "JUMP",
        "   "};

    for (const auto &input : testInputs)
    {
        const Command command =
            parseCommand(input);

        std::cout
            << "raw=\""
            << input
            << "\""
            << " -> command="
            << commandToString(command)
            << " -> executor="
            << toExecutorCommand(command)
            << '\n';
    }

    return 0;
}