#include <typedefs.h>
#include <timer.h>

#include <fmt/format.h>
#include <ogc/lwp_watchdog.h>
#include <ogc/system.h>

using namespace poyo;

Timer::Timer() : running_(false), elapsed_time(0.0), start_time(0) {
}

void Timer::start() {
    if (!running_) {
        start_time = gettick();
        running_ = true;
    }
}

void Timer::stop() {
    if (running_) {
        const auto end_time = gettick();
        const auto duration = end_time - start_time;
        elapsed_time += diff_msec(start_time, end_time);
        running_ = false;
    }
}

void Timer::reset() {
    elapsed_time = 0.0;
    running_ = false;
}

U64 Timer::elapsed() {
    if (running_) {
        stop();
        start();
    }
    return elapsed_time;
}

void Timer::printElapsed() const {
    SYS_Report(fmt::format("Transferred Time: {:.4f} ms\n", elapsed_time).c_str());
}

void Timer::stopAndPrint() {
    stop();
    printElapsed();
}

const U64& Timer::stopAndGetTime() {
    stop();
    return elapsed_time;
}
