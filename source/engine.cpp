#include <typedefs.h>
#include <gctypes.h>

#include <engine.h>
#include <sstream>

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
    //auto deltaTimeMs =  e.lastTime_ - e.currentTime_;
    auto deltaTimeMs  = diff_usec(e.lastTime_, e.currentTime_);
    //e.deltaTime_ = static_cast<float>(ticks_to_millisecs(deltaTimeTicks)) / 1000.0f; //to seconds!!
    e.deltaTime_ = static_cast<float>(deltaTimeMs ) / 1000000.0f; //to seconds!!
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

#include <common.h>
#include <string.h>


std::string getOptimizationString(bool flag, const std::string& on_string, const std::string& off_string) {
    return flag ? on_string : off_string;
}

#define STRINGIZE(x) (#x)
#define EXPAND_IF_DEFINED(y) (!*STRINGIZE(y))
#define IS_MACRO_DEFINED_NOT_TO_ITSELF(y) strcmp(#y, STRINGIZE(y))


String Engine::generateOptimizationsString() {
    std::ostringstream oss;

    oss << CHUNK_RADIUS_STRING << "-";
    oss << OCCLUSION_STRING << "-";
    
    oss << getOptimizationString(IS_MACRO_DEFINED_NOT_TO_ITSELF(OPTIMIZATION_BATCHING), "BA_Y", "BA_N") << "-";
    oss << getOptimizationString(IS_MACRO_DEFINED_NOT_TO_ITSELF(OPTIMIZATION_DISPLAY_LIST), "LI_Y", "LI_N") << "-";

    oss << STRUCT_STRING << "-";

    oss << getOptimizationString(IS_MACRO_DEFINED_NOT_TO_ITSELF(OPTIMIZATION_MODEL_MATRIX), "M_Y", "M_N") << "-";
    oss << getOptimizationString(IS_MACRO_DEFINED_NOT_TO_ITSELF(OPTIMIZATION_VERTEX_MEMORY), "VTX_Y", "VTX_N") << "-";
    oss << getOptimizationString(IS_MACRO_DEFINED_NOT_TO_ITSELF(OPTIMIZATION_NO_LIGHTNING_DATA), "NLD_Y", "NLD_N") << "-";

    oss << CHUNK_RENDER_STRING;
    
    return oss.str();
}
