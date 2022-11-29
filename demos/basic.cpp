#include <signal.h>
#include <unistd.h>
#include <cstdlib>
#include <iostream>

#include "real_time_tools/timer.hpp"

#include "slider_box/serial_reader.hpp"

using namespace std;

bool keep_running = true;

// Define the function to be called when ctrl-c (SIGINT) is sent to process.
void signal_callback_handler(int signum)
{
    // Terminate program
    keep_running = false;
}

/**
 * @brief This example demonstrates configuring a serial stream and
 *        reading serial stream data.
 */
int main(int argc, char** argv)
{
    if (argc < 2)
    {
        rt_printf(
            "Usage: demo_arduino_slider serial_port. (serial_port like "
            "/dev/ttyACM0)\n");
        return -1;
    }

    // Register ctrl-c handler.
    signal(SIGINT, signal_callback_handler);

    std::vector<int> values;
    slider_box::SerialReader serial_reader(argv[1], 5);

    while (keep_running)
    {
        serial_reader.fill_vector(values);
        for (int i = 0; i < 5; i++)
        {
            std::cout << values[i] << " ";
        }
        std::cout << std::endl;

        real_time_tools::Timer::sleep_sec(0.1);
    }

    return 0;
}
