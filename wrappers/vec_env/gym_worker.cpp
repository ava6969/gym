//
// Created by dewe on 10/22/21.
//

#include "sam_env.h"
#define BOOST_DATE_TIME_NO_LIB
#include <boost/process.hpp>

#include "gym_def.h"

int main(int argc, char** argv){

    if(argc < 3)
        std::cerr << "[USAGE] ./gym_worker rank_id:int env_id:string\n";
        return 1;
    }

    auto rank = std::stoi(argv[1]);
    auto env_id = argv[2];

    auto recv_id = "env_" + std::to_string(rank);
    auto data = managed_shared_memory{open_only, "data"};
    shared_memory_object::remove(recv_id.c_str());
    auto msm = managed_shared_memory{open_or_create, recv_id.c_str(), SHM_SIZE};
    printf("C++ Program - Successfully Opened Shared Memory: %s\n", recv_id.c_str());
    std::unique_ptr<sam_rl::SAMEnv> env;

    msm.construct<double>("reward")(0);
    msm.construct<bool>("done")(false);
    msm.construct<double>("return")(0);
    msm.construct<uint64_t>("steps")(0);

    bool close{false};
    while(not close){

        auto cmd = msm.find<uint8_t>("cmd");
        if(cmd.first){
            switch(*cmd.first){
                case 0:{
                    auto const& action = *msm.find<long>("action").first;
                    auto&& response = env->step(torch::tensor(action));
                    auto& transition = std::get<0>(response);

                    for(auto&  entry : transition.nextState){
                        serializeTensorStruct<float>(msm, std::move(entry.second), entry.first);
                    }

                    *msm.find<double>("reward").first = transition.reward.item<double>();
                    *msm.find<bool>("done").first = transition.done.item<bool>();
                    *msm.find<double>("return").first = std::get<1>(response).front();
                    *msm.find<uint64_t>("steps").first = std::get<2>(response).front();
                    break;
                }
                case 1:{
                    auto transition = env->reset();
                    for(auto&  entry : transition.nextState){
                        serializeTensorStruct<float>(msm, std::move(entry.second), entry.first);
                    }
                    break;
                }
                case 2:{
                    if(not INTERPROCESS_ARG){
                        env = std::make_unique<sam_rl::SAMEnv>([](){
//                            return std::make_unique<INTERPROCESS_ENV_TYPE>(); create from type
                        }, c10::kCPU, true);
                    }else{
//                        env = std::make_unique<sam_rl::SAMEnv>([&](){
//                            return std::make_unique<INTERPROCESS_ENV_TYPE>(INTERPROCESS_ARG);
//                        }, device, true);
                    }
                    break;
                }case 3:{
                    close = true;
                    break;
                }
                default:{
                    throw std::runtime_error("Invalid Command Type: Valid Commands are"
                                             " [STEP(0), RESET(1), INIT(2), CLOSE(3)]");
                }
            }
            (*data.find<uint32_t>("completed").first)++;
            msm.destroy<uint8_t>("cmd");
        }
    }

    std::cout << "closed environment successfully\n";
    shared_memory_object::remove(recv_id.c_str());
    return 0;
}