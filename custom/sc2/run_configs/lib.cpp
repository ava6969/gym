//
// Created by dewe on 7/14/22.
//

#include "../registry.h"
#include "lib.h"
#include <sys/stat.h>
#include "sc2/lib/util.h"
#include "ranges"
#include "sc2/lib/sc_process.h"


namespace sc2{

    std::string RunConfig::map_data(const std::string &map_name, const std::optional<int> &players) {
        std::vector<std::string> map_names{ map_name };

        if(players){
            std::filesystem::path _path(map_name);
            map_names.push_back(
                    LOG_STREAM( _path.begin()->string() << "/(" << *players << ")" << _path.filename().string() ));
        }

        for(auto const& name: map_names){
            auto path = opt.data_dir / "Maps" / name;
            if(std::filesystem::exists(path)){
                std::ifstream in_file(path, std::ios::binary);
                std::string path_bin ( std::istreambuf_iterator<char>( in_file ),
                                        (std::istreambuf_iterator<char>()) );
                return path_bin;
            }else{
                throw std::runtime_error("Reading size failed from stat");
            }
        }
        throw std::runtime_error(LOG_STREAM("Map" << map_name << " not found in " << opt.data_dir << "/Maps."));
    }

    RunConfig::RunConfig(std::string _version) {
        opt.data_dir = std::string(getenv("HOME")) + "/StarCraftII";
        opt.replay_dir = opt.data_dir / "Replays";
        opt.tmp_dir = std::nullopt;
        opt.cwd = std::nullopt;
        version = get_version(std::move(_version));
        m_execName = "SC2_x64";
    }

    std::unique_ptr<StarcraftProcess> RunConfig::start(bool want_rgb,
                          std::vector<unsigned short> const& extra_ports,
                          std::vector<std::string> extra_args) {

        if(want_rgb) {

            if (KNOWN_GL_LIBS.empty()) {
                extra_args.emplace_back("-headlessNoRender");
                std::cout << "No GL library found, so RGB rendering will be disabled. \"\n"
                             "            \"For software rendering install libosmesa.\n";
            } else {
                namespace bp = boost::process; //we will assume this for all further examples
                bp::ipstream out;
                bp::child c("/sbin/ldconfig -p", bp::std_out > out);
                std::string libs;
                out >> libs;
                c.terminate();
                auto lib_list = split(libs, "\n");
                for (auto lib: lib_list) {
                    lib = lib.substr(0, lib.size() - 1);
                    lib = split(lib, " ")[0];
                    for (auto const &item: KNOWN_GL_LIBS) {
                        auto [arg, lib_name] = item;
                        if (std::ranges::find(lib_list, lib_name) != lib_list.end()) {
                            extra_args.insert(extra_args.end(), {arg, lib_name});
                        }
                    }
                }
            }
        }

        if( not std::filesystem::exists(opt.data_dir)){
            throw SC2LaunchError(LOG_STREAM("Expected to find StarCraft II installed at '%s'. If it's not "
                                            "installed, do that and run it once so auto-detection works. If "
                                            "auto-detection failed repeatedly, then set the SC2PATH environment "
                                            "variable with the correct location." << opt.data_dir));
        }

        auto exec_path = opt.data_dir / ("Versions/Base" + std::to_string(version.build_version)) / m_execName;
        if(not std::filesystem::exists(exec_path)){
            throw SC2LaunchError(LOG_STREAM("No SC2 binary found at: " << exec_path));
        }

        return std::make_unique<StarcraftProcess>(*this, exec_path, version);
    }

    std::unordered_map<std::basic_string<char>, Version> RunConfig::get_versions(
            std::optional<std::string> const &containing) {
        auto versions_dir = opt.data_dir / "Versions";
        std::string version_prefix = "Base";
        std::vector<int> versions_found;

        for(auto const& v : fs::directory_iterator(versions_dir)){
            auto fn = v.path().stem().string();
            if(fn.starts_with(version_prefix)){
                versions_found.push_back( std::stoi(fn.substr(version_prefix.size())) );
            }
        }

        std::ranges::sort(versions_found);

        if(versions_found.empty()){
            throw SC2LaunchError(LOG_STREAM("No SC2 Versions found in " << versions_dir));
        }

        std::vector<Version > known_versions;
        for(auto v : VERSIONS | std::views::values){
            if( std::ranges::find(versions_found, v.build_version ) != versions_found.end() ){
                known_versions.push_back(v);
            }
        }

        known_versions.emplace_back("latest", std::ranges::max(versions_found), std::nullopt, std::nullopt);
        auto ret = version_dict(known_versions);

        if (containing and not ret.contains(containing.value())) {
            std::stringstream ss;
            ss << "Unknown game version: " << containing.value() << ". Known versions: ";
            for (auto const & k: ret | std::views::keys)
                ss << k << " ";
            ss << ".\n";
            throw std::runtime_error(ss.str());
        }
        return ret;
    }

    Version RunConfig::get_version(std::string game_version) {
        if (std::ranges::count(game_version, '.') == 1) {
            game_version += ".0";
        }
        auto versions = get_versions(game_version);
        return versions[game_version];
    }

    Version RunConfig::get_version(Version const &game_version) {
        if (game_version.game_version.empty()) {
            throw std::runtime_error(LOG_STREAM("Version " << game_version << " supplied without a game version."));
        }
        if (game_version.data_version and game_version.binary and game_version.build_version) {
            return game_version;
        }

        return {};
    }

}