//
// Created by dewe on 12/17/21.
//

#ifndef GYM_MONITOR_H
#define GYM_MONITOR_H

#include "common/wrapper.h"
#include <tuple>
#include <filesystem>

namespace gym {

    using namespace std::string_literals;

    struct Result{
        float r, l, t;
    };

    struct ResultWriter{
        std::ofstream os;

        template< class ... Headers> inline
        ResultWriter(std::filesystem::path fileName, size_t t_start, std::string id ){
            assert(! fileName.empty());
            os = std::ofstream(fileName.replace_extension(".csv"));
            os << "t_start: " << t_start << "\nid: " << id << "\n";
            os << "t,r,l";
        }

        ResultWriter& operator<<(Result const& w ){
            os << w.t << "," << w.r << "," << w.l << "\n";
            return *this;
        }
    };

    template<class EnvType, bool allowEarlyResets=true>
    class __attribute__ ((visibility("hidden"))) Monitor : public Wrapper<EnvType>{

    public:
        explicit Monitor(std::unique_ptr< EnvType > env,
                std::optional<double> invalid_returns=std::numeric_limits<double>::max(),
                std::string const& id = "",
                std::filesystem::path const& fileName = ""):
                Wrapper<EnvType>( std::move(env) ), invalid_returns(invalid_returns){

            this->t_start = std::chrono::high_resolution_clock::now();
            if( not fileName.empty() ){
                writer = ResultWriter(fileName, t_start.time_since_epoch().count(), id);
            }
        }

        inline typename EnvType::ObservationT reset() noexcept override{
            return reset( this->m_Env->reset() );
        }

        inline typename EnvType::ObservationT reset( typename EnvType::ObservationT && x) noexcept{
            if constexpr(not allowEarlyResets){
                if(needsReset){
                    std::cerr <<  "Tried to reset an environment before done. If you want to allow early resets, "
                                  "wrap your env with Monitor(env, path, allow_early_resets=True)\n";
                }
            }
            reward = {};
            _steps = {};
            needsReset = false;
            return x;
        }

        inline typename EnvType::StepT step(typename EnvType::ActionT const& action) noexcept override{
            return step(this->m_Env->step(action));
        }

        typename EnvType::StepT step(typename EnvType::StepT && response) noexcept {
            if( needsReset ){
                std::cerr << "Tried to step environment that needs reset\n";
            }
            reward .push_back( response.reward );
            _steps++;
            if( response.done ){
                needsReset = true;
                episodeReturns.push_back( std::accumulate(reward.begin(), reward.end(), 0.f) );
                episodeLengths.push_back( _steps );
                episodeTimes.push_back( std::chrono::duration<double>( std::chrono::high_resolution_clock::now() -
                                                                       t_start).count());
                Result epInfo{episodeReturns.back(), _steps, episodeTimes.back()};

                if(invalid_returns and epInfo.r >= invalid_returns){
                    std::stringstream ss;
                    ss << this->info() << "\n";
                    ss << "Invalid Episode Returns: " << epInfo.r << "\n" <<  reward << " Steps: " << epInfo.l;
                    std::cerr << ss.str() << "\n";
                }

                if(writer)
                    writer.value() << epInfo;
                response.info["episode"] = epInfo;
            }
            totalSteps++;

            return response;
        }

        inline auto getTotalSteps() const noexcept { return totalSteps; }
        inline auto getEpisodeRewards() const noexcept { return episodeReturns; }
        inline auto getEpisodeLengths() const noexcept { return episodeLengths; }
        inline auto getEpisodeTimes() const noexcept { return episodeTimes; }

    private:
        decltype(std::chrono::high_resolution_clock::now()) t_start;
        std::optional<ResultWriter> writer;
        std::vector<float> episodeReturns;
        std::vector<float> episodeLengths;
        std::vector<float> episodeTimes;
        size_t totalSteps{};
        std::vector<float> reward{};
        float _steps{};
        bool needsReset{true};
        std::optional<double> invalid_returns;
    };
}

#endif //GYM_MONITOR_H
