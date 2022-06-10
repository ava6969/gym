//
// Created by dewe on 9/1/21.
//

#include <numeric>
#include <iostream>
#include "custom/dm_lab/dm_lab.h"
#include "algorithm"
#include "chrono"
#include "opencv2/opencv.hpp"
#include "catch.hpp"



#define START_TIMER(title) auto t_start_xxx##title = std::chrono::high_resolution_clock().now();
#define END_TIMER(title) std::cout << #title " took " << std::chrono::duration<double, std::milli>( \
std::chrono::high_resolution_clock().now() - t_start_xxx##title).count() / 1000 \
<< " seconds to complete";

//int DMLabInit(){
//
//    gym::Lab l("lt_chasm",
//          {"RGB_INTERLEAVED"},
//          {},
//          "software");
//
//    gym::Lab l2("lt_chasm",
//               {"RGBD_INTERLEAVED"},
//               {},
//               "software");
//
//    gym::Lab l3("lt_chasm",
//                {"RGB_INTERLEAVED"},
//                {},
//                "hardware");
//}

struct Counter{
    decltype(std::chrono::high_resolution_clock::now()) start{};
    std::vector<float> deltas;

    void begin(){
        start = std::chrono::high_resolution_clock::now();
    }

    void end(){
        auto before = start;
        start = std::chrono::high_resolution_clock::now();
        deltas.emplace_back(std::chrono::duration<double, std::milli>(start - before).count() / 1000);
    }

    void printMean(std::string head="", std::string const& tail=""){
        std::cout << head << " took " <<
        std::accumulate(deltas.begin(), deltas.end(), 0.f)/float(deltas.size()) << " sec on average to complete\n";
    }

};

TEST_CASE("DMLabSinglePlayer"){

    gym::DMLabEnv::Option opt;
    opt.is_test = true;

   gym::DMLabEnv env1(opt);

    for(int i = 0; i < 1; i++){
        START_TIMER(reset)
        auto obs = env1.reset();
        END_TIMER(reset)

        Counter c;
        while (true){

            c.begin();
            auto resp = env1.step( env1.actionSpace()->sample<int>() );
            c.end();

            cv::imshow("demo", resp.observation);
            cv::waitKey(10);
            if(resp.done)
            {
                c.printMean("Step");
                break;
            }
        }
    }
}

TEST_CASE("DMLabSinglePlayerWrapper"){

    gym::DMLabEnv::Option opt;
    opt.is_test = true;

    gym::DMLabEnv2 env1( std::make_shared<gym::DMLabEnv>(opt));

    for(int i = 0; i < 1; i++){
        START_TIMER(reset)
        auto obs = env1.reset();
        END_TIMER(reset)

        int j =  0;
        Counter c;
        while (j < 50){

            c.begin();
            auto resp = env1.step( env1.actionSpace()->sample<int>() );
            c.end();

            auto img = (resp.observation["image"] * 255).permute({1, 2, 0}).to(c10::kByte);
            auto sz = img.sizes();
            cv::Mat view(sz[0], sz[1], CV_8UC3, img.data_ptr<unsigned char>());
            cv::imshow("demo", view );
            std::cout << "pastR: " << resp.observation["reward"] << "\tpastA: "<< resp.observation["action"];
            cv::waitKey(10);

            if(resp.done)
            {
                c.printMean("Step");
                break;
            }
            j++;
        }
    }
}

TEST_CASE("DMLabMultiPlayer"){

    gym::DMLabEnv::Option opt;
    opt.is_test = true;
    vector<gym::DMLabEnv*> envs = {    new gym::DMLabEnv(opt),
                                        new gym::DMLabEnv(opt),
                                        new gym::DMLabEnv(opt)};

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
                auto resp = env->step( env->actionSpace()->sample<int>());
                c.end();

                cv::imshow("demo" + std::to_string(t++), resp.observation);
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


}
//
//int main(){
//    return DMLabSinglePlayer();
//
//}