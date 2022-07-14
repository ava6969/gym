//
// Created by dewe on 7/14/22.
//

#inc
#include "custom/sc2/registry.h"
#include "lib.h"


namespace sc2{

    std::shared_ptr<Map> Map::get( std::string const& map_name){
        if(Maps.contains(map_name)){
            return Maps.at(map_name);
        }

        auto maps = getMaps();
        if( maps.contains(map_name)){
            return maps.at(map_name);
        }

        throw NoMapError("Map doesn\'t exist: " + map_name);
    }

    std::vector<std::string> Map::all_subclasses( ){
        std::vector<std::string> subs;
        for( auto const& s : map::__subclasses__.at( this->name() )){
            subs.push_back(s);
            for( auto const& c: Maps.at(s)->all_subclasses()){
                subs.push_back(c);
            }
        }
        return subs;
    }

    std::vector<std::string> Map::data( class RunConfig* run_config) const {
        try{
            return run_config->map_data(path(), players);
        } catch (std::ios_base::failure const& exp) {
            if (download){
                std::cerr << "Error reading map " << name() << " from: " << exp.what() << "\n";
                std::cerr << "Download the map from: " << download.value() << "\n";
            }
            throw;
        }
    }

    std::unordered_map< std::string, std::shared_ptr<Map>> Map::getMaps( ){
        std::unordered_map< std::string, std::shared_ptr<Map>> maps;
        for( auto const& mp_name : __subclasses__.at("Map") ){
            auto mp = Maps.at(mp_name);
            if ( mp->filename or mp->battle_net){
                auto map_name = mp->name();
                if ( maps.contains(map_name)){
                    throw DuplicateMapError("Duplicate map found: " + map_name);
                }
                maps[map_name] = mp;
            }
        }
        return maps;
    }
}