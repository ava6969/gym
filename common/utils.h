//
// Created by dewe on 8/22/21.
//

#pragma once

#include <map>
#include "any"
#include "unordered_map"
#include "string"
#include "../spaces/box.h"
#include "../spaces/discrete.h"
#include "../spaces/multi_discrete.h"
#include "../spaces/dict.h"
#include "memory"
#include "opencv4/opencv2/opencv.hpp"

#define __GYM_VECTOR_ARITHMETIC__(op)     static std::vector<float> operator op (std::vector<float> const& x, std::vector<float> const& y){ \
std::vector<float> result(x.size()); \
if(x.size() == y.size()){ \
    std::transform(x.begin(), x.end(), y.begin(), result.begin(), [](auto x1, auto y1){ return x1 op y1;}); \
    return result;\
}else{ \
    throw std::runtime_error("InvalidOperation [ op ] between vector of unequal length"); \
} \
}                   \
 static std::vector<float> operator op (std::vector<float> const& x, float y){ \
std::vector<float> result(x.size()); \
    std::transform(x.begin(), x.end(), result.begin(), [y](auto x1){ return x1 op y;}); \
    return result;\
}

#define __SINGLE_GYM_VECTOR_ARITHMETIC__(op)     static std::vector<float> operator op (std::vector<float> const& x){ \
std::vector<float> result(x.size()); \
std::transform(x.begin(), x.end(), result.begin(), [](auto x1){ return op x1;}); \
return result;\
} \

using std::string;
using std::unordered_map;

namespace cv{
    class Mat;
}

namespace gym{
    using namespace space;

    template <typename T> static inline
    std::unique_ptr<space::Space> makeBoxSpace( space::Shape const& shape){
        return std::make_unique<space::Box<T>>(shape);
    }

    template <typename T>  static inline
    std::unique_ptr<space::Space> makeBoxSpace(int64_t vector_length){
        return std::make_unique<space::Box<T>>(make_shape<T>(vector_length));
    }

    template <typename T>  static inline
    std::unique_ptr<space::Space> makeBoxSpace(std::vector<int64_t> shape){
        return std::make_unique<space::Box<T>>(make_shape<T>(shape));
    }

    template<typename T>  static inline
    std::unique_ptr<space::Space> makeBoxSpace(std::vector<T> lows,
                                               std::vector<T> highs,
                                               int64_t vector_length){

        std::vector<Range<T > > rangeVector(lows.size());
        std::transform(lows.begin(), lows.end(), highs.begin(),
                       rangeVector.begin(),[](auto l, auto h){
            return Range<T>{l, h};
        });
        return std::make_unique<space::Box<T>>(rangeVector,
                make_shape<T>(vector_length));
    }

    template<typename T>  static inline
    std::unique_ptr<space::Space> makeBoxSpace(T lows,
                                               T highs,
                                               std::vector<int64_t> const& sizes){

        return std::make_unique<space::Box<T>>(Range<T>{lows, highs}, make_shape<T>(sizes));
    }

    static inline
    std::unique_ptr<space::Space> makeDiscreteSpace( size_t n){
        return std::make_unique<space::Discrete>(n);
    }

    static inline
    std::unique_ptr<space::Space> makeMultiDiscreteSpace( std::vector<int> const& dims){
        return std::make_unique<space::MultiDiscrete>(
                std::vector<size_t>(dims.begin(), dims.end()));
    }

    static inline
    std::unique_ptr<space::Space> makeDictionarySpace( NamedSpaces spaces){
        return std::make_unique<space::ADict>(std::move(spaces));
    }

    __GYM_VECTOR_ARITHMETIC__(-)
    __GYM_VECTOR_ARITHMETIC__(+)
    __GYM_VECTOR_ARITHMETIC__(*)
    __GYM_VECTOR_ARITHMETIC__(/)

    __SINGLE_GYM_VECTOR_ARITHMETIC__(-)

    template<typename T> static inline
    std::vector<T> uniformRandom(T low, T high, size_t size,  std::mt19937& generator);

    template<> inline
    std::vector<int> uniformRandom(int low, int high, size_t size, std::mt19937& generator){
        std::uniform_int_distribution<int> uniform_distr(low, high);
        std::vector<int> result(size);
        std::generate(std::begin(result), std::end(result), [&]{ return uniform_distr(generator); });
        return result;
    }

    template<> inline
    std::vector<float> uniformRandom(float low,
                                     float high,
                                     size_t size,
                                     std::mt19937& generator){

        std::uniform_real_distribution<float> uniform_distr(low, high);
        std::vector<float> result(size);
        std::generate(std::begin(result), std::end(result), [&]{ return uniform_distr(generator); });

        return result;
    }
}
