//
// Created by dewe on 10/22/21.
//

#ifndef SAMFRAMEWORK_GYM_DEF_H
#define SAMFRAMEWORK_GYM_DEF_H

#include "gym/env.h"
#include "transition_manager.h"
#include "gym/python_gym/python_env.h"
#include "gym/classic_control/cartpole.h"
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/mapped_region.hpp>

using namespace boost::interprocess;

template<class T>
using VecAllocator = allocator<T, managed_shared_memory::segment_manager>;
template<class T>
using SamVec = boost::interprocess::vector<T, VecAllocator<T>> ;

template<typename T>
void serializeTensorStruct(boost::interprocess::managed_shared_memory& segment,
                           torch::Tensor x, std::string const& key){

    const VecAllocator<T> alloc_inst_t = segment.get_segment_manager();
    const VecAllocator<int64_t> alloc_inst_int = segment.get_segment_manager();

    std::string shape = key + "::shape";
    std::string data = key + "::data";
    std::string length = key + "::length";

    auto* int_instance = segment.find_or_construct< SamVec<int64_t> >(shape.c_str())(alloc_inst_int);
    auto sz = x.sizes().vec();
    int_instance->insert(int_instance->end(), std::begin(sz), std::end(sz));

    auto* _instance = segment.find_or_construct< SamVec<T> >(data.c_str())(alloc_inst_t);
    x = x.view(-1);
    auto n = x.size(0);
    _instance->resize(n);

    std::memmove(_instance->data(), x.data_ptr<T>(), n * sizeof(T));

    segment.find_or_construct< uint32_t >(length.c_str())( n );
}


template<typename T>
torch::Tensor fromTensorStruct(managed_shared_memory& segment, std::string const& key){

    std::string shape = key + "::shape";
    std::string data = key + "::data";
    std::string length = key + "::length";

    auto shape_instance = segment.find< SamVec<int64_t> >(shape.c_str());
    auto data_instance = segment.find< SamVec<T> >(data.c_str());
    auto length_instance = segment.find< uint32_t >(length.c_str());

    auto n = *length_instance.first;
    c10::ScalarType dtype = torch::CppTypeToScalarType<T>();
    auto t = torch::empty( c10::IntArrayRef{n}, dtype);
    std::vector<int64_t> real_shape;

    std::memmove(t.data_ptr(), data_instance.first->data(), n * sizeof(T));
    auto sz = std::move(*shape_instance.first);
    real_shape.insert(real_shape.end(), std::begin(sz), std::end(sz));

    return t.view(sz);
}

template<typename T>
auto fromTensorStruct(TensorDict const& prev_state,
                      torch::Tensor const& action,
                      managed_shared_memory& segment,
                      std::vector<std::string> const& keys){

    TensorDict state;

    for(auto const& key: keys){
        state[key] = fromTensorStruct<float>(segment, key);
    }

    auto reward = *segment.template find<double>("reward").first;
    auto done = *segment.template find<bool>("done").first;

    return sam_rl::Transition{ prev_state,
                      state,
                      action,
                      torch::tensor(reward),
                      torch::tensor(int(done)) };
}

static  const int SHM_SIZE = 1026000;
//using INTERPROCESS_ENV_TYPE = gym::CartPoleEnv; Figure out how to use type to create env
static  const std::optional<gym::ArgMap> INTERPROCESS_ARG = std::nullopt;
static  const std::vector<std::string> INTER_PROC_OBS_KEY= {"observation"};
#endif //SAMFRAMEWORK_GYM_DEF_H
