#ifndef __TIME_H
#define __TIME_H

#include <chrono>
#include <ctime>

#include <iostream>
#include <iomanip>

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

// =====================================================
// Timer
//
// Header-only utility class for measuring:
//
// 1. Wall-clock elapsed time
// 2. CPU elapsed time
//
// =====================================================

#ifdef PROFILING

#define PROFILE(context, section) \
    AccumulatedScopedTimer timer##__LINE__(context, section)

#else

#define PROFILE(context, section)

#endif

class Timer {
public:

    using WallClock = std::chrono::steady_clock;
    using TimePoint = WallClock::time_point;
    using CpuClock = std::clock_t;

private:

    TimePoint _wallStart;
    CpuClock  _cpuStart;

public:

    // =====================================================
    // Construction
    // =====================================================

    Timer() {
        Reset();
    }

    // =====================================================
    // Reset
    // =====================================================

    void Reset() {

        _wallStart = WallClock::now();
        _cpuStart = std::clock();
    }

    // =====================================================
    // Wall-clock elapsed time
    // =====================================================

    double ElapsedSeconds() const {

        return std::chrono::duration<double>(
            WallClock::now() - _wallStart
        ).count();
    }

    double ElapsedMilliseconds() const {

        return std::chrono::duration<double, std::milli>(
            WallClock::now() - _wallStart
        ).count();
    }

    // =====================================================
    // CPU elapsed time
    // =====================================================

    double CpuElapsedSeconds() const {

        return static_cast<double>(
            std::clock() - _cpuStart
            ) / CLOCKS_PER_SEC;
    }
};

// =====================================================
// AccumulatedScopedTimer
//
// Accumulates profiling statistics grouped by:
//
// Context:
//   GLOBAL
//   NODE_15
//   ROOT
//
// Section:
//   Pricing
//   FixVariables
//   GenerateColumns
//
// Example:
//
// {
//     AccumulatedScopedTimer timer(
//         "GLOBAL",
//         "Pricing"
//     );
//
//     ...
// }
//
// =====================================================

class AccumulatedScopedTimer {
private:

    // =====================================================
    // Profile data
    // =====================================================

    struct ProfileData {

        long long _calls = 0;
        double _totalMs = 0.0;
        double _totalCpu = 0.0;

    };

    // =====================================================
    // Report line
    // =====================================================

    struct ReportLine {

        std::string _context;
        std::string _section;

        long long _calls = 0;
        double _totalMs = 0.0;
        double _avgMs = 0.0;
        double _totalCpu = 0.0;
        double _avgCpu = 0.0;
        double _percent = 0.0;
    };

    // =====================================================
    // Global profiling storage
    //
    // context -> section -> profile data
    // =====================================================

    inline static std::unordered_map< std::string, std::unordered_map<std::string, ProfileData> > _profiles;

    // =====================================================
    // Instance data
    // =====================================================

    std::string _context;
    std::string _section;

    Timer _timer;

public:

    // =====================================================
    // Construction
    // =====================================================

    AccumulatedScopedTimer(const std::string& context, const std::string& section) :
        _context(context),
        _section(section)
    {
    }

    // =====================================================
    // Destruction
    // =====================================================

    ~AccumulatedScopedTimer() {

        double elapsedMs = _timer.ElapsedMilliseconds();
        double elapsedCpu = _timer.CpuElapsedSeconds();
        auto& profile = _profiles[_context][_section];

        profile._calls++;
        profile._totalMs += elapsedMs;
        profile._totalCpu += elapsedCpu;
    }

    // =====================================================
    // Reset all profiling data
    // =====================================================

    static void Reset() {

        _profiles.clear();
    }

    // =====================================================
    // Reset profiling data from a specific context
    //
    // Example:
    // Reset("NODE_15");
    //
    // =====================================================

    static void Reset(const std::string& context) {
        _profiles.erase(context);
    }

    // =====================================================
    // Print all profiling data
    // =====================================================

    static void PrintReport() {
        PrintReport("");
    }

    // =====================================================
    // Print profiling data from a specific context
    //
    // Example:
    // PrintReport("GLOBAL");
    // PrintReport("NODE_15");
    //
    // =====================================================

    static void PrintReport(const std::string& context) {

        std::vector<ReportLine> report;

        double grandTotalMs = 0.0;

        // =============================================
        // Compute total time
        // =============================================

        for (const auto& [ctx, sections] : _profiles) {

            if (!context.empty() && ctx != context) {
                continue;
            }

            for (const auto& [section, data] : sections) {

                grandTotalMs += data._totalMs;
            }
        }

        // =============================================
        // Build report lines
        // =============================================

        for (const auto& [ctx, sections] : _profiles) {

            if (!context.empty() && ctx != context) {
                continue;
            }

            for (const auto& [section, data] : sections) {

                ReportLine line;

                line._context = ctx;
                line._section = section;
                line._calls = data._calls;
                line._totalMs = data._totalMs;
                line._avgMs = data._totalMs / static_cast<double>(data._calls);
                line._totalCpu = data._totalCpu;
                line._avgCpu = data._totalCpu / static_cast<double>(data._calls);
                line._percent = (grandTotalMs > 0.0) ? (100.0 * data._totalMs / grandTotalMs) : 0.0;

                report.push_back(line);
            }
        }

        // =============================================
        // Sort descending by total time
        // =============================================

        std::sort(report.begin(), report.end(),
            [](const ReportLine& a,
                const ReportLine& b) {
                    return a._totalMs > b._totalMs;
            }
        );

        // =============================================
        // Print report
        // =============================================

        std::cout << "\n";

        std::cout << "==================== PROFILING ====================\n";

        if (!context.empty()) {
            std::cout << "Context: " << context << "\n";
        }

        std::cout << "\n";

        std::cout << std::left << std::setw(20) << "Context" << std::setw(30)
            << "Section" << std::setw(10) << "Calls" << std::setw(15)
            << "Total(ms)" << std::setw(15) << "Avg(ms)" << std::setw(15)
            << "CPU(s)" << std::setw(10) << "%" << "\n";

        std::cout << std::string(115, '-') << "\n";

        for (const auto& line : report) {

            std::cout
                << std::left
                << std::setw(20) << line._context
                << std::setw(30) << line._section
                << std::setw(10) << line._calls
                << std::setw(15) << std::fixed
                << std::setprecision(3) << line._totalMs
                << std::setw(15) << line._avgMs
                << std::setw(15) << line._totalCpu
                << std::setw(10) << line._percent
                << "\n";
        }

        std::cout
            << std::string(115, '-') << "\n";

        std::cout
            << std::left << std::setw(60) << "TOTAL"
            << std::fixed << std::setprecision(3) << grandTotalMs << " ms\n";

        std::cout << "\n===================================================\n";
    }
};


#endif
