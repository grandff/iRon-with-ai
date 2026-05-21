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

#pragma once

#include <string>
#include <fstream>
#include <thread>
#include <atomic>
#include <chrono>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "iracing.h"

struct TelemetryData {
    double timestamp;
    int lap;
    int lapCompleted;
    double lapDistPct;
    double speed;
    double rpm;
    int gear;
    double throttle;
    double brake;
    double steering;
    double fuelLevelPct;
    double fuelLevel;
    double waterTemp;
    double oilTemp;
    double oilPress;
    double voltage;
    double trackTemp;
    double airTemp;
    int sessionState;
    int sessionFlags;
    int driverCarIdx;
    bool isOnTrackCar;
    bool isInGarage;
};

class TelemetryLogger {
public:
    TelemetryLogger(const std::string& filename);
    ~TelemetryLogger();

    void start();
    void stop();
    void push(const TelemetryData& data);

private:
    void loggingThread();

    std::string m_filename;
    std::ofstream m_file;
    std::thread m_thread;
    std::atomic<bool> m_running{false};
    std::atomic<bool> m_stopRequested{false};
    std::chrono::steady_clock::time_point m_lastFlush;
    const std::chrono::minutes m_flushInterval{5};

    std::queue<TelemetryData> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_cv;
};