//
// Created by dewe on 9/3/21.
//

#include "spaces/space.h"
#include "spaces/box.h"


namespace gym::space{

    template<>
    std::vector<float> Space::sample() {
        std::vector<float> res;

        auto box = dynamic_cast<Box<float>*>(this);
        if(box){
            for(auto& range : box->getRange()){
                std::uniform_real_distribution<float> distribution(range.low, range.high);
                res.emplace_back( distribution(m_Generator) );
            }
        }else{
            std::cerr << "Error samplying float: only allowed for box environment\n";
        }

        return res;
    }

    template<>
    int  Space::sample() {
        std::uniform_int_distribution<int64_t> distribution(0, int(m_Shape.m_Axis[0]) - 1);

        return static_cast<int>( distribution(m_Generator) );
    }

}