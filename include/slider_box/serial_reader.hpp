/**
 * @file
 * @brief Class for reading new-line terminated list of values from serial port.
 * @author Julian Viereck
 * @copyright Copyright (c) 2020, New York University & Max Planck Gesellschaft
 */
#pragma once

#include <unistd.h>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <vector>

#include <spdlog/spdlog.h>

#include "real_time_tools/thread.hpp"

namespace slider_box
{
class SerialReader
{
public:
    //! @brief Name of the spdlog logger used by the class.
    inline static const std::string LOGGER_NAME = "SerialReader";

    /**
     * @param serial_port The address of the serial port to use.  Set to ""
     *      (empty string) or "auto" to auto-detect the port.
     * @param num_values The number of values to read in each line.
     * @throws std::runtime_error if opening the port fails.
     */
    SerialReader(const std::string& serial_port, const int& num_values);

    ~SerialReader();

    /**
     * @brief Fills a vector with the latest values.
     * @param values Vector to place values into.
     * @return How often fill_vector was called without new data.
     */
    int fill_vector(std::vector<int>& values);

private:
    /**
     * @brief Open the specified port.
     *
     * Sets fd_.
     *
     * @param port Name of the serial port.
     *
     * @return True on success.
     */
    bool open_port(const std::string& port);

    /**
     * @brief This is the helper function used for spawning the real time
     * thread.
     *
     * @param instance_pointer is the current object in this case.
     * @return THREAD_FUNCTION_RETURN_TYPE depends on the current OS.
     */
    static THREAD_FUNCTION_RETURN_TYPE loop(void* instance_pointer)
    {
        static_cast<SerialReader*>(instance_pointer)->loop();
        return THREAD_FUNCTION_RETURN_VALUE;
    }

    /**
     * @brief This is the real time thread that streams the data to/from the
     * main board.
     */
    void loop();

    std::shared_ptr<spdlog::logger> log_;

    /**
     * @brief This boolean makes sure that the loop is stopped upon destruction
     * of this object.
     */
    bool is_loop_active_;

    /**
     * @brief This is the thread object that allow to spwan a real-time thread
     * or not dependening on the current OS.
     */
    real_time_tools::RealTimeThread rt_thread_;

    /**
     * @brief Holds the device serial port.
     */
    int fd_;

    /**
     * @brief If false, the communication is workinng as expected.
     */
    bool has_error_;

    /**
     * @brief If the communication is active.
     */
    bool is_active_;

    int new_data_counter_;

    int missed_data_counter_;

    /**
     * @brief Holds vector with the latest double values.
     */
    std::vector<int> latest_values_;

    /**
     * @brief mutex_ multithreading safety
     */
    std::mutex mutex_;
};

}  // namespace slider_box
