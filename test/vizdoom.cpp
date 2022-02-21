//
// Created by dewe on 9/25/21.
//

#include "vizdoom/viz_doom.h"
#include "python_gym/python_env.h"

void test_viz_doom(){
    using namespace gym;
    unordered_map<std::string, std::any> arg = {};
    arg["level"] = 7;
    arg["scenario_path"] = std::string("/home/dewe/CLionProjects/SAMFramework/gym_environments/third_party/linux/ViZDoom/scenarios");
    arg["screen_size"] = 64L;
    VizDoomEnv env(arg);

    auto state = env.reset();

    std::cout << state << "\n";

    auto done = false;
    auto prev = cv::Mat();
    while(not done){
        auto[_state, reward, _done, _] = env.step(torch::randint(2, {1}));
        auto img = _state["image"].clone();
        std::cout << img << "\n";
        auto cv_img = cv::Mat(img.size(1), img.size(2), CV_8UC3, img.permute({2, 0, 1}).data_ptr<uint8_t>());
        cv::cvtColor(cv_img, cv_img, cv::COLOR_BGR2RGB);
        cv::imshow("vizdoom", cv_img);
        cv::waitKey(0);


        done  = _done;
    }

}

void test_python(){

    using namespace gym;
    PythonEnv env(unordered_map<std::string, std::any>{{"env",
                                                        std::string("CarRacing-v0")}});
    PythonEnv env2(unordered_map<std::string, std::any>{{"env",
                                                        std::string("CarRacing-v0")}});
    auto obs = env.reset();
    obs = env2.reset();
    auto res = env.render();
    env2.render();
    while(true){
        auto [o, r, d, _] = env.step(torch::cat({
            torch::rand({1}) - 1, torch::rand({1}), torch::rand({1})
        }));

        env2.step(torch::cat({
                                     torch::rand({1}) - 1, torch::rand({1}), torch::rand({1})
                             }));
        res = env.render();
        env2.render();
        if(d){
            break;
        }

    }

}

int main(){

    test_python();
}
