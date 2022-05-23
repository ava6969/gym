//
// Created by dewe on 8/22/21.
//

#pragma once

#include <map>
#include "opencv2/opencv.hpp"
#include "any"
#include "unordered_map"
#include "string"
#include "../spaces/box.h"
#include "../spaces/discrete.h"
#include "../spaces/multi_discrete.h"
#include "../spaces/dict.h"
#include "memory"


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

    template <class T>
    class is_dict {
        struct One { char a[1]; };
        struct Two { char a[2]; };

        template <class U>
        static One foo(typename U::key_type*);

        template <class U>
        static Two foo(...);

    public:
        static const bool value = sizeof(foo<T>(nullptr)) == sizeof(One);
    };

    struct FrameStackVisitor{
        std::mt19937 gen;

        FrameStackVisitor():
                gen(std::mt19937(std::random_device{}()))
        {}

        int operator()(int const& frameStack){
            return frameStack;
        }

        int operator()(std::array<int, 2> const& bound){
            std::uniform_int_distribution<int> dist(bound[0], bound[1]);
            return dist(gen);
        }
    };

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

    template<size_t N, typename T> static inline
    std::conditional_t< (N > 1), std::array<T, N>, T> uniformRandom( std::vector<T> const& list,
                                                                     std::mt19937  generator){
        std::array<T, N> result{};
        std::sample(list.begin(), list.end(), result.begin(), N, generator);

        if constexpr(N > 1){
            return result;
        }else{
            return result[0];
        }
    }

    template<class T, class K>
    int floor_div(T&& dividend, K&& divisor){
        return static_cast<int>( std::floor( std::forward<T>(dividend) / std::forward<T>(divisor)) );
    }

    template<bool single, typename Cont, typename Gen> static inline
    std::conditional_t< (not single), std::vector<typename Cont::value_type>, typename Cont::value_type> sample(
            size_t N, Cont const& list, Gen && generator){
        std::vector<typename Cont::value_type> result(N);
        std::sample(list.begin(), list.end(), result.begin(), N, std::forward<Gen>(generator) );

        if constexpr(not single){
            return result;
        }else{
            return result[0];
        }
    }

    template<typename Cont> static inline
    std::vector<typename Cont::value_type> choice(
            size_t N, Cont const& list){
        std::vector<typename Cont::value_type> result(N);
        std::generate(result.begin(), result.end(), [&list](){
            std::random_device r;                                       // 1
            std::seed_seq seed{r(), r()}; // 2
            return sample<true>(1, list, std::mt19937(seed));
        });

        return result;

    }

    template<typename Cont, typename T>
    inline bool contains(Cont&& x, T const& y){
        return std::find( std::begin(x), std::end(x), y) != std::end(x);
    }

}
