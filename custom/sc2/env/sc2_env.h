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
#include "custom/sc2/run_parallel.h"
#include "custom/sc2/env/enums.h"
#include "custom/sc2/lib/features.h"
#include "../lib/sc_process.h"
#include "../lib/remote_controller.h"


namespace sc2{

    const std::unordered_map<sc_pb::Result, int8_t> possible_results{
            {sc_pb::Victory, 1},
            {sc_pb::Defeat, -1},
            {sc_pb::Tie, 0},
            {sc_pb::Undecided, 0}
    };

    struct PlayerImpl{
        std::vector<sc_pb::Race> race;
    };
    using PlayerPtr = std::unique_ptr<PlayerImpl>;

    struct AgentImpl : PlayerImpl{
        std::string name;
    };
    using Agent = std::unique_ptr<AgentImpl>;

    struct BotImpl : PlayerImpl{
        std::vector<sc_pb::Difficulty> difficulty;
        std::vector<sc_pb::BuildItem> build;
    };
    using Bot = std::unique_ptr<BotImpl>;

    constexpr float REALTIME_GAME_LOOP_SECONDS = 1 / 22.4;
    constexpr int MAX_STEP_COUNT = 524000; // The game fails above 2^19=524288 steps.;
    constexpr auto NUM_ACTION_DELAY_BUCKETS = 10ll;

    using InterfaceFormat = std::variant<sc_pb::InterfaceOptions, AgentInterfaceFormat>;

    template<class ObsT, class ActionT>
    class SC2Env : public Base<ObsT, ActionT> {

    public:
        struct Option{
            std::optional<std::vector<std::string>> map_name{std::nullopt};
            bool battle_net_map{false};
            std::vector<Player> players{};
            std::optional< std::vector< InterfaceFormat > >
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

        static sc_pb::InterfaceOptions get_interface(InterfaceFormat& interface_format,
                                                     bool require_raw);

    private:
        int num_agents;
        Option opt;
        std::optional<int> last_step_time{std::nullopt};
        std::shared_ptr<RunConfig>  runConfig;
        RunParallel parallel;
        std::optional<std::string> game_info, requested_races;
        std::vector<std::optional< std::function<int()> > > action_delay_fns;
        std::vector< InterfaceFormat > interface_formats;
        std::vector< sc_pb::InterfaceOptions > interface_options;
        std::vector<unsigned short> ports;
        std::vector<StarcraftProcess> sc2_procs;
        std::vector<RemoteController> controllers;

        void launch_game();
        void create_join();
        void finalize(bool);
    };
}



