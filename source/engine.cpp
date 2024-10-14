#include <typedefs.h>
#include <gctypes.h>

#include <engine.h>

#include <ogc/lwp_watchdog.h>

using namespace poyo;

Engine::Engine(){
    deltaTime_ = 0.0f;
    currentTime_ = 0;
    lastTime_ = 0;
}

Engine& Engine::get(){
    static Engine instance;
    return instance;
}

void Engine::UpdateEngine() {
    auto& e = get();
    e.lastTime_ = e.currentTime_;
    e.currentTime_ = gettick();
    //u64 deltaTimeTicks = e.currentTime_ - e.lastTime_;
    auto deltaTimeMs  = diff_msec(e.lastTime_, e.currentTime_);
    //e.deltaTime_ = static_cast<float>(ticks_to_millisecs(deltaTimeTicks)) / 1000.0f; //to seconds!!
    e.deltaTime_ = static_cast<float>(deltaTimeMs ) / 1000.0f; //to seconds!!
    e.deltaTime_ = glm::clamp(e.deltaTime_, 0.0f, 0.1f);
}

cfloat Engine::getDeltaTime() {
    return get().deltaTime_;
}

u64 Engine::getCurrentTime() {
    return get().currentTime_;
}

u64 Engine::getLastTime() {
    return get().lastTime_;
}
