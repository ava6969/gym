//
// Created by dewe on 5/7/21.
//

#pragma once

#include <memory>
#include "map"
#include "string"
#include "vector"
#include "space.h"

using std::vector;
using std::string;

namespace gym::space{
    using NamedSpaces  = std::map<string, std::shared_ptr<Space> >;

    class ADict : public Space {

    public:

        explicit ADict(NamedSpaces spaces);

        [[nodiscard]] inline
        std::vector<std::string> keys() const override { return m_Keys; };

        [[nodiscard]] inline
        Spaces spaces() const override { return m_Spaces; }

        [[nodiscard]] inline
        SpaceMap namedSpaces() const override {
            SpaceMap spaces;
            for(auto const&[k, space] : m_NamedSpaces)
                spaces[k] = space.get();
            return spaces;
        }

        [[nodiscard]] Shape flatten() const;

        [[nodiscard]] std::shared_ptr<Space> clone() override {
            NamedSpaces spaces;
            for (auto const& item: m_NamedSpaces) {
                spaces[item.first] = item.second->clone();
            }
            return std::make_shared<ADict>(spaces);
        }

        void update(std::string const& name, std::shared_ptr<Space> space){
            m_NamedSpaces[name] = space;
        }

    private:
        NamedSpaces m_NamedSpaces;

        std::vector<std::string> m_Keys;

        Spaces m_Spaces;

        void merge(SpaceMap & shape1, const SpaceMap & shape2, string const& prefix);
    };
}