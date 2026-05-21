/*
MIT License

Copyright (c) 2024 Hermes Agent

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "TelemetryLogger.h"
#include "util.h"
#include <iostream>
#include <iomanip>
#include <sstream>

TelemetryLogger::TelemetryLogger(const std::string& filename)
    : m_filename(filename) {
    // Constructor does not open the file yet; that happens in start()
}

TelemetryLogger::~TelemetryLogger() {
    stop(); // Ensure the thread is stopped and file closed
}

void TelemetryLogger::start() {
    if (m_running.exchange(true)) {
        // Already running
        return;
    }
    m_stopRequested = false;
    m_file.open(m_filename, std::ios::app); // Open in append mode
    if (!m_file.is_open()) {
        std::cerr << "Failed to open telemetry log file: " << m_filename << std::endl;
        m_running = false;
        return;
    }
    m_thread = std::thread(&TelemetryLogger::loggingThread, this);
    m_lastFlush = std::chrono::steady_clock::now();
}

void TelemetryLogger::stop() {
    if (!m_running.exchange(false)) {
        // Not running
        return;
    }
    m_stopRequested = true;
    m_cv.notify_all();
    if (m_thread.joinable()) {
        m_thread.join();
    }
    // Final flush and close
    if (m_file.is_open()) {
        m_file.flush();
        m_file.close();
    }
}

void TelemetryLogger::push(const TelemetryData& data) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push(data);
    m_cv.notify_one();
}

void TelemetryLogger::loggingThread() {
    while (true) {
        std::vector<TelemetryData> localQueue;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(lock, [this] { return !m_queue.empty() || m_stopRequested.load(); });
            
            if (m_stopRequested.load() && m_queue.empty()) {
                break;
            }
            
            while (!m_queue.empty()) {
                localQueue.push_back(m_queue.front());
                m_queue.pop();
            }
        }

        for (const auto& data : localQueue) {
            // Prepare a JSON line with selected telemetry data
            std::ostringstream jsonStream;
            jsonStream << std::fixed << std::setprecision(3);

            jsonStream << "{";
            jsonStream << "\"timestamp\":" << data.timestamp << ",";
            jsonStream << "\"lap\":" << data.lap << ",";
            jsonStream << "\"lapCompleted\":" << data.lapCompleted << ",";
            jsonStream << "\"lapDistPct\":" << data.lapDistPct << ",";
            jsonStream << "\"speed\":" << data.speed << ",";
            jsonStream << "\"rpm\":" << data.rpm << ",";
            jsonStream << "\"gear\":" << data.gear << ",";
            jsonStream << "\"throttle\":" << data.throttle << ",";
            jsonStream << "\"brake\":" << data.brake << ",";
            jsonStream << "\"steering\":" << data.steering << ",";
            jsonStream << "\"fuelLevelPct\":" << data.fuelLevelPct << ",";
            jsonStream << "\"fuelLevel\":" << data.fuelLevel << ",";
            jsonStream << "\"waterTemp\":" << data.waterTemp << ",";
            jsonStream << "\"oilTemp\":" << data.oilTemp << ",";
            jsonStream << "\"oilPressure\":" << data.oilPress << ",";
            jsonStream << "\"voltage\":" << data.voltage << ",";
            jsonStream << "\"trackTemp\":" << data.trackTemp << ",";
            jsonStream << "\"airTemp\":" << data.airTemp << ",";
            jsonStream << "\"sessionState\":" << data.sessionState << ",";
            jsonStream << "\"sessionFlags\":" << data.sessionFlags << ",";
            jsonStream << "\"driverCarIdx\":" << data.driverCarIdx << ",";
            jsonStream << "\"isOnTrackCar\":" << data.isOnTrackCar << ",";
            jsonStream << "\"isInGarage\":" << data.isInGarage << "";
            jsonStream << "}" << std::endl;

            if (m_file.is_open()) {
                m_file << jsonStream.str();
            }
        }

        if (m_file.is_open()) {
            // Check if it's time to flush (every 5 minutes)
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::minutes>(now - m_lastFlush) >= m_flushInterval) {
                m_file.flush();
                m_lastFlush = now;
            }
        }
    }
}