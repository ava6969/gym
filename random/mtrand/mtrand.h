#pragma once
//
// Created by dewe on 6/30/22.
//

#include "cmath"
#include "randomkit.h"
#include "distributions.h"
#include "initarray.h"
#include "optional"
#include "vector"
#include "stdexcept"
#include "helper.h"
#include "tuple"


namespace np{

    template<class T, class K=T, class ... Args>
    using RandIntFncT = std::vector<T>(*)(T, T, rk_state&, unsigned long, Args ...);

    template<class T, class K=T, class ... Args>
    using RandIntReturn =  std::tuple<int64_t, int64_t, RandIntFncT<K, K ,Args ...>>;


    static std::vector<bool> _rand_bool(uint8_t low, uint8_t high, rk_state& state, unsigned long sizes=0){
        auto rng = static_cast<bool>( high - low );
        auto off = static_cast<bool>(low);
        if(sizes == 0){
            bool buf;
            rk_random_bool(off, rng, 1, &buf, &state);
            return std::vector<bool>{ buf };
        }else{
            std::vector<uint8_t> out(sizes);
            rk_random_uint8(off, rng, sizes, out.data(), &state);
            return {out.begin(), out.end()};
        }
    }

    template<class T, class RandomFnc>
    std::vector<T> _rand_int(T low, T high, rk_state& state, unsigned long sizes=0, RandomFnc  randomFnc={}){

        using unsignedT = std::make_unsigned_t<T>;

        auto rng = static_cast<unsignedT>( high - low );
        auto off = static_cast<unsignedT>(low);
        if(sizes == 0){
            unsignedT buf;
            randomFnc(off, rng, 1, &buf, &state);
            return std::vector<T>{ static_cast<T>(buf)};
        }else{
            std::vector<unsignedT> out(sizes);
            randomFnc(off, rng, sizes, out.data(), &state);
            return std::vector<T>(out.begin(), out.end());
        }
    }

    template<class Ts>
    std::pair<uint64_t , uint64_t > _randint_type(){
        if constexpr( std::same_as<Ts, bool> ){
            return std::make_pair( 0UL, 2UL  );
        }
        else if constexpr( std::same_as<Ts, uint8_t> ){
            return std::make_pair( -std::pow(2, 7), std::pow(2, 7) );
        }
        else if constexpr( std::same_as<Ts, int16_t> ){
            return std::make_pair( -std::pow(2, 7), std::pow(2, 7) );
        }
        else if constexpr( std::same_as<Ts, int32_t> ){
            return std::make_pair( -std::pow(2, 31), std::pow(2, 31) );
        }
        else if constexpr( std::same_as<Ts, int64_t> ){
            return std::make_pair( -std::pow(2, 63), std::pow(2, 63) );
        }
        else if constexpr( std::same_as<Ts, uint8_t> ){
            return std::make_pair( 0, std::pow(2, 8) );
        }
        else if constexpr( std::same_as<Ts, uint16_t> ){
            return std::make_pair( 0, std::pow(2, 16) );
        }
        else if constexpr( std::same_as<Ts, uint32_t> ){
            return std::make_pair(0, std::pow(2, 32) );
        }
        else if constexpr( std::same_as<Ts, uint64_t> ){
            return std::make_pair( 0, std::pow(2, 64) );
        }
    }

    template<class Ts>
    std::vector<Ts> randfnc(Ts low, Ts high, rk_state& state, unsigned long sizes=0){
        if constexpr( std::same_as<Ts, bool> ){
            return _rand_bool(low, high, state, sizes);
        }
        else if constexpr( std::same_as<Ts, uint8_t> or std::same_as<Ts, int8_t>  ){
            return _rand_int(low, high, state, sizes, rk_random_uint8);
        }
        else if constexpr( std::same_as<Ts, int16_t> or std::same_as<Ts, int16_t> ){
            return _rand_int(low, high, state, sizes, rk_random_uint16);
        }
        else if constexpr( std::same_as<Ts, int32_t> or std::same_as<Ts, int32_t>  ){
            return _rand_int(low, high, state, sizes, rk_random_uint32);
        }
        else if constexpr( std::same_as<Ts, int64_t> or std::same_as<Ts, int64_t> ){
            return _rand_int(low, high, state, sizes, rk_random_uint64);
        }
        static_assert(true, "_randint_fnc only supports integral literal types");
    }

    class RandomState {

        rk_state internal_state{};

    public:
        RandomState()=default;

        explicit RandomState(std::optional<unsigned long> const& seed){
            this->seed(seed);
        }

        explicit RandomState(std::vector<unsigned long> && seed){
            this->seed(seed);
        }

        void seed(std::optional<unsigned long > const& seed){
            if(not seed.has_value()){
                auto error_code = rk_randomseed(&internal_state);
            }else{
                rk_seed(seed.value(), &internal_state);
            }
        }

        void seed(std::vector<unsigned long>& seed){
            if(seed.empty()){
                rk_randomseed(&internal_state);
            }else{
                auto l = static_cast<npy_intp>(seed.size());
                init_by_array(&internal_state, seed.data(), l);
            }
        }

        template<class ... Sizes>
        std::conditional_t< sizeof...(Sizes) == 0, double, std::vector<double>>
        uniform(float low=0.f, float high=1.f, Sizes ... sizes ){
            auto fscale = high - low;
            if( std::isinf(fscale)){
                throw std::overflow_error("Range exceeds valid bounds");
            }
            if constexpr(sizeof...(sizes) == 0){
                return cont2_array_sc(internal_state, rk_uniform, low, fscale );
            }else{
                return cont2_array_sc(internal_state, rk_uniform, sizes ..., low, fscale);
            }

        }

        template<class T>
        std::vector<T> randint(size_t sizes, T low, T high){
            return randfnc(low, high-1, internal_state, sizes);
        }

        template<class T>
        std::vector<T>  randint(size_t sizes, T high=0.f){
            return randint(sizes, 0, high);
        }

        template<class T>
        T randint(T low, T high){
            return randfnc(low, high-1, internal_state)[0];
        }

        template<class T>
        T  randint(T high=0.f){
            return randint(0, high);
        }

    };

}

