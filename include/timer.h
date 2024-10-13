#pragma once

namespace poyo {
    class Timer {
    public:
        Timer();

        void start();
        void stop();
        void reset();
        
        U64 elapsed();

        void printElapsed() const;
        void stopAndPrint();
        const U64& stopAndGetTime();

    private:
        bool running_;
        U64 elapsed_time;
        U32 start_time;
    };
}


