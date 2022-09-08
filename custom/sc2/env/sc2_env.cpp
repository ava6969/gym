//
// Created by dewe on 7/14/22.
//

#include "algorithm"
#include "sc2_env.h"
#include "random"
#include "glog/logging.h"


namespace sc2{

    SC2Env::SC2Env(Option  _opt, std::vector<PlayerPtr> players ):
    opt(std::move(_opt)),
    m_players( std::move(players) ){

        if(m_players.empty()){
            throw std::runtime_error("You must specify the list of players.");
        }

        auto num_players = m_players.size();
        num_agents = std::accumulate(std::begin(m_players),
                                     std::end(m_players),
                                     0, [](int accum, auto const& p){
            return accum + int(p->type() == "agent") ;
        });

        if( not ((1 <= num_players) and (num_players <= 2)) or not num_agents ){
            throw std::runtime_error("Only 1 or 2 players with at least one agent is supported at the moment.");
        }

        if (not opt.map_name){
            throw std::runtime_error("Missing a map name.");
        }
        for( auto const& name: *opt.map_name){
            m_maps.push_back( Maps.at(name) );
        }

        std::vector<int> flat_num_players{};
        for( auto const& m: m_maps){
            flat_num_players.push_back(*m->players());
        }

        auto min_players = std::ranges::min(flat_num_players);
        auto max_players = std::ranges::max(flat_num_players);

        if(opt.battle_net_map){
            auto containsBattleMap = [](auto const& m) { return m->battleNet().has_value(); };
            if( auto it = std::ranges::find_if(m_maps, containsBattleMap); it != m_maps.end())
                throw std::runtime_error( (*it)->name() + " isn't known on Battle.net");
        }

        if( max_players == 1){
            if( num_agents != 1){
                throw std::runtime_error("Single player maps require exactly one Agent.");
            }
        }
        else if( not( 2 <= num_players and num_players <= min_players)){
            throw std::runtime_error(
                    "Maps support 2 - " + std::to_string(min_players) + " players, but trying to join with "
                    + std::to_string (num_players));
        }

        if (opt.save_replay_episodes and not opt.replay_dir){
            throw std::runtime_error("Missing replay_dir");
        }

        m_runConfig = RunConfig{opt.version.value_or("latest")};
        m_parallel = std::make_unique<RunParallel>();

        if( not opt.agent_interface_format ){
            throw std::runtime_error("Please specify agent_interface_format.");
        }

        if( opt.agent_interface_format->size() == 1 ){
            opt.agent_interface_format->resize( num_agents, opt.agent_interface_format->at(0));
        }

        if( opt.agent_interface_format->size() != num_agents){
            throw std::runtime_error(
                    "The number of entries in agent_interface_format should "
                    "correspond 1-1 with the number of agents.");
        }

        for( auto const& aif : *opt.agent_interface_format){
            if ( std::holds_alternative<AgentInterfaceFormat>(aif) ){
                auto aif_f = std::get<AgentInterfaceFormat>(aif);
                action_delay_fns.emplace_back(aif_f.opt.action_delay_fn);
            }else{
                action_delay_fns.emplace_back(std::nullopt);
            }
        }

        interface_formats = *opt.agent_interface_format;
        for( int i = 0; i < interface_formats.size(); i++){
            interface_options.emplace_back(
                    get_interface( interface_formats[i], opt.visualize and i == 0 ) );
        }

        launch_game();
        create_join();
        finalize(opt.visualize);
    }


    sc_pb::InterfaceOptions SC2Env::get_interface(InterfaceFormat& interface_format,
                                                  bool require_raw){

        if ( std::holds_alternative<sc_pb::InterfaceOptions>(interface_format) ){
            auto interface_options = std::get<sc_pb::InterfaceOptions>(interface_format);
            if( require_raw and not interface_options.raw() ){
                interface_options.set_raw(true);
                return interface_options;
            }else{
                return interface_options;
            }
        }

        auto& aif = std::get<AgentInterfaceFormat>(interface_format);
        sc_pb::InterfaceOptions interface;
        interface.set_raw(aif.use_feature_units() or
                          aif.use_unit_counts() or
                          aif.use_raw_units() or
                          require_raw);
        interface.set_show_cloaked(aif.show_cloaked());
        interface.set_show_burrowed_shadows(aif.show_burrowed_shadows());
        interface.set_show_placeholders(aif.show_placeholders());
        interface.set_raw_affects_selection(true);
        interface.set_raw_crop_to_playable_area(aif.raw_crop_to_playable_area());
        interface.set_score(true);

        if ( aif.has_feature_dimensions() ){
            interface.mutable_feature_layer()->set_width( aif.camera_width_world_units() );
            aif.feature_dimensions()->screen().assign_to( interface.mutable_feature_layer()->mutable_resolution() );
            aif.feature_dimensions()->minimap().assign_to( interface.mutable_feature_layer()->mutable_minimap_resolution() );
            interface.mutable_feature_layer()->set_crop_to_playable_area( aif.crop_to_playable_area() );
            interface.mutable_feature_layer()->set_allow_cheating_layers( aif.allow_cheating_layers() );
        }

        if( aif.has_rgb_dimensions( )){
            aif.rgb_dimensions()->screen( interface.render().resolution() );
            aif.rgb_dimensions()->minimap( interface.render().minimap_resolution() );
        }
        return interface;
    }

    void SC2Env::launch_game(){
        if( num_agents > 1){
            //ports = pick_unused_ports(num_agents*2);
            throw std::runtime_error("MultiPlayer not implemented yet");
        }else{
            ports = {};
        }

        std::ranges::transform(interface_options, std::back_inserter(sc2_procs),
                               [this](auto const& interface){
            return m_runConfig->start(interface.has_render(), ports);
        });

        std::ranges::transform(sc2_procs, std::back_inserter(controllers), [](auto const& p){
            return &p->controller();
        });

        if(opt.battle_net_map){
            auto available_maps = controllers[0]->available_maps();
            std::vector<std::string> unavailable;

            for(auto const& m : m_maps){
                if( std::ranges::find(available_maps, *m->battleNet()) == available_maps.end() ){
                    unavailable.push_back(m->name());
                }
            }

            if( not unavailable.empty() ){
                std::stringstream ss;
                ss << "Requested map(s) not in the battle.net cache: ";
                std::ranges::copy(unavailable, std::ostream_iterator<std::string>(ss, " "));
                throw std::runtime_error(ss.str());
            }
        }
    }

    void SC2Env::create_join(){
        auto choice = [](auto&& cont){
            using T = std::remove_reference_t<decltype(cont)>;
            typename T::value_type _res;
            std::ranges::sample(cont, &_res, 1, std::mt19937{std::random_device{}()});
            return _res;
        };

        std::shared_ptr<Map> map = choice(m_maps);

        m_map_name = map->name();
        opt.step_mul = std::max(1, m_defaultStepMul.value_or(map->stepMul()));
        opt.score_multiplier = m_defaultStepMul.value_or(map->scoreMultiplier());
        m_episodeLength = m_defaultEpisodeLength.value_or(map->gameStepsPerEpisode());

        if(m_episodeLength <= 0 or m_episodeLength > MAX_STEP_COUNT)
            m_episodeLength = MAX_STEP_COUNT;

        SC2APIProtocol::Request req;
        auto create = req.mutable_create_game();
        create->set_disable_fog(opt.disable_fog);
        create->set_realtime(opt.realtime);

        if(opt.battle_net_map){
            create->set_battlenet_map_name(*map->battleNet());
        }else{
            create->mutable_local_map()->set_map_data(map->path()->string());
            auto map_data = map->data(m_runConfig.value());
            if(num_agents == 1){
                create->mutable_local_map()->set_map_data(map_data);
            }else{
                for(auto const& c: controllers){
//                    c.saveMap(map->path(), map_data);
                }
            }
        }

        if(opt.random_seed){
            create->set_random_seed(*opt.random_seed);
        }

        for(auto const& p : m_players){
            if(p->type() == "agent"){
                create->add_player_setup()->set_type(SC2APIProtocol::Participant);
            }else{
                auto bot = dynamic_cast<BotImpl*>(p.get());
                auto player = create->add_player_setup();
                player->set_type(SC2APIProtocol::Computer);
                player->set_difficulty(bot->difficulty);
                player->set_ai_build( choice(bot->build) );
                player->set_race( choice(bot->race) );
            }
        }

        controllers[0]->createGame(req);


    };

    void SC2Env::finalize(bool visualize){

    };

}