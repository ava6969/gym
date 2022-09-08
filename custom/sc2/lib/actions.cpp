
//
// Created by dewe on 7/15/22.
//

#include <ranges>
#include "actions.h"


namespace sc2{

    SC2APIProtocol::ActionSpatial spatial(SC2Action const& action, ActionSpace actionSpace){
        switch (actionSpace) {
            case ActionSpace::Features:
                return action.action_feature_layer();
            case ActionSpace::RGB:
                return action.action_render();
            default:
                throw std::runtime_error("Unexpected value for action_space: ActionSpace::RAW");
        }
    }

    void moveCamera(FunctionOption& opt){
        opt.minimap->assign_to(spatial(opt).mutable_camera_move()->mutable_center_minimap());
    }

    void selectPoint(FunctionOption& opt){
        auto select = spatial(opt).mutable_unit_selection_point();
        opt.screen->assign_to(select->mutable_selection_screen_coord());
        select->set_type(static_cast<SC2APIProtocol::ActionSpatialUnitSelectionPoint_Type>(*opt.select_point_act));
    }

    void selectRect(FunctionOption& opt){
        auto select = spatial(opt).mutable_unit_selection_rect();
        auto out_rect = select->mutable_selection_screen_coord()->Add();
        auto screen_rect = Rect{*opt.screen, *opt.screen2};
        screen_rect.tl().assign_to(out_rect->mutable_p0());
        screen_rect.br().assign_to(out_rect->mutable_p1());
        select->set_selection_add(*opt.select_add);
    }

    void selectUnit(FunctionOption& opt){
        auto select = opt.action.mutable_action_ui()->mutable_multi_panel();
        select->set_type(static_cast<SC2APIProtocol::ActionMultiPanel_Type>(*opt.select_unit_act));
        select->set_unit_index(*opt.select_unit_id);
    }

    void selectIdleWorker(FunctionOption&){

    }

    void selectArmy(FunctionOption&){

    }

    void selectWarpGates(FunctionOption&){

    }

    void selectLarva(FunctionOption&){

    }

    void controlGroup(FunctionOption&){

    }

    void unload(FunctionOption&){

    }

    void buildQueue(FunctionOption&){

    }

    void cmdQuick(FunctionOption&){

    }

    void cmdScreen(FunctionOption&){

    }

    void cmdMiniMap(FunctionOption&){

    }

    FactoryType fromEnum(std::unordered_map<std::string, int> const& options){
        std::vector<bool> real;
        for(auto const& v : options | std::views::values){
            real.push_back(v);
        }
        return [real](int i, std::string const& name) -> ArgumentType{
            return ArgumentType{i, name, { static_cast<uint32_t>(real.size()), },
                                [real](std::vector<int> const& a) -> bool { return real[a.at(0)]; } };
        };
    }

    FactoryType fromScalar(int value){
        return [value](int i, std::string const& name) -> ArgumentType{
            return ArgumentType{i, name, { static_cast<uint32_t>(value), },
                                [](auto const& a) -> int { return a.at(0); } };
        };
    }

    FactoryType fromPoint(){
        return [](int i, std::string const& name) -> ArgumentType{
            return ArgumentType{i, name, {0, 0},
                                [](auto const& a) -> Point { return {a.at(0), a.at(0)}; } };
        };
    }

    ArgumentType fromSpec(int id_, std::string const& name, std::array<uint32_t, 2> const& sizes){
        return ArgumentType{id_, name, sizes};
    }

    FactoryType fromUnitTag(int count, uint32_t size ){
        return [size, count](int i, std::string const& name) -> ArgumentType {
            return ArgumentType{i, name, {size, }, [count](auto const& arg) -> std::vector<int> {
                return std::vector(arg.begin()+1, arg.begin() + count);
            }};
        };
    }

}