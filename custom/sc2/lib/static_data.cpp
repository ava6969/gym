//
// Created by dewe on 9/7/22.
//
#include "ranges"
#include "util.h"
#include "static_data.h"


namespace sc2{

    StaticData::StaticData(sc_pb::ResponseData const& data):
            m_units(data.units_size()),
            m_unitStats(data.units_size()),
            m_upgrades(data.upgrades_size()),
            m_abilities(data.abilities_size()){

        for(auto const& u: data.units()){
            m_units[u.unit_id()] = u.name();
            m_unitStats.push_back(u.unit_id());
        }

        for(auto&& a: data.upgrades())
            m_upgrades[a.upgrade_id()] = std::move(a);

        for(auto const& a: data.abilities()){
            m_abilities[a.ability_id()] =  std::move(a);
            if(a.has_remaps_to_ability_id())
                m_generalAbilities.push_back(a.remaps_to_ability_id());
        }

        for(auto& a: m_abilities | std::views::values){
            lower(*a.mutable_hotkey());
        }

    }
}