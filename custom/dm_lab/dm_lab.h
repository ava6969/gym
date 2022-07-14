//
// Created by dewe on 9/2/21.
//

#ifndef GYMENV_DM_LAB_H
#define GYMENV_DM_LAB_H

#include "base_lab.h"
#include "opencv2/opencv.hpp"
#include "env.h"
#include <openssl/md5.h>
#include "spaces/dict.h"
#include "action_mapping.h"
#include <utility>
#include "torch/torch.h"
#include "common/wrapper.h"


namespace gym {

    struct LevelCache : LevelFetch{

        explicit LevelCache( std::filesystem::path const& cache_dir):path( cache_dir ) {}

        bool fetch(const std::string &key, const std::string &pk3_path) override{
            auto _path = getPath(key);
            std::filesystem::copy(_path, pk3_path, std::filesystem::copy_options::overwrite_existing);
            return true;
        }

        void write(const std::string &key, const std::string &pk3_path) override{
            auto _path = getPath(key);
            if( not std::filesystem::exists(_path) ){
                std::filesystem::create_directories(_path);
                std::filesystem::copy(pk3_path, _path, std::filesystem::copy_options::recursive);
            }
        }


    private:
        std::filesystem::path path;

        std::filesystem::path getPath( std::string key){
            unsigned char result[MD5_DIGEST_LENGTH];
            MD5((unsigned char*)key.c_str(), key.size(), result);

            std::ostringstream sout;
            sout<<std::hex<<std::setfill('0');
            for(long long c: result)
            {
                sout<<std::setw(2)<<(long long)c;
            }
            key = sout.str();
            auto dir_ = key.substr(0, 3), filename = key.substr(3, key.size());
            return path / dir_ / filename;
        }
    };

    class DMLabEnv : public Env<cv::Mat, int>{

    private:
        int num_action_repeats;
        LabObject m_Lab;
        std::vector<std::vector<int>> m_ActionSet;
    public:
        struct Option{
            int action_repeats{1};
            std::string game{"lt_chasm"}, dataset_path, renderer{"hardware"};
            std::vector<std::string> observation_names{"RGB_INTERLEAVED"};
            std::optional<std::filesystem::path> level_cache_dir{};
            std::vector<std::vector<int>> action_set{
                    {0, 0, 0, 1, 0, 0, 0},
                    {0, 0, 0, -1, 0, 0, 0},
                    {0, 0, -1, 0, 0, 0, 0},
                    {0, 0, 1, 0, 0, 0, 0},
                    {-20, 0, 0, 0, 0, 0, 0},
                    {20, 0, 0, 0, 0, 0, 0},
                    {-20, 0, 0, 1, 0, 0, 0},
                    {20, 0, 0, 1, 0, 0, 0},
                    {0, 0, 0, 0, 1, 0, 0}
            };
            bool is_test{false};
            std::optional<int> seed;
            int width{96}, height{72};

            Option()=default;
            explicit Option(std::string const& game):game(game){}
        };

        explicit DMLabEnv(Option args);

        int N(){ return int(m_ActionSet.size()); }

        ObservationT reset() noexcept final;

        StepResponse<ObservationT> step(ActionT const& action) noexcept final;

        void render() final;

    private:
        static std::map<std::string, std::string> makeConfig(Option& args);

        inline auto observation(){
            return m_Lab.observations()["RGB_INTERLEAVED"];
        }
    };

    struct DMLabEnv2 :  ObservationWrapper2< DMLabEnv, std::unordered_map<std::string, torch::Tensor>>{

        explicit DMLabEnv2(std::shared_ptr<DMLabEnv> env);

        std::unordered_map<std::string, torch::Tensor> observation( cv::Mat &&) const noexcept override;

        inline StepResponse<std::unordered_map<std::string, torch::Tensor>>
        step(const typename DMLabEnv::ActionT &action) noexcept override {
            auto resp = ObservationWrapper2< DMLabEnv, std::unordered_map<std::string, torch::Tensor>>::step(
                    this->m_Env->step(action) );
            resp.reward = std::clamp<float>(resp.reward, -1, 1);
            lastAction = resp.done ? 0 : action;
            lastReward = resp.done ? 0 : resp.reward;
            return resp;
        }

    private:
        long lastAction;
        int N;
        float lastReward;

    };
}

#endif //GYMENV_DM_LAB_H
