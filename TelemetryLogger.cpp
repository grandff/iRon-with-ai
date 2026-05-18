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
    if (m_thread.joinable()) {
        m_thread.join();
    }
    // Final flush and close
    if (m_file.is_open()) {
        m_file.flush();
        m_file.close();
    }
}

void TelemetryLogger::loggingThread() {
    while (!m_stopRequested.load()) {
        // Only log when we are driving (or at least connected) and the session is valid
        ConnectionStatus status = ir_tick(); // This also updates the session data
        if (status == ConnectionStatus::DRIVING || status == ConnectionStatus::CONNECTED) {
            // Prepare a JSON line with selected telemetry data
            std::ostringstream jsonStream;
            jsonStream << std::fixed << std::setprecision(3);

            jsonStream << "{";
            jsonStream << "\"timestamp\":" << ir_SessionTime.getDouble() << ",";
            jsonStream << "\"lap\":" << ir_Lap.getInt() << ",";
            jsonStream << "\"lapCompleted\":" << ir_LapCompleted.getInt() << ",";
            jsonStream << "\"lapDistPct\":" << ir_LapDistPct.getDouble() << ",";
            jsonStream << "\"speed\":" << ir_Speed.getDouble() << ",";
            jsonStream << "\"rpm\":" << ir_RPM.getDouble() << ",";
            jsonStream << "\"gear\":" << ir_Gear.getInt() << ",";
            jsonStream << "\"throttle\":" << ir_Throttle.getDouble() << ",";
            jsonStream << "\"brake\":" << ir_Brake.getDouble() << ",";
            jsonStream << "\"steering\":" << ir_SteeringWheelAngle.getDouble() << ",";
            jsonStream << "\"fuelLevelPct\":" << ir_FuelLevelPct.getDouble() << ",";
            jsonStream << "\"fuelLevel\":" << ir_FuelLevel.getDouble() << ",";
            jsonStream << "\"waterTemp\":" << ir_WaterTemp.getDouble() << ",";
            jsonStream << "\"oilTemp\":" << ir_OilTemp.getDouble() << ",";
            jsonStream << "\"oilPressure\":" << ir_OilPress.getDouble() << ",";
            jsonStream << "\"voltage\":" << ir_Voltage.getDouble() << ",";
            jsonStream << "\"trackTemp\":" << ir_TrackTempCrew.getDouble() << ",";
            jsonStream << "\"airTemp\":" << ir_AirTemp.getDouble() << ",";
            jsonStream << "\"sessionState\":" << (int)ir_SessionState.getInt() << ",";
            jsonStream << "\"sessionFlags\":" << ir_SessionFlags.getInt() << ",";
            jsonStream << "\"driverCarIdx\":" << ir_session.driverCarIdx << ",";
            jsonStream << "\"isOnTrackCar\":" << ir_IsOnTrackCar.getBool() << ",";
            jsonStream << "\"isInGarage\":" << ir_IsInGarage.getBool() << "";

            // Optionally, add more data like tire temperatures, pressures, etc.
            // For brevity, we stop here.

            jsonStream << "}" << std::endl;

            if (m_file.is_open()) {
                m_file << jsonStream.str();
                // Check if it's time to flush (every 5 minutes)
                auto now = std::chrono::steady_clock::now();
                if (std::chrono::duration_cast<std::chrono::minutes>(now - m_lastFlush) >= m_flushInterval) {
                    m_file.flush();
                    m_lastFlush = now;
                }
            }
        }

        // Sleep a bit to avoid hogging the CPU; we want to log at about 60Hz but we can do less for the file.
        // The ir_tick() call already waits for new data (about 16ms). We'll just loop.
        // However, note that ir_tick() blocks until new data is available, so we don't need an extra sleep.
        // But to avoid spinning too fast when not connected, we can sleep a little.
        if (status != ConnectionStatus::DRIVING && status != ConnectionStatus::CONNECTED) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}