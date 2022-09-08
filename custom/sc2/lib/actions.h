#pragma once
//
// Created by dewe on 7/15/22.
//

#include "s2clientprotocol/spatial.pb.h"
#include "s2clientprotocol/ui.pb.h"
#include "s2clientprotocol/sc2api.pb.h"
#include "point.h"
#include "variant"
#include "optional"


namespace sc2{

    using SC2Action = SC2APIProtocol::Action;
    using FactoryType = std::function<struct ArgumentType(int, std::string const& )>;

    enum class ActionSpace{
        Features,
        RGB,
        Raw
    };

    struct FunctionOption{
        SC2Action action;
        ActionSpace space;
        Point* screen{nullptr};
        Point* screen2{nullptr};
        Point* minimap{nullptr};
        int* select_point_act{nullptr};
        int* select_add{nullptr};
        int* select_unit_act{nullptr};
        int* select_unit_id{nullptr};
    };

    SC2APIProtocol::ActionSpatial spatial(SC2Action const& action, ActionSpace actionSpace);
    inline SC2APIProtocol::ActionSpatial spatial(FunctionOption const& opt) { return spatial(opt.action, opt.space); }

    static void noop(FunctionOption& ){}

    void moveCamera(FunctionOption&);

    void selectPoint(FunctionOption&);

    void selectRect(FunctionOption&);

    void selectIdleWorker(FunctionOption&);

    void selectArmy(FunctionOption&);

    void selectWarpGates(FunctionOption&);

    void selectLarva(FunctionOption&);

    void selectUnit(FunctionOption&);

    void controlGroup(FunctionOption&);

    void unload(FunctionOption&);

    void buildQueue(FunctionOption&);

    void cmdQuick(FunctionOption&);

    void cmdScreen(FunctionOption&);

    void cmdMiniMap(FunctionOption&);

    using FunctionSignature = void(*)(FunctionOption&);

    struct ArgumentType{
        int id;
        std::string name;
        std::array<uint32_t, 2 > sizes;
        std::function< std::variant<Point, int, bool, std::vector<int>>(std::vector<int>)> fn;
        std::optional<uint32_t> count;
    };

    FactoryType fromEnum(std::unordered_map<std::string, int> const& options);

    FactoryType fromScalar(int value);
    FactoryType fromPoint();
    ArgumentType fromSpec(int id_, std::string const&, std::array<uint32_t, 2> const&);
    FactoryType fromUnitTag(int count, uint32_t );

    struct Arguments{
        ArgumentType screen, minimap, screen2,  queued, control_group_act, control_group_id, select_point_act,
        select_add, select_unit_act, select_unit_id, select_worker, build_queue_id, unload_id;
    };

    struct RawArguments {
        ArgumentType world, queued, unit_tags, target_unit_tag;
    };

    const std::unordered_map<std::string, int> QUEUED_OPTIONS{
            {"now", false}, {"queue", true}
    };

    const std::unordered_map<std::string, int> CONTROL_GROUP_ACT_OPTIONS{
            {"recall", SC2APIProtocol::ActionControlGroup::Recall},
            {"set", SC2APIProtocol::ActionControlGroup::Set},
            {"append", SC2APIProtocol::ActionControlGroup::Append},
            {"set_and_steal", SC2APIProtocol::ActionControlGroup::SetAndSteal},
            {"append_and_steal", SC2APIProtocol::ActionControlGroup::AppendAndSteal}
    };

    const std::unordered_map<std::string, int> SELECT_POINT_ACT_OPTIONS{
            {"select", SC2APIProtocol::ActionSpatialUnitSelectionPoint::Select},
            {"toggle", SC2APIProtocol::ActionSpatialUnitSelectionPoint::Toggle},
            {"select_all_type", SC2APIProtocol::ActionSpatialUnitSelectionPoint::AllType},
            {"add_all_type", SC2APIProtocol::ActionSpatialUnitSelectionPoint::AddAllType}
    };

    const std::unordered_map<std::string, int> SELECT_UNIT_ACT_OPTIONS{
            {"select", SC2APIProtocol::ActionMultiPanel::SingleSelect},
            {"deselect", SC2APIProtocol::ActionMultiPanel::DeselectUnit},
            {"select_all_type", SC2APIProtocol::ActionMultiPanel::SelectAllOfType},
            {"deselect_all_type", SC2APIProtocol::ActionMultiPanel::DeselectAllOfType}
    };

    const std::unordered_map<std::string, int> SELECT_WORKER_OPTIONS{
            {"select", SC2APIProtocol::ActionSelectIdleWorker::Set},
            {"add", SC2APIProtocol::ActionSelectIdleWorker::Add},
            {"select_all", SC2APIProtocol::ActionSelectIdleWorker::All},
            {"add_all", SC2APIProtocol::ActionSelectIdleWorker::AddAll}
    };

    const std::unordered_map<std::string, int> SELECT_ADD_OPTIONS{
            {"select", false}, {"add", true}
    };

    const Arguments TYPES{
            fromPoint()(0, "screen"),
            fromPoint()(1, "minimap"),
            fromPoint()(2, "screen2"),
            fromEnum(QUEUED_OPTIONS)(3, "queued"),
            fromEnum(CONTROL_GROUP_ACT_OPTIONS)(4, "control_group_act"),
            fromScalar(10)(5, "control_group_id"),
            fromEnum(SELECT_POINT_ACT_OPTIONS)(6, "select_point_act"),
            fromEnum(SELECT_ADD_OPTIONS)(7, "select_add"),
            fromEnum(SELECT_UNIT_ACT_OPTIONS)(8, "select_unit_act"),
            fromScalar(500)(9, "select_unit_id"),
            fromEnum(SELECT_WORKER_OPTIONS)(10, "select_worker"),
            fromScalar(10)(11, "build_queue_id"),
            fromScalar(500)(12, "unload_id")
    };

    const Arguments RAW_TYPES{
            fromPoint()(0, "world"),
            fromEnum(QUEUED_OPTIONS)(1, "queued"),
            fromUnitTag(512, 512)(2, "unit_tags"),
            fromUnitTag(1, 512)(3, "target_unit_tag")
    };

    const std::unordered_map<FunctionSignature, std::vector<ArgumentType>> FUNCTION_TYPES{
            {noop, {}},
            {moveCamera, {TYPES.minimap}},
            {selectPoint, {TYPES.select_point_act, TYPES.screen}},
            {selectRect, {TYPES.select_add, TYPES.screen, TYPES.screen2}},
            {selectUnit, {TYPES.select_unit_act, TYPES.select_unit_id}},
            {controlGroup, {TYPES.control_group_act, TYPES.control_group_id}},
            {selectIdleWorker, {TYPES.select_worker}},
            {selectArmy, {}},
            {selectWarpGates, {}},
            {selectLarva, {}},
            {unload, {}},
            {cmdQuick, {}},
            {cmdScreen, {}},
            {cmdMiniMap, {}},
            {buildQueue, {TYPES.select_add}}
    };

    const auto ALWAYS = [](SC2APIProtocol::Observation const&) { return true; };

    struct Function{
        std::string name;
        int id{};
        std::optional<int> ability_id{}, general_id{};
        FunctionSignature function_type;
        std::vector<ArgumentType> args;
        std::function<bool(SC2APIProtocol::Observation const&)> avail_fn{ALWAYS};
        bool raw{false};
    };

    static Function fromUIFunc(int id_, std::string const& name, FunctionSignature function_type,
                        std::function<bool(SC2APIProtocol::Observation const&)> avail_fn=ALWAYS){
        return {name, id_, 0, 0, function_type, FUNCTION_TYPES.at(function_type), avail_fn, false};
    }

    static Function fromAbility(int id_, std::string const& name, FunctionSignature function_type,
                               int ability_id, int general_id=0){
        return {name, id_, ability_id, general_id, function_type, FUNCTION_TYPES.at(function_type), nullptr, false};
    }

    static Function fromSpec(int id_, std::string const& name, std::vector<ArgumentType> const& args){
        return {name, id_, std::nullopt, std::nullopt, nullptr, args, nullptr, false};
    }

    struct Functions{
        Function noop, move_camera, select_point, select_rect, select_control_group, select_unit, select_idle_worker,
                select_army, select_warp_gates, select_larva, unload, build_queue;
    };

    const Functions FUNCTIONS{
            fromUIFunc(0, "no_op", noop),
            fromUIFunc(1, "move_camera", moveCamera),
            fromUIFunc(2, "select_point", selectPoint),
            fromUIFunc(3, "select_rect", selectRect),
            fromUIFunc(4, "select_control_group", controlGroup),
            fromUIFunc(5, "select_unit", selectUnit, [](auto const& obs) { return obs.ui_data().has_multi(); }),
            fromUIFunc(6, "select_idle_worker", selectIdleWorker,
                       [](auto const& obs) { return obs.player_common().idle_worker_count() > 0; }),
            fromUIFunc(7, "select_army", selectArmy,
                       [](auto const& obs) { return obs.player_common().army_count() > 0; }),
            fromUIFunc(8, "select_warp_gates", selectWarpGates,
                       [](auto const& obs) { return obs.player_common().warp_gate_count() > 0; }),
            fromUIFunc(9, "select_larva", selectLarva,
                       [](auto const& obs) { return obs.player_common().larva_count() > 0; }),
            fromUIFunc(10, "unload", unload, [](auto const& obs) { return obs.ui_data().has_cargo(); }),
            fromUIFunc(11, "build_queue", buildQueue, [](auto const& obs) { return obs.ui_data().has_production(); }),
    };

    struct ValidActions{
        Arguments types;
        Functions functions;
    };

}