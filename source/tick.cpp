#include <typedefs.h>
#include <tick.h>

#include <fmt/format.h>
#include <ogc/lwp_watchdog.h>
#include <ogc/system.h>

using namespace poyo;

Tick::Tick() : running_(false), elapsed_tick(0.0), start_tick(0) {
}

void Tick::start() {
    if (!running_) {
        start_tick = gettick();
        running_ = true;
    }
}

void Tick::stop() {
    if (running_) {
        const auto end_tick = gettick();
        elapsed_tick += diff_ticks(start_tick, end_tick);
        running_ = false;
    }
}

void Tick::reset() {
    elapsed_tick = 0;
    running_ = false;
}

U64 Tick::elapsed() {
    if (running_) {
        stop();
        start();
    }
    return elapsed_tick;
}

void Tick::printElapsed() const {
    SYS_Report(fmt::format("Transferred Time: {:.4f} ms\n", elapsed_tick).c_str());
}

void Tick::stopAndPrint() {
    stop();
    printElapsed();
}

const U64& Tick::stopAndGetTick() {
    stop();
    return elapsed_tick;
}

U64 Tick::TickToMs(U64 ticks) {
    return ticks_to_millisecs(ticks);
}

float Tick::TickToMsfloat(U64 ticks) {
    auto result = ticks_to_microsecs(ticks);
    return static_cast<float>(result ) / 1000.0f; //to ms!!
}
