
//
// Created by dewe on 6/4/22.
//

#include "envs/door_key.h"
#include "envs/memory.h"

int main(){

    auto env = gym::MemoryS13Random();

    for(int i = 0;  i < 5; i++){
        env.reset();
        bool done = false;
        while (not done){
            env.render(gym::RenderType::HUMAN);
            auto key = cv::waitKey(0) - 48;
            if(key < 7 and key > 0){
                auto response = env.step(key-1);
                done = response.done;
                std::cout << "reward --> " << response.reward << "\n";
            }

        }
    }


    return 0;
}