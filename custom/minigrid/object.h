#pragma once
//
// Created by dewe on 6/4/22.
//

#include "common.h"
#include "memory"
#include "optional"
#include "cassert"
#include "opencv2/opencv.hpp"
#include <boost/functional/hash.hpp>


namespace gym{
    class MiniGridEnv;
}

namespace mg{

    class WorldObj{

    public:

        struct State{
            uint8_t type, color, state;
        };

        using Ptr = std::shared_ptr<WorldObj>;

        WorldObj(Object type, Color color);

        [[nodiscard]] inline virtual bool canOverlap() const { return false; }
        [[nodiscard]] inline virtual bool canPickup() const { return false; }
        [[nodiscard]] inline virtual bool canContain() const { return false; }
        [[nodiscard]] inline virtual bool seeBehind() const { return true; }
        inline virtual bool toggle(gym::MiniGridEnv& env, Point pos)  { return false; }
        [[nodiscard]] inline virtual State encode() const { return {toIndex(type), toIndex(color), 0}; }

        [[nodiscard]] inline auto getType() const { return type; }
        [[nodiscard]] inline auto getColor() const { return color; }
        [[nodiscard]] inline auto ContainedObj() const { return contains; }

        void resetPosition(Point pos);
        void currentPosition(Point pos) { cur_pos = pos; }

        static WorldObj::Ptr decode(State const&);

        virtual void render(cv::Mat& ) = 0;

        template<class T, class ... Args>
        static WorldObj::Ptr make(Args ... args){
            return std::make_shared<T>( std::forward<Args>(args) ... );
        }
    protected:
        Object type;
        Color color;
        Ptr contains{nullptr};
        std::optional<Point> init_pos{std::nullopt};
        std::optional<Point> cur_pos{std::nullopt};
    };

    static bool operator==(mg::WorldObj::State s1, mg::WorldObj::State s2){
        return (s1.state == s2.state) and (s1.type == s2.type) and (s1.color == s2.color);
    }


    struct Goal : WorldObj{

        Goal(): WorldObj(Object::Goal, Color::Green){}
        [[nodiscard]] inline bool canOverlap() const override { return true; }
        void render(cv::Mat& ) override;

    };

    struct Floor : WorldObj{

        explicit Floor(Color color=Color::Blue): WorldObj(Object::Floor, color){}
        [[nodiscard]] inline bool canOverlap() const override { return true; }
        void render(cv::Mat& ) override;
    };

    struct Lava : WorldObj{

        Lava(): WorldObj(Object::Lava, Color::Red){}
        [[nodiscard]] inline bool canOverlap() const override { return true; }
        void render(cv::Mat& ) override;
    };

    struct Wall : WorldObj{

        explicit Wall(Color color=Color::Grey): WorldObj(Object::Wall, color){}
        [[nodiscard]] inline bool seeBehind() const override { return false; }
        void render(cv::Mat& ) override;
    };

    struct Door : WorldObj{

        explicit Door(Color color, bool is_open=false, bool is_locked=false):
                WorldObj(Object::Door, color), is_open(is_open), is_locked(is_locked) {}

        [[nodiscard]] inline bool canOverlap() const override { return is_open; }
        [[nodiscard]] inline bool seeBehind() const override { return is_open; }
        bool toggle(gym::MiniGridEnv & env, Point pos) override;
        [[nodiscard]] State encode() const override;
        void render(cv::Mat&) override;

        [[nodiscard]] inline auto isOpen() const { return is_open; }
        [[nodiscard]] inline auto isLocked() const { return is_locked; }
    private:
        bool is_open, is_locked;
    };

    struct Key: WorldObj{

        explicit Key( Color color=Color::Blue): WorldObj(Object::Key, color){}
        [[nodiscard]] inline bool canPickup() const override { return true; }
        void render(cv::Mat&) override;
    };

    struct Ball: WorldObj{

        explicit Ball( Color color=Color::Blue): WorldObj(Object::Ball, color){}
        [[nodiscard]] inline bool canPickup() const override { return true; }
        void render(cv::Mat&) override;
    };

    struct Box: WorldObj{

        explicit Box( Color color, const WorldObj::Ptr & contains=nullptr):
                WorldObj(Object::Box, color){ this->contains = contains; }
        [[nodiscard]] inline bool canPickup() const override { return true; }
        [[nodiscard]] bool toggle(gym::MiniGridEnv & env, Point pos) override ;
        void render(cv::Mat&) override;

    };
}