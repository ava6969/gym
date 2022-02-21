//
// Created by dewe on 5/7/21.
//

#include "dict.h"

namespace gym::space{
    ADict::ADict(NamedSpaces spaces) :
    m_NamedSpaces(move(spaces)){

       for(auto const& [key, value] : m_NamedSpaces){
           m_Keys.push_back(key);
           m_Spaces.push_back(value.get());
           value->setName(key);
       }
    }

    Shape ADict::flatten() const {

        std::vector<Shape> shapes;

        for(auto const& space: m_Spaces)
        {
            if(auto dict = dynamic_cast<ADict*>(space))
            {
                shapes.push_back(dict->flatten());
            }else
            {
//                shapes.push_back(space->m_Shape);
            }

        }

        Shape result{{0}, c10::kFloat};
        for(auto const& s: shapes)
            result.m_Axis[0] += ::gym::space::flatten(s.m_Axis);

        return result;
    }

    void ADict::merge(SpaceMap& outShape,
                      const SpaceMap & shape2,
                      string const& prefix) {

        for(auto& entry: shape2)
        {
            outShape[prefix + "/" + entry.first] = entry.second;
        }
    }
}