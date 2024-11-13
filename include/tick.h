#pragma once

namespace poyo {
    class Tick {
    public:
        Tick();

        void start();
        void stop();
        void reset();
        
        U64 elapsed();

        void printElapsed() const;
        void stopAndPrint();
        const U64& stopAndGetTick();

        U64 static TickToMs(U64 ticks);
        static float TickToMsfloat(U64 ticks);

    private:
        bool running_;
        U64 elapsed_tick;
        U32 start_tick;
    };
}