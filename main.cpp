#include "lib/AikaPi/AikaPi.h"
#include "lib/pigpio/pigpio.h"

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

constexpr unsigned CS = 16;
constexpr unsigned MISO = 19;
constexpr unsigned MOSI = 20;
constexpr unsigned SCLK = 21;
constexpr unsigned BAUD = 500;
constexpr unsigned SPI_FLAGS = 0;

std::string
data_loopback_using_pigpio(std::string& msg)
{
  gpioInitialise();

  if (bbSPIOpen(CS, MISO, MOSI, SCLK, BAUD, SPI_FLAGS) != 0) {
    throw std::runtime_error("Error with bbSPIOpen!");
  }

  std::vector<char> rx_buff{};

  rx_buff.reserve(msg.size());

  bbSPIXfer((CS),
            reinterpret_cast<char*>(msg.data()),
            reinterpret_cast<char*>(rx_buff.data()),
            msg.size());

  bbSPIClose(CS);

  gpioTerminate();

  return std::string{ rx_buff.data() };
}

std::string
data_loopback_using_aikapi(std::string& msg)
{
  AikaPi& ap = AikaPi::get_instance();

  ap.aux.master_enable_spi(0);

  ap.gpio.set(SCLK, AP::GPIO::FUNC::ALT4, AP::GPIO::PULL::OFF);
  ap.gpio.set(MOSI, AP::GPIO::FUNC::ALT4, AP::GPIO::PULL::DOWN);
  ap.gpio.set(MISO, AP::GPIO::FUNC::ALT4, AP::GPIO::PULL::OFF);

  auto aux_spi = ap.aux.spi(0);

  aux_spi.enable();
  aux_spi.mode(AP::SPI::MODE::_0);
  aux_spi.frequency(BAUD);

  std::vector<char> rx_buff{};

  rx_buff.reserve(msg.size());

  aux_spi.xfer(reinterpret_cast<char*>(rx_buff.data()),
               reinterpret_cast<char*>(msg.data()),
               msg.size());

  return std::string{ rx_buff.data() };
}

int
main()
{
  std::string msg{ "Hello, world!" };

  std::cout << "The message to send is: " << msg << "\n\n";

  std::cout << "Sending message through SPI using pigpio..." << "\n\n";

  std::string pigpio_response = data_loopback_using_pigpio(msg);

  std::cout << "The received string is: " << pigpio_response << "\n\n";

  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  std::cout << "Sending message through SPI using AikaPi..." << "\n\n";

  std::string aikapi_response = data_loopback_using_aikapi(msg);

  std::cout << "The received string is: " << aikapi_response << "\n\n";
}