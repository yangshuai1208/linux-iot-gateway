#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>
#include <vector>

struct CommandRecord
{
    int sequence;
    std::string command;
    bool accepted;
};

std::string normalizeCommand(
    std::string command)
{
    std::transform(
        command.begin(),
        command.end(),
        command.begin(),
        [](unsigned char ch)
        {
            return static_cast<char>(
                std::toupper(ch));
        });

    return command;
}

int main()
{
    std::vector<CommandRecord> records{
        {1, "open", true},
        {2, "Grab", true},
        {3, "jump", false},
        {4, "stop", true},
        {5, "release", true},
        {6, "JUMP", false}};

    std::cout
        << "===== NORMALIZE =====\n";

    std::for_each(
        records.begin(),
        records.end(),
        [](CommandRecord &record)
        {
            record.command =
                normalizeCommand(
                    record.command);
        });

    for (const auto &record : records)
    {
        std::cout
            << "seq="
            << record.sequence
            << " command="
            << record.command
            << " accepted="
            << std::boolalpha
            << record.accepted
            << '\n';
    }

    std::cout
        << "\n===== FIND STOP =====\n";

    const auto stopIterator =
        std::find_if(
            records.begin(),
            records.end(),
            [](const CommandRecord &record)
            {
                return record.command ==
                       "STOP";
            });

    if (stopIterator != records.end())
    {
        std::cout
            << "found STOP, seq="
            << stopIterator->sequence
            << '\n';
    }

    std::cout
        << "\n===== COUNT REJECTED =====\n";

    const auto rejectedCount =
        std::count_if(
            records.begin(),
            records.end(),
            [](const CommandRecord &record)
            {
                return !record.accepted;
            });

    std::cout
        << "rejected="
        << rejectedCount
        << '\n';

    std::cout
        << "\n===== THRESHOLD CAPTURE =====\n";

    int minimumSequence = 4;

    const auto laterCommandCount =
        std::count_if(
            records.begin(),
            records.end(),
            [minimumSequence](
                const CommandRecord &record)
            {
                return record.sequence >=
                       minimumSequence;
            });

    std::cout
        << "sequence >= "
        << minimumSequence
        << ": "
        << laterCommandCount
        << '\n';

    std::cout
        << "\n===== REFERENCE CAPTURE =====\n";

    int acceptedCount = 0;

    std::for_each(
        records.begin(),
        records.end(),
        [&acceptedCount](
            const CommandRecord &record)
        {
            if (record.accepted)
            {
                ++acceptedCount;
            }
        });

    std::cout
        << "accepted="
        << acceptedCount
        << '\n';

    return 0;
}