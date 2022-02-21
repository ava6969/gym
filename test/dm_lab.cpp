//
// Created by dewe on 9/1/21.
//

#include <numeric>
#include "dm_lab/dm_lab.h"
#include "algorithm"
#include "chrono"

#define START_TIMER(title) auto t_start_xxx##title = std::chrono::high_resolution_clock().now();
#define END_TIMER(title) std::cout << #title " took " << std::chrono::duration<double, std::milli>( \
std::chrono::high_resolution_clock().now() - t_start_xxx##title).count() / 1000 \
<< " seconds to complete";

int DMLabInit(){

    gym::Lab l("lt_chasm",
          {"RGB_INTERLEAVED"},
          {},
          "software");

    gym::Lab l2("lt_chasm",
               {"RGBD_INTERLEAVED"},
               {},
               "software");

    gym::Lab l3("lt_chasm",
                {"RGB_INTERLEAVED"},
                {},
                "hardware");
}

struct Counter{
    decltype(std::chrono::high_resolution_clock().now()) start{};
    std::vector<float> deltas;

    void begin(){
        start = std::chrono::high_resolution_clock().now();
    }

    void end(){
        auto before = start;
        start = std::chrono::high_resolution_clock().now();
        deltas.emplace_back(std::chrono::duration<double, std::milli>(start - before).count() / 1000);
    }

    void printMean(std::string head="", std::string const& tail=""){
        std::cout << head << " took " <<
        std::accumulate(deltas.begin(), deltas.end(), 0.f)/float(deltas.size()) << " sec on average to complete\n";
    }

};

int DMLabSinglePlayer(){

    gym::ArgMap args;
    args = unordered_map<std::string, std::any>{};
    args->insert_or_assign("is_test", true);
    auto env1 = gym::CloneableEnv<gym::DMLabEnv>::make(args);

    for(int i = 0; i < 1; i++){
        START_TIMER(reset)
        auto obs = env1->reset();
        END_TIMER(reset)

        Counter c;
        while (true){

            c.begin();
            auto resp = env1->step( torch::tensor(env1->actionSpace()->sample<int>()) );
            c.end();

            auto screen2 = env1->render(gym::RenderType::HUMAN);
            cv::imshow("demo", screen2.value());
            cv::waitKey(1);
            if(resp.done)
            {
                c.printMean("Step");
                break;
            }
        }
    }

    return 0;
}

int DMLabMultiPlayer(){

    gym::ArgMap args;
    args = unordered_map<std::string, std::any>{};

    args->insert_or_assign("is_test", true);
    vector<gym::DMLabEnv*> envs = {    new gym::DMLabEnv(args),
                                        new gym::DMLabEnv(args),
                                        new gym::DMLabEnv(args)};

    for(int i = 0; i < 5; i++){

        for(auto& env : envs){
            START_TIMER(reset)
            auto obs = env->reset();
            END_TIMER(reset)
        }

        Counter c;
        while (true){
            int t =0 ;
            for(auto& env : envs) {

                c.begin();
                auto resp = env->step( torch::tensor(env->actionSpace()->sample<int>()));
                c.end();

                auto screen2 = env->render(gym::RenderType::HUMAN);
                cv::imshow("demo" + std::to_string(t++), screen2.value());
                cv::waitKey(1);

                if (resp.done) {
                    c.printMean("Step");
                    break;
                }
            }
        }
    }

    for(auto* env : envs)
        delete env;

    return 0;
}

int main(){

    return DMLabSinglePlayer();

}