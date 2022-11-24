/**
 * @file
 * @author Julian Viereck and others
 * @copyright Copyright (c) 2020, New York University & Max Planck Gesellschaft
 */
#include "slider_box/serial_reader.hpp"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include <fmt/format.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace slider_box
{
SerialReader::SerialReader(const std::string &serial_port,
                           const int &num_values)
{
    // initialise logger and set level
    log_ = spdlog::get(LOGGER_NAME);
    if (!log_)
    {
        log_ = spdlog::stderr_color_mt(LOGGER_NAME);
        log_->set_level(spdlog::level::debug);
    }

    // open the serial port
    if (serial_port.empty() || serial_port == "auto")
    {
        // try to auto-detect port by checking a number of potential candidates
        bool success = false;
        for (std::string port : {"/dev/ttyACM", "/dev/ttyACM0", "/dev/ttyACM1"})
        {
            log_->debug("Try to open serial port '{}'", port);
            if (open_port(port))
            {
                success = true;
                break;
            }
        }
        if (!success)
        {
            throw std::runtime_error(
                "Unable to auto-detect serial reader port.");
        }
    }
    else
    {
        // if an explicit port is specified by the user, use that one
        if (!open_port(serial_port))
        {
            throw std::runtime_error(
                fmt::format("Unable to open serial reader on port '{}'. "
                            "Set the port to 'auto' to try auto-detection.",
                            serial_port));
        }
    }

    struct termios options;

    fcntl(fd_, F_SETFL, FNDELAY);  // Open the device in nonblocking mode

    // Set parameters
    tcgetattr(fd_, &options);          // Get the current options of the port
    bzero(&options, sizeof(options));  // Clear all the options
    speed_t Speed = B115200;
    cfsetispeed(&options, Speed);  // Set the baud rate at 115200 bauds
    cfsetospeed(&options, Speed);
    options.c_cflag |=
        (CLOCAL | CREAD |
         CS8);  // Configure the device : 8 bits, no parity, no control
    options.c_iflag |= (IGNPAR | IGNBRK);
    options.c_cc[VTIME] = 0;  // Timer unused
    options.c_cc[VMIN] = 0;   // At least on character before satisfy reading
    tcsetattr(fd_, TCSANOW, &options);  // Activate the settings

    latest_values_.resize(num_values);

    has_error_ = false;
    is_active_ = false;
    is_loop_active_ = true;
    // Launch the main processing loop.
    rt_thread_.create_realtime_thread(&SerialReader::loop, this);
}

bool SerialReader::open_port(const std::string &port)
{
    fd_ = open(port.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd_ != -1)
    {
        log_->info("Opened serial port {}", port);
    }
    return fd_ != -1;
}

void SerialReader::loop()
{
    int byte_read;
    const int buffer_size = 128;
    char buffer[buffer_size];
    char line[buffer_size];
    int line_index = 0;

    is_active_ = true;
    while (is_loop_active_)
    {
        int byte_consumed = 0;
        byte_read = read(fd_, buffer, buffer_size);
        while (byte_consumed < byte_read)
        {
            line[line_index++] = buffer[byte_consumed++];
            if (buffer[byte_consumed - 1] == '\n')
            {
                // Ignore the "\r\n" in the string.
                line[line_index - 1] = '\0';
                line[line_index - 2] = '\0';
                line_index -= 2;

                // Read the actual numbers from the line.
                int bytes_scanned_total = 0;
                int bytes_scanned;
                int number;
                mutex_.lock();
                for (std::size_t i = 0; i < latest_values_.size(); i++)
                {
                    sscanf(line + bytes_scanned_total,
                           "%d %n",
                           &number,
                           &bytes_scanned);
                    if (bytes_scanned_total >= line_index)
                    {
                        break;
                    }
                    bytes_scanned_total += bytes_scanned;
                    latest_values_[i] = number;
                }
                new_data_counter_ += 1;
                mutex_.unlock();

                line_index = 0;
            }
        }
        usleep(100);
    }
}

SerialReader::~SerialReader()
{
    is_loop_active_ = false;
    rt_thread_.join();
    close(fd_);
}

int SerialReader::fill_vector(std::vector<int> &values)
{
    mutex_.lock();
    if (new_data_counter_ == 0)
    {
        missed_data_counter_ += 1;
    }
    else
    {
        missed_data_counter_ = 0;
    }
    new_data_counter_ = 0;
    values = latest_values_;
    mutex_.unlock();

    return missed_data_counter_;
}

}  // namespace slider_box
