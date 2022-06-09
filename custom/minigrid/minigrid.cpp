//
// Created by dewe on 6/3/22.
//

#include "common/utils.h"
#include <boost/uuid/uuid.hpp>
#include "boost/compute/detail/sha1.hpp"
#include "common/utils.h"
#include "tabulate/table.hpp"
#include "minigrid.h"


namespace gym {
    MiniGridEnv::MiniGridEnv(Option opt) {
        if(opt.grid_size){
            assert(not opt.width and not opt.height);
            opt.width = opt.grid_size;
            opt.height = opt.grid_size;
        }

        m_ActionSpace = makeDiscreteSpace( 7 );

        if(opt.agent_view_size % 2 != 1){
            throw std::runtime_error(" opt.agentViewSize % 2 must equal 1 ");
        }

        if(opt.agent_view_size < 3){
            throw std::runtime_error(" opt.agentViewSize must >= 3 ");
        }

        agentViewSize = opt.agent_view_size;
        m_ObservationSpace = makeBoxSpace<uint8_t>(0, 255, {opt.agent_view_size, opt.agent_view_size, 3});
        m_ObservationSpace = makeDictionarySpace({{"image", m_ObservationSpace},
                                                 {"direction", makeBoxSpace<uint8_t>(0, 3, {1})},
                                                  {"mission", makeBoxSpace<int>(0, std::numeric_limits<int>::max(),
                                                                                {1})}});
        reward_range = {0, 1};

        width = opt.width.value();
        height = opt.height.value();
        max_steps = opt.max_steps;
        see_through_walls = opt.see_through_walls;

        Env<ObservationT, int>::seed( opt.seed );

    }

    MiniGridEnv::ObservationT MiniGridEnv::reset() noexcept{
        agent_pos = std::nullopt;
        agent_dir = std::nullopt;

        genGrid(width, height);
        assert( agent_pos and agent_dir);

        auto start_cell = grid.get(agent_pos->x, agent_pos->y);
        assert(not start_cell or start_cell->canOverlap());

        m_Carrying = nullptr;
        step_count = 0;
        return genObs();

    }

    size_t MiniGridEnv::hash(unsigned int size) {
        std::size_t seed = 0;
        auto img = grid.encode();

        auto get_sha1_digest = [&]() {
            boost::uuids::detail::sha1 sha1;
            sha1.process_bytes(img.data, img.rows * img.channels() * img.cols);

            std::vector<unsigned int> sha1_hash(size, 0);
//            sha1.get_digest(sha1_hash.data());

            std::stringstream sstr;
            for (std::size_t i = 0; i < size; ++i) {
                sstr << std::setfill('0') << std::setw(8) << std::hex << sha1_hash[i];
            }

            return sstr.str();
        };
        return 0;
    }

    ostream& operator<<(ostream &os, const MiniGridEnv &env) {
        using namespace std::string_literals;
        tabulate::Table table;

        std::unordered_map<mg::Object, std::string> OBJECT_TO_STR {
                {mg::Object::Wall, "W"},
                {mg::Object::Floor, "F"},
                {mg::Object::Door, "D"},
                {mg::Object::Key, "K"},
                {mg::Object::Ball, "A"},
                {mg::Object::Box, "B"},
                {mg::Object::Goal, "G"},
                {mg::Object::Lava, "V"},
        };
        constexpr auto OPENED_DOOR_IDS = '_';
        std::vector<std::string> AGENT_DIR_TO_STR{ ">", "v", "<", "^"};

        for(int j = 0; j < env.grid.getHeight(); j++ ){
            std::vector< std::variant<std::string, tabulate::Table> > row;
            for(int i = 0; i < env.grid.getWidth(); i++ ){
                if( i == env.agent_pos->x and j == env.agent_pos->y){
                   row.emplace_back(AGENT_DIR_TO_STR[*env.agent_dir].append(AGENT_DIR_TO_STR[*env.agent_dir]) );
                    continue;
                }

                auto c = env.grid.get(i, j);
                if( not c){
                    row.emplace_back(" " );
                    continue;
                }

                std::string res;
                if( auto cPtr = std::dynamic_pointer_cast<mg::Door>(c) ){
                    if(cPtr->isOpen())
                        row.emplace_back("__"s );
                    else if (cPtr->isLocked())
                        row.emplace_back("L"s + mg::to_string( cPtr->getColor() ) );
                    else
                        row.emplace_back("D"s + mg::to_string( cPtr->getColor() ) );
                    continue;
                }
                res = std::string( OBJECT_TO_STR[c->getType()] + mg::to_string( c->getColor() ));
                row.emplace_back( res );
            }

//            if( j < env.grid.getHeight() - 1)
                table.add_row( row ) ;
        }
        os << table;
        return os;
    }

    template<> int MiniGridEnv::rand<int>(int low, int high){
        std::uniform_int_distribution dist(low, high);
        return dist(m_Device);
    }

    template<> float MiniGridEnv::rand<float>(float low, float high){
        std::uniform_real_distribution dist(low, high);
        return dist(m_Device);
    }

    template<> mg::Point MiniGridEnv::rand< mg::Point >(int xLow, int xHigh, int yLow, int yHigh){
        return { rand<int>(xLow, xHigh), rand<int>(yLow, yHigh)};
    }

    template<> bool MiniGridEnv::rand<bool>(){
        return rand<int>(0, 1) == 0;
    }

    template<> int MiniGridEnv::rand<std::vector<int>>( std::vector<int> const& iterable ){
        return uniformRandom<1>(iterable, m_Device);
    }

    template<> float MiniGridEnv::rand<std::vector<float>>( std::vector<float> const& iterable ){
        return uniformRandom<1>(iterable, m_Device);
    }

    template<> const char* MiniGridEnv::rand< Color >( ){
        return uniformRandom<1>( std::vector( std::begin(mg::COLOR_NAMES), std::end(mg::COLOR_NAMES)), m_Device);
    }

    template<> std::vector<float> MiniGridEnv::rand<std::vector<float>>( std::vector<float> const& iterable, int n_elems ){
        return sample<false>(n_elems, iterable, m_Device);
    }

    template<> std::vector<int> MiniGridEnv::rand<std::vector<float>>( std::vector<int> const& iterable, int n_elems ){
        return sample<false>(n_elems, iterable, m_Device);
    }

    mg::Point MiniGridEnv::placeObj(WorldObj::Ptr const& obj,
                                std::optional<mg::Rect>  region,
                                std::function<bool(const MiniGridEnv*, mg::Point pos)> const& reject_fn,
                                int max_tries){

        if(not region){
            region->top_left = {0, 0};
            region->height = grid.getHeight();
            region->width = grid.getWidth();
        }else{
            region->top_left = { std::max(region->top_left.x, 0), std::max(region->top_left.y, 0)};
        }

        auto num_tries = 0;
        mg::Point pos{};
        while (true){
            if( num_tries > max_tries)
                throw std::runtime_error("RecursionError: rejection sampling failed in place_obj");
            num_tries++;

            pos = {
                rand<int>(region->top_left.x, std::min(region->top_left.x + region->width, grid.getWidth()-1)),
                rand<int>(region->top_left.y, std::min(region->top_left.y + region->height, grid.getHeight()-1))
            };

            if( grid.get(pos.x, pos.y))
                continue;

            if( pos == *agent_pos)
                continue;

            if(reject_fn and reject_fn(this, pos))
                continue;

            break;
        }

        grid.set(pos.x, pos.y, obj);
        if(obj){
            obj->resetPosition(pos);
        }
        return pos;
    }

    void MiniGridEnv::putObj(const WorldObj::Ptr &obj, mg::Point pos) {
        grid.set(pos.x, pos.y, obj);
        obj->resetPosition(pos);
    }

    mg::Point MiniGridEnv::placeAgent(std::optional<mg::Rect> region,
                     bool rand_dir,
                     int max_tries){
        agent_pos = std::nullopt;
        auto pos = placeObj(nullptr, region, nullptr, max_tries);
        agent_pos = pos;
        if( rand_dir )
            agent_dir = rand<int>(0, 3);
        return pos;
    }

    mg::Point MiniGridEnv::getViewCoord(mg::Point pos) const{
        auto [i, j] = pos;
        auto [ax, ay] = *agent_pos;
        auto [dx, dy] = dir_vec();
        auto [rx, ry] = right_vec();

        auto sz = agentViewSize;
        auto hs = floor_div(sz, 2);

        auto tx = ax + (dx * (sz-1)) - (rx * hs),
             ty = ay + (dy * (sz-1)) - (ry * hs),
             lx = i - tx,
             ly = j - ty,

    // Project the coordinates of the object relative to the top-left
    // corner onto the agent's own coordinate system
        vx = (rx*lx + ry*ly),
        vy = -(dx*lx + dy*ly);

        return {vx, vy};
    }

    std::array<mg::Point, 2> MiniGridEnv::getViewExts() const{
        mg::Point top{}, bot{};

        // Facing down
        if (*agent_dir == 0){
            top.x = agent_pos->x;
            top.y = agent_pos->y - floor_div(agentViewSize, 2);
        }
        // Facing down
        else if (*agent_dir == 1) {
            top.x = agent_pos->x - floor_div(agentViewSize, 2);
            top.y = agent_pos->y;
        }
        // Facing left
        else if(*agent_dir == 2) {
            top.x = agent_pos->x  - agentViewSize + 1;
            top.y = agent_pos->y - floor_div(agentViewSize, 2);
        }
        //Facing up
        else if (*agent_dir == 3) {
            top.x = agent_pos->x - floor_div(agentViewSize, 2);
            top.y = agent_pos->y - agentViewSize + 1;
        }
        else
            throw std::runtime_error("invalid agent direction");

        bot.x = top.x + agentViewSize;
        bot.y = top.y + agentViewSize;

        return { top, bot};
    }

    std::optional<mg::Point> MiniGridEnv::relativeCoords(mg::Point point) const{
        auto[vx, vy] = getViewCoord(point);

        if (vx < 0 or vy < 0 or vx >= agentViewSize or vy >= agentViewSize)
            return std::nullopt;

        return std::make_optional<mg::Point>({vx, vy});
    }

    bool MiniGridEnv::agentSees( mg::Point point){

        auto coordinates = relativeCoords(point);
        if(not coordinates)
            return false;

        auto[vx, vy] = *coordinates;

        auto obs = genObs();
        auto [obs_grid, _] = mg::Grid::decode(obs.at("image"));
        auto obs_cell = obs_grid.get(vx, vy);
        auto world_cell = grid.get(point);

        return obs_cell and (obs_cell->getType() == world_cell->getType());
    }

    StepResponse< std::unordered_map< std::string, cv::Mat> > MiniGridEnv::step(const int &action) noexcept {
        step_count++;

        auto reward = 0.f;
        bool done = false;

        auto fwd_pos = front_pos();
        auto fwd_cell = grid.get(fwd_pos);

        switch ( m_ActionMap[action] ) {

            case Actions::left:
                (*agent_dir)--;
                if(*agent_dir < 0)
                    *agent_dir += 4;
                break;
            case Actions::right:
                agent_dir = (*agent_dir + 1) % 4;
                break;
            case Actions::forward:
                if(not fwd_cell or fwd_cell->canOverlap())
                    agent_pos = fwd_pos;
                if( fwd_cell ){
                    if(fwd_cell->getType() == mg::Object::Goal) {
                        done = true;
                        reward = this->reward();
                    }else if(fwd_cell->getType() == mg::Object::Lava) {
                        done = true;
                    }
                }
                break;
            case Actions::pickup:
                if(fwd_cell and fwd_cell->canPickup()){
                    if( not m_Carrying ){
                        m_Carrying = fwd_cell;
                        m_Carrying->currentPosition( {-1, -1} );
                        grid.set(fwd_pos, nullptr);
                    }
                }
                break;
            case Actions::drop:
                if(not fwd_cell and m_Carrying){
                    grid.set(fwd_pos, m_Carrying);
                    m_Carrying->currentPosition(fwd_pos);
                    m_Carrying = nullptr;
                }
                break;
            case Actions::toggle:
                if(fwd_cell)
                    fwd_cell->toggle(*this, fwd_pos);
                break;
            case Actions::done:
                break;
        }

        if( step_count >= max_steps)
            done = true;

        return { genObs(), reward, done };
    }

    std::pair<mg::Grid, mg::Mask2D> MiniGridEnv::genObsGrid() {
        auto [top, _] = getViewExts();
        auto [topX, topY] = top;

        auto && _grid = grid.slice( {topX, topY, agentViewSize, agentViewSize} );

        for(int i = 0; i < (*agent_dir)+1; i++)
            _grid = _grid.rotate_left();

        mg::Mask2D visMask;
        if( not see_through_walls)
            visMask = _grid.process_vis({floor_div(agentViewSize, 2), agentViewSize-1});
        else
            visMask.resize(width, vector(height, true));

        auto _agent_pos = mg::Point{floor_div(_grid.getWidth(), 2), _grid.getHeight()-1};
        if( m_Carrying )
            _grid.set(_agent_pos, m_Carrying);
        else
            _grid.set(_agent_pos, nullptr);

        return { std::move(_grid), visMask};
    }

    unordered_map<string, cv::Mat> MiniGridEnv::genObs() {
        auto [_grid, vis_mask] = genObsGrid();
        auto image = _grid.encode(vis_mask);
        if(not mission)
            throw std::runtime_error("environments must define a textual mission string");

        return {
                {"image", image},
                {"direction", cv::Mat1i({*agent_dir}) },
                {"mission", cv::Mat1i ( tokenizeMission() ) }
        };
    }

    std::vector<int> MiniGridEnv::tokenizeMission() {

        if(missionCache)
            return *missionCache;
        std::vector<std::string> mission_split;
        if(mission)
            boost::split(mission_split, *mission, boost::is_any_of(" "));
        else
            throw std::runtime_error("cannot tokenize null mission");

        std::vector<int> result(mission_split.size());
        std::ranges::transform(mission_split, result.begin(), [this](auto const& word){
           return this->missionWordDictionary[word];
        });
        missionCache = result;
        return *missionCache;
    }

    cv::Mat MiniGridEnv::getObsRender(const cv::Mat &obs, int _tile_size) {
        auto[_grid, vis_mask] = mg::Grid::decode(obs);
        return  _grid.render(_tile_size,
                             { floor_div(agentViewSize, 2), agentViewSize-1}, 3, vis_mask);
    }

    void MiniGridEnv::render() {
        cv::imshow(*mission, _render(true));
    }

    cv::Mat MiniGridEnv::_render(bool highlight, int tile_size) {

        if (not highlight) {
            return grid.render(tile_size, *agent_pos, *agent_dir, std::nullopt);
        } else {

            auto [_, vis_mask] = genObsGrid();

            mg::Point f_vec = dir_vec(),
                    r_vec = right_vec();

            mg::Point top_left = *agent_pos + (f_vec * (agentViewSize - 1)) - (r_vec * (floor_div(agentViewSize, 2)));

            mg::Mask2D highlight_mask(width, std::vector(height, false));

            for (int vis_j = 0; vis_j < agentViewSize; vis_j++) {
                for (int vis_i = 0; vis_i < agentViewSize; vis_i++) {
                    if (not vis_mask[vis_i][vis_j])
                        continue;

                    auto [abs_i, abs_j] = top_left - (f_vec * vis_j) + (r_vec * vis_i);

                    if (abs_i < 0 or abs_i >= width)
                        continue;
                    if (abs_j < 0 or abs_j >= height)
                        continue;

                    highlight_mask[abs_i][abs_j] = true;
                }
            }

            return grid.render(tile_size, *agent_pos, *agent_dir, highlight_mask);
        }
    }

}
