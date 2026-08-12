#include <iostream>
#include <memory>
#include <string>

namespace
{

class SerialPortMock
{
public:
    explicit SerialPortMock(
        const std::string &device)
        : device_(device)
    {
        std::cout
            << "[SERIAL] open "
            << device_
            << '\n';
    }

    ~SerialPortMock()
    {
        std::cout
            << "[SERIAL] close "
            << device_
            << '\n';
    }

    void send(
        const std::string &command)
    {
        std::cout
            << "[SERIAL] send "
            << command
            << '\n';
    }

private:
    std::string device_;
};

struct GatewayConfig
{
    std::string gatewayName;
    int baudRate;
};

class GatewayWorker
{
public:
    GatewayWorker(
        std::unique_ptr<SerialPortMock> serialPort,
        std::shared_ptr<GatewayConfig> config)
        : serialPort_(std::move(serialPort)),
          config_(std::move(config))
    {
    }

    void execute(
        const std::string &command)
    {
        std::cout
            << "[WORKER] gateway="
            << config_->gatewayName
            << " baud="
            << config_->baudRate
            << '\n';

        serialPort_->send(command);
    }

private:
    std::unique_ptr<SerialPortMock> serialPort_;

    std::shared_ptr<GatewayConfig> config_;
};

void showConfig(
    const std::weak_ptr<GatewayConfig> &weakConfig)
{
    const std::shared_ptr<GatewayConfig> config =
        weakConfig.lock();

    if (!config)
    {
        std::cout
            << "[MONITOR] config expired"
            << '\n';

        return;
    }

    std::cout
        << "[MONITOR] gateway="
        << config->gatewayName
        << '\n';
}

} // namespace

int main()
{
    auto config =
        std::make_shared<GatewayConfig>(
            GatewayConfig{
                "linux-iot-gateway",
                115200});

    std::cout
        << "[MAIN] config use_count="
        << config.use_count()
        << '\n';

    std::weak_ptr<GatewayConfig> weakConfig =
        config;

    auto serialPort =
        std::make_unique<SerialPortMock>(
            "/dev/ttyUSB0");

    GatewayWorker worker(
        std::move(serialPort),
        config);

    std::cout
        << "[MAIN] serial moved="
        << std::boolalpha
        << (serialPort == nullptr)
        << '\n';

    std::cout
        << "[MAIN] config use_count="
        << config.use_count()
        << '\n';

    worker.execute(
        "HAND_GRAB");

    showConfig(
        weakConfig);

    config.reset();

    std::cout
        << "[MAIN] external config reset"
        << '\n';

    showConfig(
        weakConfig);

    return 0;
}