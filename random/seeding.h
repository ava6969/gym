#pragma once
//
// Created by dewe on 6/30/22.
//

#include "mtrand/mtrand.h"
#include <linux/random.h>
#include <syscall.h>
#include "string"
#include <errno.h>
#include "sha512.h"
#include "tuple"
#include "span"


static auto urandom(int size, std::string& myRandomData){
    myRandomData.resize(size);
    return syscall(SYS_getrandom, myRandomData.data(), myRandomData.size(), GRND_NONBLOCK);
}

static std::vector<unsigned long> _int_list_from_bigint(uint64_t bigint){

    if(bigint < 0){
        throw std::invalid_argument("Seed must be a non-negative integer, not " + std::to_string(bigint));
    }else if(bigint == 0)
        return {0};

    std::vector<unsigned long> ints;
    ints.reserve(5);
    unsigned long mod;
    unsigned long divisor = std::pow(2, 32);
    while (bigint > 0){
        std::tie(bigint, mod) = std::make_pair(bigint/divisor, bigint%divisor);
        ints.push_back(mod);
    }

    return ints;
}

static uint64_t _bigint_from_bytes(std::vector<unsigned char> bytes){
    constexpr int sizeof_int = 4;
    auto padding = sizeof_int - bytes.size() % sizeof_int;
    bytes.insert(bytes.end(), padding, 0);

    auto int_count = int(bytes.size() / sizeof_int);

    uint64_t accum = 0;
    for(int i = 0; i < int_count; i++){
        uint32_t val;
        auto start = i*sizeof_int;
        auto sub_byte = std::vector<unsigned char >( bytes.begin() + start, bytes.begin() + start + sizeof_int);
        memcpy(&val, sub_byte.data(), sizeof_int);
        accum += (uint64_t)( std::pow(2, (sizeof_int * 8 * i) ) * val);
    }
    return accum;
}

static long create_seed( std::optional<int32_t> const& a, char max_bytes=8){
    if(not a){
        std::string bytes;
        auto err = urandom(max_bytes, bytes);
        return _bigint_from_bytes( std::vector<unsigned char>(bytes.begin(), bytes.end()) );
    }else{
        return (*a % int(std::pow(2, 8*max_bytes)));
    }
}

static uint64_t hash_seed(std::optional<int32_t> seed, char max_bytes=8){

    if(not seed){
        seed = create_seed(seed, max_bytes);
    }
    std::vector<unsigned char> hash;
    sha512( std::to_string(*seed), hash );
    return _bigint_from_bytes( std::vector<unsigned char>(hash.begin() , hash.begin()+max_bytes) );
}

static auto np_random( std::optional<int32_t> seed){
    if(seed and seed < 0){
        throw std::invalid_argument("Seed must be a non-negative integer or omitted, not " + std::to_string(*seed));
    }

    auto _seed = create_seed(seed);

    np::RandomState rng( _int_list_from_bigint(hash_seed(_seed)) );
    return std::pair(rng, _seed);

}

