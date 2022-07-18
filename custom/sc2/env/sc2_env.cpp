//
// Created by dewe on 7/14/22.
//

#include "algorithm"
#include "sc2_env.h"
#include "glog/logging.h"
#include "../lib/portspicker.h"


namespace sc2{

    template<class ObsT, class ActionT>
    SC2Env<ObsT, ActionT>::SC2Env(Option const& _opt):
    opt(std::move(_opt)){

        if(opt.players.empty()){
            throw std::runtime_error("You must specify the list of players.");
        }

        auto num_players = opt.players.size();
        num_agents = std::accumulate(std::begin(opt.players),
                                     std::end(opt.players),
                                     0.0, [](auto const& p, int accum){
            return accum + (dynamic_cast<AgentImpl*>(p.get()) != nullptr ? 1 : 0);
        });

        if( not ((1 <= num_players) and (num_players <= 2)) or not num_agents ){
            throw std::runtime_error("Only 1 or 2 players with at least one agent is supported at the moment.");
        }

        if (not opt.map_name){
            throw std::runtime_error("Missing a map name.");
        }
        for( auto const& name: opt.map_name){
            opt.maps.push_back( Maps.at(name) );
        }

        std::vector<int> flat_num_players{};
        for( auto m: opt.maps){
            flat_num_players.push_back(m->players.value());
        }

        auto min_players = std::ranges::min(flat_num_players);
        auto max_players = std::ranges::max(flat_num_players);

        if(opt.battle_net_map){
            for( auto const& m : opt.maps){
                if(not m->battle_net){
                    throw std::runtime_error(m->name() + " isn't known on Battle.net");
                }
            }
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

        runConfig = RunConfig::get(opt.version);

        if( opt.agent_interface_format ){
            throw std::runtime_error("Please specify agent_interface_format.");
        }

        if( opt.agent_interface_format->size() == 1 ){
            opt.agent_interface_format->resize( opt.agent_interface_format[0], num_agents);
        }

        if( opt.agent_interface_format.size() != num_agents){
            throw std::runtime_error(
                    "The number of entries in agent_interface_format should "
                    "correspond 1-1 with the number of agents.");
        }

        for( auto const& aif : *opt.agent_interface_format){
            if ( auto aif_f = std::get<AgentInterfaceFormat>(aif) ){
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

    template<class ObsT, class ActionT>
    sc_pb::InterfaceOptions SC2Env<ObsT, ActionT>::get_interface(InterfaceFormat& interface_format,
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
            aif.feature_dimensions()->screen( interface.feature_layer().resolution() );
            aif.feature_dimensions()->minimap( interface.feature_layer().minimap_resolution() );
            interface.mutable_feature_layer()->set_crop_to_playable_area( aif.crop_to_playable_area() );
            interface.mutable_feature_layer()->set_allow_cheating_layers( aif.allow_cheating_layers() );
        }

        if( aif.has_rgb_dimensions( )){
            aif.rgb_dimensions()->screen( interface.render().resolution() );
            aif.rgb_dimensions()->minimap( interface.render().minimap_resolution() );
        }
        return interface;
    }

    template<class ObsT, class ActionT>
    void SC2Env<ObsT, ActionT>::launch_game(){
        if( num_agents > 1){
            ports = pick_unused_ports(num_agents*2);
        }else{
            ports.clear();
        }

        std::ranges::transform(interface_options, std::back_inserter(sc2_procs),
                               [this](auto const& interface){
            return runConfig->start(ports, interface.has_render());
        });

        std::ranges::transform(sc2_procs, std::back_inserter(controllers), [](auto const& p){
            return p.controller();
        });

        if(opt.battle_net_map){
            auto available_maps = controllers[0].available_maps();
            auto available_maps_set = std::unordered_set(available_maps.begin(),
                                                         available_maps.end() );
            std::vector<std::string> unavailable;
            for(auto const& m : opt.maps){
                if( not available_maps_set.contains( m.battle_net ) ){
                    unavailable.emplace_back(m.name());
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

    template<class ObsT, class ActionT>
    void SC2Env<ObsT, ActionT>::create_join(){

    };

    template<class ObsT, class ActionT>
    void SC2Env<ObsT, ActionT>::finalize(bool visualize){

    };

}