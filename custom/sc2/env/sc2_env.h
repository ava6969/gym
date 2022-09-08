#pragma once
//
// Created by dewe on 7/14/22.
//

#include "environment.h"
#include "memory"
#include "variant"
#include "optional"
#include "filesystem"
#include "../registry.h"
#include "../run_parallel.h"
#include "enums.h"
#include "../lib/features.h"
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
        explicit PlayerImpl(std::vector<sc_pb::Race> _race):race( std::move(_race) ){}
        virtual std::string type() = 0;
        virtual ~PlayerImpl()=default;
    };

    using PlayerPtr = std::unique_ptr<PlayerImpl>;

    struct AgentImpl : PlayerImpl{
        std::string name;
        explicit AgentImpl(std::vector<sc_pb::Race> _race, std::string _name=""):
        PlayerImpl( std::move(_race)), name( std::move(_name) ) {}
        std::string type() override { return "agent"; }
    };
    using Agent = std::unique_ptr<AgentImpl>;

    struct BotImpl : PlayerImpl{
        sc_pb::Difficulty difficulty;
        std::vector<sc_pb::AIBuild> build;

        BotImpl(std::vector<sc_pb::Race> _race,
                  sc_pb::Difficulty const& _difficulty,
                  std::vector<sc_pb::AIBuild> _build):
        PlayerImpl( std::move(_race)), difficulty( _difficulty ), build( std::move(_build) ) {}
        std::string type() override { return "bot"; }
    };
    using Bot = std::unique_ptr<BotImpl>;

    constexpr float REALTIME_GAME_LOOP_SECONDS = 1 / 22.4;
    constexpr int MAX_STEP_COUNT = 524000; // The game fails above 2^19=524288 steps.;
    constexpr auto NUM_ACTION_DELAY_BUCKETS = 10ll;

    using InterfaceFormat = std::variant<sc_pb::InterfaceOptions, AgentInterfaceFormat>;

    class SC2Env  {

    public:
        struct Option{
            std::optional<std::vector<std::string>> map_name{std::nullopt};
            bool battle_net_map{false};
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

        std::vector<PlayerPtr> m_players;
    public:
        SC2Env(Option ,  std::vector<PlayerPtr>  );
        static sc_pb::InterfaceOptions get_interface(InterfaceFormat& interface_format,
                                                     bool require_raw);

    private:
        int num_agents;
        std::string m_map_name;
        Option opt;
        std::optional<int> last_step_time{std::nullopt};
        std::optional<RunConfig>  m_runConfig;
        std::unique_ptr<RunParallel> m_parallel;
        std::optional<std::string> game_info, requested_races;
        std::vector<std::optional< std::function<int()> > > action_delay_fns;
        std::vector< InterfaceFormat > interface_formats;
        std::vector< sc_pb::InterfaceOptions > interface_options;
        std::vector<unsigned short> ports;
        std::vector< std::unique_ptr<StarcraftProcess> > sc2_procs;
        std::vector<RemoteController*> controllers;
        std::vector<std::shared_ptr<Map>> m_maps;
        std::optional<int> m_defaultStepMul, m_defaultScoreMultiplier, m_defaultEpisodeLength, m_episodeLength;


        void launch_game();
        void create_join();
        void finalize(bool);
    };
}



