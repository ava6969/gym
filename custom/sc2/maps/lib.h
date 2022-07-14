#pragma once
//
// Created by dewe on 7/14/22.
//

#include "stdexcept"
#include "optional"
#include "string"
#include "filesystem"
#include "iostream"
#include "vector"
#include "functional"


namespace sc2{
    struct DuplicateMapError : std::exception {
        std::string msg;
        DuplicateMapError(std::string const& msg);
        const char * what() const noexcept override{
            return msg.c_str();
        }
    };

    struct NoMapError : std::exception {
        std::string msg;
        NoMapError(std::string const& msg);
        const char * what() const noexcept override{
            return msg.c_str();
        }
    };

    class Map{

    public:
        inline std::optional<std::filesystem::path> path() const noexcept{
            if(filename){
                auto map_path = directory / filename.value();
                if( not (map_path.extension() == ".SC2Map") ){
                    map_path = map_path.append(".SC2Map");
                }
                return map_path;
            }
            return {};
        }

        std::vector<std::string> data( class RunConfig* run_config) const;

        virtual std::string name() const noexcept { return "Map"; }

        friend std::ostream& operator<<(std::ostream& os, Map const& map){
            auto _path = map.path();
            os << map.name() << "\n";
            if (_path){
                os << "    file: " << _path.value().string()<< "\n";
            }
            if (battle_net){
                os << "    battle_net: " + battle_net.value()<< "\n";
            }
            os << "    players: " << players.value() << ", score_index: " << score_index << ", score_multiplier: "
            << score_multiplier << "\n";
            os << "    step_mul: " << step_mul << ", game_steps_per_episode: " << game_steps_per_episode << "\n";

            return os;
        }

        std::vector<std::string> all_subclasses( );
        std::shared_ptr<Map> get( std::string const& map_name);
        std::unordered_map< std::string, std::shared_ptr<Map>> getMaps( );


    protected:
        inline static std::filesystem::path directory{};
        inline static std::optional<std::filesystem::path> download{};
        inline static std::optional<std::string> filename{};
        inline static int game_steps_per_episode{0}, step_mul{8}, score_index{-1}, score_multiplier{1};
        inline static std::optional<int> players{};
        inline static std::optional<std::string> battle_net{};

    };

}


