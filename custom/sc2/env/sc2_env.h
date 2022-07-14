#pragma once
//
// Created by dewe on 7/14/22.
//

#include "environment.h"
#include "memory"
#include "variant"
#include "optional"
#include "filesystem"
#include "custom/sc2/registry.h"


namespace sc2{


    struct PlayerImpl{

    };

    using Player = std::unique_ptr<PlayerImpl>;

    template<class ObsT, class ActionT>
    class SC2Env : public Base<ObsT, ActionT> {

    public:
        struct Option{
            std::optional<std::vector<std::string>> map_name{std::nullopt};
            bool battle_net_map{false};
            std::vector<Player> players{};
            std::optional<std::variant< std::vector<AgentInterfaceFormat>, AgentInterfaceFormat>>
            agent_interface_format{std::nullopt};
            float discount{1.};
            bool discount_zero_after_timeout{false};
            bool visualize{false};
            std::optional<int> step_mul{std::nullopt};
            bool realtime{false};
            int save_replay_episodes{0};
            std::optional< std::filesystem::path > replay_dir{std::nullopt};
            std::optional< std::string > replay_prefix{std::nullopt};
            std::optional<int> game_steps_per_episode{std::nullopt},
             score_index{std::nullopt},
            score_multiplier{std::nullopt},
            random_seed{std::nullopt};
            bool disable_fog{false};
            bool ensure_available_actions{true};
            std::optional<std::string> version{std::nullopt};
        };

    public:
        SC2Env(Option const& );

    private:
        int num_agents;
        Option opt;
        std::optional<int> last_step_time{std::nullopt};
    };
}



