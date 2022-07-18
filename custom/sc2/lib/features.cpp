//
// Created by dewe on 7/15/22.
//

#include "features.h"


namespace sc2{

    AgentInterfaceFormat::AgentInterfaceFormat(Option const& _opt):opt(_opt){

        if( not(opt.feature_dimensions or opt.rgb_dimensions or opt.use_raw_units)){
            throw std::runtime_error("Must set either the feature layer or rgb dimensions, "
                                     "or use raw units.");
        }

        if(opt.action_space){

        }else{

        }

        if(opt.raw_resolution){
            opt.raw_resolution = {};
        }

        if(opt.use_raw_actions){

        }

        if(opt.rgb_dimensions and
        opt.rgb_dimensions->screen().x() < opt.rgb_dimensions->minimap().x() or
        opt.rgb_dimensions->screen().y() < opt.rgb_dimensions->minimap().y() ){
            throw std::runtime_error( "RGB Screen (%s) can't be smaller than the minimap (%s)." );
        }

        action_dimensions = (opt.action_space == ActionSpace::Features) ?
                opt.feature_dimensions.value() :
                opt.rgb_dimensions.value();

    }
}