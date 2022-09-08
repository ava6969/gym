//
// Created by dewe on 7/15/22.
//
#include "util.h"
#include "features.h"


namespace sc2{

    AgentInterfaceFormat::AgentInterfaceFormat(Option  _opt):opt( std::move(_opt)){

        if( not(opt.feature_dimensions or opt.rgb_dimensions or opt.use_raw_units)){
            throw std::runtime_error("Must set either the feature layer or rgb dimensions, "
                                     "or use raw units.");
        }

        if(opt.action_space){
            if(opt.action_space == ActionSpace::Raw){
                opt.use_raw_actions = true;
            }else if( (opt.action_space == ActionSpace::Features and (not opt.feature_dimensions)) or
                      (opt.action_space == ActionSpace::RGB and (not opt.rgb_dimensions)) ){
                throw std::runtime_error("Action space must match the observations, action space={}, "
                                         "feature_dimensions={}, rgb_dimensions={}");
            }
        }else{
            if(opt.use_raw_actions){
                opt.action_space = ActionSpace::Raw;
            }else if(opt.feature_dimensions and opt.rgb_dimensions){
                throw std::runtime_error("You must specify the action space if you have both screen and "
                                         "rgb observations.");
            }else if(opt.feature_dimensions){
                opt.action_space = ActionSpace::Features;
            }else{
                opt.action_space = ActionSpace::RGB;
            }
        }

        if(opt.raw_resolution){
            opt.raw_resolution = Point{std::get<int>(*opt.raw_resolution)};
        }

        if(opt.use_raw_actions){
            if(not opt.use_raw_units){
                throw std::runtime_error(
                        "You must set use_raw_units if you intend to use_raw_actions");
            }
            if (opt.action_space != ActionSpace::Raw){
                throw std::runtime_error(
                        "Don't specify both an action_space and use_raw_actions.");
            }
        }

        if(opt.rgb_dimensions and
        opt.rgb_dimensions->screen().x() < opt.rgb_dimensions->minimap().x() or
        opt.rgb_dimensions->screen().y() < opt.rgb_dimensions->minimap().y() ){
            throw std::runtime_error( "RGB Screen (%s) can't be smaller than the minimap (%s)." );
        }

        m_action_dimensions = (opt.action_space == ActionSpace::Features) ?
                opt.feature_dimensions.value() :
                opt.rgb_dimensions.value();

    }

    Features::Features(std::optional<AgentInterfaceFormat> const& agent_interface_format,
            std::optional<SC2APIProtocol::Size2DI> const& map_size,
            std::optional<std::unordered_map<int, SC2APIProtocol::Race>> const& requested_races,
            std::string const& map_name){

        if(not agent_interface_format){
            throw std::runtime_error("Please specify agent_interface_format");
        }

        m_agentInterfaceFormat = agent_interface_format.value();
        auto aif = m_agentInterfaceFormat;
        if(not aif.raw_resolution() and map_size){
            aif.raw_resolution<Point>() = *map_size;
        }
        m_mapSize = *map_size;
        m_mapName = map_name;

        if( aif.use_feature_units() or aif.use_camera_position() or aif.use_raw_units()){
            initCamera(aif.feature_dimensions(), map_size, aif.camera_width_world_units(), aif.raw_resolution());
        }

        m_sendObservationProto = aif.send_observation_proto();
        m_raw = aif.use_raw_actions();

        if(m_raw){
            m_validFunctions = initValidRawFunctions(aif.raw_resolution(), aif.max_selected_units());
            m_rawTags = {};
        }
        else{
            m_validFunctions = initValidFunctions(aif.action_dimensions());
        }

        m_requested_races = requested_races;
        if(requested_races and requested_races->size() > 2){
            throw std::runtime_error("len(requested_races) must be <= 2");
        }
    }

    void Features::initCamera(Dimensions *, const std::optional<SC2APIProtocol::Size2DI> &, int,
                              const std::optional<std::variant<Point, int>> &) {

    }

    ValidActions initValidFunctions(Dimensions const& action_dimensions){
        std::array<uint32_t, 2> screen  =
                {static_cast<unsigned int>(action_dimensions.screen().x()),
                 static_cast<unsigned int>(action_dimensions.screen().y())},
         minimap = {static_cast<unsigned int>(action_dimensions.minimap().x()),
                    static_cast<unsigned int>(action_dimensions.minimap().y())};

        Arguments types{
                fromSpec(0, "screen", screen),
                fromSpec(1, "minimap", screen),
                fromSpec(2, "screen2", minimap),
                fromSpec(3, "queued", TYPES.queued.sizes),
                fromSpec(4, "control_group_act", TYPES.control_group_act.sizes),
                fromSpec(5, "control_group_id", TYPES.control_group_id.sizes),
                fromSpec(6, "select_add", TYPES.select_add.sizes),
                fromSpec(7, "select_unit_act", TYPES.select_unit_act.sizes),
                fromSpec(8, "select_unit_id", TYPES.select_unit_id.sizes),
                fromSpec(9, "select_worker", TYPES.select_worker.sizes),
                fromSpec(10, "build_queue_id", TYPES.build_queue_id.sizes),
                fromSpec(11, "unload_id", TYPES.unload_id.sizes)
        };

        auto getArgs = [&types](Function const& f){
            std::vector<ArgumentType> result(f.args.size());
            std::ranges::transform(f.args, result.begin(), [&types](auto const& arg){
                switch (arg.id) {
                    case 0:
                        return types.screen;
                    case 1:
                        return types.minimap;
                    case 2:
                        return types.screen2;
                    case 3:
                        return types.queued;
                    case 4:
                        return types.control_group_act;
                    case 5:
                        return types.control_group_id;
                    case 6:
                        return types.select_add;
                    case 7:
                        return types.select_unit_act;
                    case 8:
                        return types.select_unit_id;
                    case 9:
                        return types.select_worker;
                    case 10:
                        return types.build_queue_id;
                    case 11:
                        return types.unload_id;
                    default:
                        throw std::runtime_error("Invalid Action ID: " + std::to_string(arg.id));
                }
            });
            return result;
        };

        Functions functions{
                fromSpec(0, "noop", getArgs(FUNCTIONS.noop)),
                fromSpec(1, "move_camera", getArgs(FUNCTIONS.move_camera)),
                fromSpec(2, "select_point", getArgs(FUNCTIONS.select_point)),
                fromSpec(3, "select_rect", getArgs(FUNCTIONS.select_rect)),
                fromSpec(4, "select_control_group", getArgs(FUNCTIONS.select_control_group)),
                fromSpec(5, "select_unit", getArgs(FUNCTIONS.select_unit)),
                fromSpec(6, "select_idle_worker", getArgs(FUNCTIONS.select_idle_worker)),
                fromSpec(7, "select_army", getArgs(FUNCTIONS.select_army)),
                fromSpec(8, "select_warp_gates", getArgs(FUNCTIONS.select_warp_gates)),
                fromSpec(9, "select_larva", getArgs(FUNCTIONS.select_larva)),
                fromSpec(10, "unload", getArgs(FUNCTIONS.unload)),
                fromSpec(11, "build_queue", getArgs(FUNCTIONS.build_queue))
        };

        return {types, functions};
    }

    ValidActions initValidRawFunctions(std::optional<std::variant<Point, int>> const& raw_resolution,
                                       int max_selected_units){
        return {};
    }

    Features fromGameInfo(SC2APIProtocol::ResponseGameInfo const& game_info,
                          AgentInterfaceFormat const& aif, std::string map_name){

        if(map_name.empty())
            map_name = game_info.map_name();

        std::optional<Dimensions> feature_dimensions, rgb_dimensions;
        std::optional<float> camera_width_world_units;

        if(game_info.options().has_feature_layer()){
            auto fl_opts = game_info.options().feature_layer();
            feature_dimensions = Dimensions().screen(fl_opts.resolution()).minimap(fl_opts.minimap_resolution());
            camera_width_world_units = game_info.options().feature_layer().width();
        }

        if(game_info.options().has_render()){
            auto rgb_opts = game_info.options().render();
            rgb_dimensions = Dimensions().screen(rgb_opts.resolution()).minimap(rgb_opts.minimap_resolution());
        }

        auto map_size = game_info.start_raw().map_size();
        std::unordered_map<int, SC2APIProtocol::Race> requested_races;
        for(auto const& info : game_info.player_info()){
            if(info.type() != SC2APIProtocol::Observer){
                requested_races[info.player_id()] = info.race_requested();
            }
        }

        if(aif.rgb_dimensions() != *rgb_dimensions or aif.feature_dimensions() != *feature_dimensions or
                (feature_dimensions and aif.camera_width_world_units() != camera_width_world_units)){
            throw std::runtime_error(
                    LOG_STREAM("The supplied agent_interface_format doesn't match the resolutions computed from\n"
                               "the game_info:"
                               "\nrgb_dimensions: " << aif.rgb_dimensions() << " vs " << *rgb_dimensions <<
                               "\nfeature_dimensions: " << aif.feature_dimensions() << " vs " << *feature_dimensions <<
                               "\ncamera_width_world_units: " << aif.camera_width_world_units() << " vs " << *camera_width_world_units)
                    );
        }

        return {aif, map_size, requested_races, map_name};
    }
}