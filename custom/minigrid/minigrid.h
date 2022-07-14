#pragma once
//
// Created by dewe on 6/3/22.
//

#include "gym_.h"
#include "grid.h"
#include "common.h"


using mg::WorldObj;


namespace gym {

class MiniGridEnv : public Env< std::unordered_map<std::string, cv::Mat>, int>{

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

        void render() override;
        cv::Mat _render(bool highlight=true, int tile_size=mg::TILE_PIXELS);
        void setAgentSize(int agent_sz) { agentViewSize = agent_sz; }
        inline auto wordCount() { return missionWordDictionary.size(); }
        inline auto getWidth() { return width; }
        inline auto getHeight() { return height; }
        inline void setTileSize(int ts) { tile_size = ts; }
        inline auto tileSize() const { return tile_size; }
        cv::Mat getObsRender(cv::Mat const& obs, int tile_size= mg::TILE_PIXELS / 2);

protected:
        mg::Grid grid;
        std::optional<std::string> mission{};
        static inline std::mutex staticDataMtx{};
        static inline  std::unordered_map<std::string, int> missionWordDictionary{};
        std::optional<int> agent_dir{};
        std::optional<mg::Point> agent_pos{};
        std::optional<std::vector<int>> missionCache;
        int tile_size=mg::TILE_PIXELS;

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

        template<class T>
        T rand( T low, T high);

        inline bool rand_bool(){
            return this->_np_random.randint(0, 2) == 0;
        }

        inline auto rand_color(){
            std::vector iterable( std::begin(mg::COLOR_NAMES), std::end(mg::COLOR_NAMES) );
            return iterable[ _np_random.randint<int>(0, static_cast<int>(iterable.size())) ];
        }

        template<class T>
        T rand(std::vector<T> const& iterable){
                return iterable[_np_random.randint<int>(0, static_cast<int>(iterable.size()))];
        }

        template<class T, class OutT=T, class ... Args>
        std::vector<T> rand_subset(std::vector<T> a, int num_elem){
            try{
                std::vector<T> out;
                out.reserve(num_elem);
                while ( a.size() < num_elem){
                    auto elem = rand(a);
                    std::erase(a, elem);
                    out.push_back(elem);
                }
                return out;
            } catch (std::out_of_range const & err) {
                std::stringstream ss;
                ss << num_elem << " > size of list --> " << a.size() << "\n";
                throw std::out_of_range(ss.str());
            }
        }

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

        std::vector<int> tokenizeMission();
    };

        ostream& operator<<(ostream&os, MiniGridEnv const& env);

}