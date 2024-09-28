#pragma once

//Typedefs for Poyo Engine - SEBASTIAN VALDES SANCHEZ (2022-2023)

#define LOCK_NOT_NECESSARY //Define added in order to no compile not necessary things for the Nintendo GameCube D: (27/09/2024)

#include <cstdint>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#ifndef LOCK_NOT_NECESSARY
#include <gtc/epsilon.hpp>
#include <gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/hash.hpp>
#include <gtx/quaternion.hpp>
#include <gtx/rotate_vector.hpp>

#define _USE_MATH_DEFINES
#include <math.h>
#endif

namespace poyo {
    using cbool = const bool;
    using cfloat = const float;
    using cdouble = const double;
    using Uchar = unsigned char;

    using SizeT = size_t;     using cSizeT = const size_t;
    
    using Int8  = int8_t;     using cInt8  = const int8_t;
    using Int16 = int16_t;    using cInt16 = const int16_t;
    using Int32 = int32_t;    using cInt32 = const int32_t;
    using Int64 = int64_t;    using cInt64 = const int64_t;

    using Uint8  = uint8_t;   using cUint8  = const uint8_t;
    using Uint16 = uint16_t;  using cUint16 = const uint16_t;
    using Uint32 = uint32_t;  using cUint32 = const uint32_t;
    using Uint64 = uint64_t;  using cUint64 = const uint64_t; 

    //*********** Vector Bool ***********//
    using BVec2 = glm::bvec2; using cBVec2 = const glm::bvec2;
    using BVec3 = glm::bvec3; using cBVec3 = const glm::bvec3;
    using BVec4 = glm::bvec4; using cBVec4 = const glm::bvec4;

    //*********** Vector Int ***********//
    using IVec2 = glm::ivec2; using cIVec2 = const glm::ivec2;
    using IVec3 = glm::ivec3; using cIVec3 = const glm::ivec3;
    using IVec4 = glm::ivec4; using cIVec4 = const glm::ivec4;
    
    //*********** Vector Unsigned ***********//
    using UVec2 = glm::uvec2; using cUVec2 = const glm::uvec2;
    using UVec3 = glm::uvec3; using cUVec3 = const glm::uvec3;
    using UVec4 = glm::uvec4; using cUVec4 = const glm::uvec4;

    //******** Vector Unsigned Short ********//
    using USVec2 = glm::lowp_uvec2; using cUSVec2 = const USVec2;
    using USVec3 = glm::lowp_uvec3; using cUSVec3 = const USVec3;
    using USVec4 = glm::lowp_uvec4; using cUSVec4 = const USVec4;

    //*********** Vector Float ***********//
    using FVec2 = glm::vec2;  using cFVec2 = const glm::vec2;
    using FVec3 = glm::vec3;  using cFVec3 = const glm::vec3;
    using FVec4 = glm::vec4;  using cFVec4 = const glm::vec4;

    //*********** Vector Double ***********//
#ifndef LOCK_NOT_NECESSARY
    using DVec2 = glm::dvec2; using cDVec2 = const glm::dvec2;
    using DVec3 = glm::dvec3; using cDVec3 = const glm::dvec3;
    using DVec4 = glm::dvec4; using cDVec4 = const glm::dvec4;
#endif

    //*********** Matrix Float ***********//
    using FMat2 = glm::mat2;  using cFMat2 = const glm::mat2;
    using FMat3 = glm::mat3;  using cFMat3 = const glm::mat3;
    using FMat4 = glm::mat4;  using cFMat4 = const glm::mat4;

#ifndef LOCK_NOT_NECESSARY
    //*********** Matrix Double ***********//
    using DMat2 = glm::dmat2; using cDMat2 = const glm::dmat2;
    using DMat3 = glm::dmat3; using cDMat3 = const glm::dmat3;
    using DMat4 = glm::dmat4; using cDMat4 = const glm::dmat4;

    //*********** Quaternion Float ***********//
    using Quat = glm::quat;   using cQuat = const glm::quat;
    
    //*********** Quaternion Double ***********//
    using DQuat = glm::dquat; using cDQuat = const glm::dquat;

    using EntityID = size_t;
#endif
}

//**** STD DEFINES ****// //use namespace poyo

//Smart Pointers
#include <memory>

namespace poyo {
    template <typename T> using UPtr  = std::unique_ptr<T>;
    template <typename T> using cUPtr = const std::unique_ptr<T>;
    
    template <typename T> using SPtr  = std::shared_ptr<T>;
    template <typename T> using cSPtr = const std::shared_ptr<T>;

    template <typename T> using WPtr  = std::weak_ptr<T>;
    template <typename T> using cWPtr = const std::weak_ptr<T>;
}

#ifndef LOCK_NOT_NECESSARY

//Reference Wrapper
#include <functional>

namespace poyo {
    template <typename T>
    using RefWrapper = std::reference_wrapper<T>;
}

#endif

// Sequences
#include <vector>
#include <deque>
#include <queue>
#include <stack>

namespace poyo {
    template <typename T> using Vector  = std::vector<T>;
    template <typename T> using cVector = const std::vector<T>;

    template <typename T> using Deque  = std::deque<T>;
    template <typename T> using cDeque = const std::deque<T>;

    template <typename T> using Queue  = std::queue<T>;
    template <typename T> using cQueue = const std::queue<T>;

    template <typename T> using Stack  = std::stack<T>;
    template <typename T> using cStack = const std::stack<T>;
}

// Associative Containers
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>

namespace poyo {
    // Non Multi Version
    
    template <typename Key, typename Value> using Map  = std::map<Key, Value>;
    template <typename Key, typename Value> using cMap = const std::map<Key, Value>;

    template <typename Key, typename Value> using HashMap  = std::unordered_map<Key, Value>;
    template <typename Key, typename Value> using cHashMap = const std::unordered_map<Key, Value>;

    template <typename T> using Set  = std::set<T>;
    template <typename T> using cSet = const std::set<T>;
    
    template <typename T> using HashSet  = std::unordered_set<T>;
    template <typename T> using cHashSet = const std::unordered_set<T>;

#ifndef LOCK_NOT_NECESSARY
    // Multi Version
    template <typename Key, typename Value> using MultiMap  = std::multimap<Key, Value>;
    template <typename Key, typename Value> using cMultiMap = const std::multimap<Key, Value>;

    template <typename Key, typename Value> using MultiHashMap  = std::unordered_multimap<Key, Value>;
    template <typename Key, typename Value> using cMultiHashMap = const std::unordered_multimap<Key, Value>;

    template <typename T> using MultiSet  = std::multiset<T>;
    template <typename T> using cMultiSet = const std::multiset<T>;

    template <typename T> using MultiHashSet  = std::unordered_multiset<T>;
    template <typename T> using cMultiHashSet = const std::unordered_multiset<T>;
#endif
}

// Arrays
#include <array>
#include <tuple>

namespace poyo {
    template <typename T, std::size_t N>
    using Array = std::array<T, N>;

    template <typename... Types>
    using Tuple = std::tuple<Types...>;
}

// Strings
#include <string>

namespace poyo {
    using String = std::string;          using cString = const std::string;        
    using StringView = std::string_view; using cStringView = const std::string_view;
}

#ifndef LOCK_NOT_NECESSARY

// Optionals
#include <optional>

namespace poyo {
    template <typename T>
    using Opt = std::optional<T>;
}

#endif

// Pairs
#include <utility>

namespace poyo {
    template <typename T1, typename T2>
    using Pair = std::pair<T1, T2>;
}

#ifndef LOCK_NOT_NECESSARY

// Futures
#include <future>
#include <thread>

namespace poyo {
    template <typename T>
    using Future = std::future<T>;

    template <typename T>
    using SFuture = std::shared_future<T>;

    using Thread = std::thread;
}

// Variants
#include <variant>

namespace poyo {
    template <typename... Types>
    using Variant = std::variant<Types...>;
}

// Initializer List
#include <initializer_list>

namespace poyo {
    template <typename T>
    using InitializerList = std::initializer_list<T>;
}

#include <typeindex>
namespace poyo {
    using TypeIndex = std::type_index;
}

#include <typeinfo>
namespace poyo {
    using TypeInfo = std::type_info;
}

#endif

// Creation Functions
namespace poyo {
    template <typename T, typename... Args>
    constexpr UPtr<T> MUnique(Args&&... args) {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
    constexpr SPtr<T> MShared(Args&&... args) {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

#ifndef LOCK_NOT_NECESSARY
    template <typename T, typename... Args>
    constexpr Future<T> MFuture(Args&&... args) {
        return std::async(std::launch::async, std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
    constexpr SFuture<T> MSharedFuture(Args&&... args) {
        return std::async(std::launch::async, std::forward<Args>(args)...).share();
    }

    template <typename... Args>
    constexpr auto MakeTuple(Args&&... args) {
        return Tuple<std::decay_t<Args>...>(std::forward<Args>(args)...);
    }
#endif

    template <typename T>
    constexpr String ToString(T value) {
        return std::to_string(value);
    }
}