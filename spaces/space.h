//
// Created by dewe on 4/14/21.
//

#ifndef FASTDRL_SPACE_H
#define FASTDRL_SPACE_H

#include <memory>
#include "random"
#include "map"
#include "util.h"

namespace gym::space{

    class Space;
    using Spaces   = std::vector<gym::space::Space*>;
    using SpaceMap   = std::map<std::string, Space*>;
    using SpaceHolder = std::shared_ptr<gym::space::Space>;

    class Space{

    protected:
        const Shape m_Shape{};
        std::mt19937 m_Generator{};
        std::string m_Name{"observation"};

        template<class T>
        auto sampleContinuousUniformDist(std::vector<Range<T >> const& m_Range)    {
            std::vector<T> result;

            auto&& generateInRange = [this](Range<T> range){
                std::uniform_real_distribution<float> distribution(range.low, range.high);
                return distribution(m_Generator);
            };

            std::transform(m_Range.begin(), m_Range.end(),
                           std::back_inserter(result),
                           generateInRange);

            return result;
        }

        template<class T>
        auto sampleDiscreteUniformDist(std::vector<Range<T >> const& m_Range){

            std::vector<T> result;

            auto&& generateInRange = [this](Range<T> range){
                std::uniform_int_distribution<int> distribution(range.low, range.high);
                return distribution(m_Generator);
            };

            std::transform(m_Range.begin(), m_Range.end(),
                           std::back_inserter(result),
                           generateInRange);

            return result;
        }

    public:
        Space() = default;

        explicit
        Space(Shape _shape): m_Shape(std::move(_shape)) {
            std::random_device rd;
            m_Generator = std::mt19937{rd()};
        }

        template<typename T>
        T sample();

        template<typename T> inline T* as(){
            return dynamic_cast<T*>(this);
        }
        [[nodiscard]] virtual inline
        std::vector<std::string> keys() const { return {m_Name}; };

        [[nodiscard]] virtual inline
        Spaces spaces() const { return {const_cast<Space*>(this)}; }

        [[nodiscard]] virtual inline
        SpaceMap namedSpaces() const { return {{m_Name, const_cast<Space*>(this)}}; }

        void setName(std::string const& name) {
            m_Name = name;
        }

        virtual std::shared_ptr<Space> clone() =0;

        template<class T>
        [[nodiscard]] std::shared_ptr<Space> cloneHelper() {
            return std::make_shared<T>( *(dynamic_cast< T*>(this)) );
        }

        virtual ~Space()=default;

        inline auto type() const{
            return m_Shape.m_Type;
        }

        inline auto size() const{
            return m_Shape.m_Axis;
        }
    };
}

#endif //FASTDRL_SPACE_H
