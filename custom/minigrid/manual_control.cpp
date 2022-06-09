
//
// Created by dewe on 6/4/22.
//

#include "envs/door_key.h"
#include "envs/memory.h"
#include "wrappers.h"

void runView(){
    auto env = gym::FlatObsWrapper( std::make_shared<gym::MemoryS9>() );

    for(int i = 0;  i < 5; i++){
        auto x = env.reset();
        std::cout << x << "\n";
        bool done = false;
        while (not done){
            env.render();
            auto key = cv::waitKey(0) - 48;
            if(key < 7 and key > 0){
                auto response = env.step(key-1);
                done = response.done;
                std::cout << response.observation << "\n";
                std::cout << "reward --> " << response.reward << "\n";
            }

        }
    }
}

void runImageWrapper(){
    auto env = gym::ViewSizeWrapper2(
            std::make_shared<gym::RGBImgPartialObsWrapper>(
            std::make_unique<gym::DoorKeyEnv6x6>(), 28), 3);

    for(int i = 0;  i < 5; i++){
        auto x = env.reset();
        bool done = false;
        while (not done){
            cv::imshow("minigrid_view", x);
            cv::waitKey(1);
            env.render();
            auto key = cv::waitKey(0) - 48;
            if(key < 7 and key > 0){
                auto response = env.step(key-1);
                done = response.done;
                x = response.observation;
                std::cout << "reward --> " << response.reward << "\n";
            }

        }
    }
}

int main(){

    runImageWrapper();
    return 0;
}