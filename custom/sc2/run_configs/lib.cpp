//
// Created by dewe on 7/14/22.
//

#include "../registry.h"
#include "lib.h"


namespace sc2{

    std::vector<std::string> RunConfig::all_subclasses( ){
        std::vector<std::string> subs;
        for( auto const& s : rc::__subclasses__.at( this->name() )){
            subs.push_back(s);
            for( auto const& c: RunConfigs.at(s)->all_subclasses()){
                subs.push_back(c);
            }
        }
        return subs;
    }

    std::shared_ptr<RunConfig> RunConfig::get(std::optional<std::string> const & _version) {
        std::unordered_map<std::string, std::shared_ptr<RunConfig>> configs;
        for (auto const &c_: RunConfig::all_subclasses()) {
            auto c = RunConfigs.at(c_);
            if (c->priority()) {
                configs[c->name()] = c;
            }
        }

        if (configs.empty()) {
            throw SC2LaunchError("No valid run_configs found.");
        }

        auto sc2_run_config = flags.present<std::string>("sc2_run_config");
        if (not sc2_run_config) {
            return std::ranges::max(configs, [](
                    std::pair<std::string, std::shared_ptr<RunConfig>> const & a_,
                    std::pair<std::string, std::shared_ptr<RunConfig>> const& b_) {
                auto a = a_.second->priority();
                auto b = b_.second->priority();
                return ((not a) ? true : not b ? false : *a < *b);
            }).make(_version);
        }

        try {
            return configs.at(sc2_run_config.value())->make(_version);
        } catch (std::out_of_range const &err) {
            std::stringstream ss;
            int i = 0;
            for (auto const &[k, _]: configs) {
                ss << k;
                if (++i < configs.size())
                    ss << ", ";
            }
            throw SC2LaunchError("Invalid run_config. Valid configs are: " + ss.str());
        }
    }

    std::ofstream RunConfig::map_data(std::string const& map_name,
                                      std::optional<std::vector<Player>> const& players){
        std::vector<std::string> map_names{map_name};
        if(players){
            std::filesystem::path path{map_name};
//            std::stringstream ss;
//            ss << '(' << players <<
//            path /= players +
            map_names.push_back(path);
        }
        for ( auto const& name: map_names){
            auto path = opt.data_dir / "Maps" / name;
            if( std::filesystem::exists(path) ){
                return std::ofstream(path);
            }
        }
        std::stringstream ss;
        ss << "Map " << map_name << " not found in " << opt.data_dir << "/Maps.";
        throw std::runtime_error(ss.str());
    }
}