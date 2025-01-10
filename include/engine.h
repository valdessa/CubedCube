#ifndef INCLUDE_ENGINE_H_
#define INCLUDE_ENGINE_H_ 1

namespace poyo {
    class Engine {
     public:
        //Copy Constructor
        Engine(const Engine&) = delete;
        Engine& operator=(const Engine&) = delete;
		
        //Movement Constructor
        Engine(Engine&&) = delete;
        Engine& operator=(Engine&&) = delete;

        static void UpdateEngine();

        static cfloat getDeltaTime();
        static u64 getCurrentTime();
        static u64 getLastTime();

        static String generateOptimizationsString();
        
     private:
        Engine();
        static Engine& get();

        float deltaTime_;
        u64 lastTime_ = 0;
        u64 currentTime_ = 0;
    };
}

#endif

