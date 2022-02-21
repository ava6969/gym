#include "python_gym/python_env.h"
#include "classic_control/cartpole.h"
#include "wrappers/vec_env/sync_env.h"
#include "gym_.h"
#include "wrappers/vec_env/vec_atari.h"
#include "wrappers/vec_env/vec_frame_stack.h"
#include "atari/atari_env.h"
#include "wrappers/atari_wrappers.h"

int main() {
    using namespace gym;


    cv::Mat x, y;
    std::vector<uint8_t> x1, y1;
    x1 = {1, 2, 4, 7};
    y1 = {3, 1, 1, 1};

    x = cv::Mat(2, 2 , CV_8UC(1), x1.data());
    y = cv::Mat(2, 2 , CV_8UC(1), y1.data());

    std::cout << cv::max(x, y) << "\n";
//    auto compute = [](std::string const& info, auto&& fnc){
//        auto start = std::chrono::high_resolution_clock::now();
//        fnc();
//        auto t = std::chrono::duration<double >(std::chrono::high_resolution_clock::now() - start).count();
//        std::cout << info  << "\t" << t << std::endl;
//        return t;
//    };


//    compute("static binding", [](){
//        torch::manual_seed(64);
//        auto atariProcessing = AtariWrapper::makeAtariWrapperT<true, true>( std::string("pong") );
//
//        for(int i =0; i < 5000; i++) {
//
//            atariProcessing.reset();
//            float t{};
//            int  j = 0;
//            while (j++ < 15) {
//                auto action = torch::tensor(atariProcessing.actionSpace()->sample<int>());
//
//                auto[obs, reward, done, info] = atariProcessing.step(action);
////
////            atariProcessing.render<0>(gym::RenderType::HUMAN);
//
//                t += reward;
//
//                if (done) {
////                std::cout << std::any_cast<int>(info.at("ale.lives")) << "\n";
////                std::cout << t << "\n";
//                    break;
//                }
//            }
//        }
//    });

//    compute("wrap binding", []() {
//        torch::manual_seed(64);
//        using WrappedT = gym::AtariWrapper;
//        using SyncT = gym::SyncVecEnv<WrappedT, false>;
//        using VNorm = gym::VecNormAndPermute<SyncT>;
//        using VStack = gym::VecFrameStack<VNorm, false>;
//
//        auto wrappedEnv = std::make_unique<gym::AtariWrapper>( std::make_unique<gym::AtariEnv<>>("pong") );
//
//        std::vector<std::unique_ptr<gym::AtariWrapper>> envs;
//        envs.emplace_back( std::move(wrappedEnv) );
//
//        std::unique_ptr<gym::VecEnv<false> > venv =
//                VStack::make(VNorm::make(SyncT::make(std::move(envs) ) ), 4 , "first");
//        auto s = venv->actionSpace();
//        auto n = s->as<Discrete>()->n;
//        for (int i = 0; i < 5000; i++) {
//
//            venv->reset();
//            float t{};
//            while (true){
//                auto action = torch::randint(n, {1, 1});
//                auto[obs, reward, done, info] = venv->step( action );
//
//                assert(obs.dim()==4);
//
//                t += reward.item<float>();
//
//                if (done.item<bool>()) {
////                    std::cout << std::any_cast<int>(info[0].at("ale.lives")) << "\n";
////                    std::cout << t << "\n";
//                    break;
//                }
//            }
//        }
//    });

    return 0;
}