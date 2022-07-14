//
// Created by dewe on 7/14/22.
//

#include "algorithm"
#include "sc2_env.h"


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

    }
}