#pragma once
//
// Created by dewe on 6/3/22.
//

#include "gym_.h"
#include "grid.h"
#include "common.h"


using mg::WorldObj;


namespace gym {

class MiniGridEnv : public Env< cv::Mat, int>{

    public:

        using Ptr = std::shared_ptr<MiniGridEnv>;
        enum class Actions{
            left=0,
            right,
            forward,
            pickup,
            drop,
            toggle,
            done
        };

        struct State{
            cv::Mat image{};
            gym::Point direction{};
            std::string mission{};
        };

        struct Option{
            std::optional<int>  grid_size, width, height;
            int max_steps=100;
            bool see_through_walls=false;
            std::optional<int> seed=1337;
            int agent_view_size=7;

            explicit Option(std::optional<int> seed=1337):seed(seed){}

            inline Option& gridSize(int size) { grid_size = size; return *this; }
            inline Option& maxSteps(int x) { max_steps = x; return *this; }
            inline Option& seeThroughWalls(bool x) { see_through_walls = x; return *this; }
        };

        explicit MiniGridEnv( Option );
        MiniGridEnv()=default;

        ObservationT reset() noexcept override;

        size_t hash(unsigned int size=16);

        inline auto stepsRemaining() const { return max_steps - step_count; }

        friend ostream& operator<<(ostream&os, MiniGridEnv const& env);

        inline mg::Grid& getGrid() &{ return grid; }
        inline auto carrying() { return m_Carrying; }

        StepT step(const ActionT &action) noexcept override;

        void render(RenderType) override;

        void setAgentSize(int agent_sz) { agentViewSize = agent_sz; }

protected:
        mg::Grid grid;
        std::optional<std::string> mission{};
        static inline std::mutex staticDataMtx{};
        static inline  std::unordered_map<std::string, int> missionWordDictionary{};
        std::optional<int> agent_dir{};
        std::optional<mg::Point> agent_pos{};

        virtual void genGrid( int width, int height) = 0;

        inline float reward() const { return 1.f - 0.9f*( float(step_count) / float(max_steps) ); };

        mg::Point placeObj(WorldObj::Ptr const& obj,
                           std::optional<mg::Rect> region=std::nullopt,
                           std::function<bool(const MiniGridEnv*, mg::Point pos)> const& reject_fn=nullptr,
                           int max_tries=std::numeric_limits<int>::max());

        void putObj(WorldObj::Ptr const& obj, mg::Point  pos);

        mg::Point  placeAgent(std::optional<mg::Rect> region=std::nullopt,
                              bool rand_dir=true,
                              int max_tries=std::numeric_limits<int>::max());

        template<class T, class OutT=T, class ... Args>
        OutT rand(Args ...);

        void fillDictionary(){
            auto sz = missionWordDictionary.size();

            std::set<std::string> tokens;
            boost::split(tokens, *mission, boost::is_any_of(" "));

            for( auto const& word : tokens){
                if( not missionWordDictionary.contains(word))
                    missionWordDictionary[word] = sz++;
            }
        }

        std::vector<Actions> m_ActionMap;

private:
        int step_count{}, max_steps{};
        std::array<float, 2> reward_range{};
        int width{}, height{}, agentViewSize{};
        bool see_through_walls{};
        WorldObj::Ptr m_Carrying;

        unordered_map<string, cv::Mat> genObs();

        inline mg::Point dir_vec() const {
            assert( agent_dir >= 0 and agent_dir < 4);
            return mg::direction[ *agent_dir ];
        }

        inline mg::Point right_vec() const{
            auto[dx, dy] = dir_vec();
            return {-dy, dx};
        }

        inline mg::Point front_pos() const{
            return { dir_vec() + *agent_pos };
        }

        mg::Point getViewCoord(mg::Point pos) const;
        std::array<mg::Point, 2> getViewExts() const;
        std::optional<mg::Point> relativeCoords(mg::Point point) const;

        inline bool inView(mg::Point point) const { return relativeCoords(point).has_value(); }

        bool agentSees( mg::Point point);

        std::pair<mg::Grid, mg::Mask2D> genObsGrid();

        cv::Mat getObsRender(cv::Mat const& obs, int tile_size= mg::TILE_PIXELS / 2);

        std::vector<int> tokenizeMission();
    };

        ostream& operator<<(ostream&os, MiniGridEnv const& env);

}