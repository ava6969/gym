//
// Created by dewe on 6/3/22.
//
#include "rendering.h"
#include "grid.h"

namespace mg{


    Grid::Grid(const Grid &_clone): Grid(_clone.width, _clone.height) {}

    Grid::Grid(Grid && moved) noexcept {
        width = moved.width;
        height = moved.height;
        grid = std::move(moved.grid);
    }

    Grid& Grid::operator=(const Grid &_clone) {
        *this = Grid(_clone);
        return *this;
    }

    Grid& Grid::operator=(Grid && moved) noexcept{
        this->width = moved.width;
        this->height = moved.height;
        this->grid = std::move(moved.grid);
        return *this;
    }

    bool Grid::contains(const WorldObj::Ptr& key) {
        return std::ranges::find_if(grid, [&key](auto& e){ return e.get() == key.get(); }) != std::end(grid);
    }

    bool Grid::contains(std::pair< std::optional<Color>, int> const& key ) {
        return std::ranges::find_if(grid, [&key](auto& e) -> bool {
            return (e->getColor() == key.first and toIndex( e->getType() ) == key.second) or
                    (!key.first.has_value() and key.second == toIndex( e->getType() ) );
        }) != std::end(grid);
    }

    bool Grid::operator==(Grid const& other) const{
        auto grid1 = encode();
        auto grid2 = other.encode();
        return std::equal(grid1.begin<uchar>(), grid1.end<uchar>(), grid2.begin<uchar>());
    }

    void Grid::vert_wall(int x, int y, std::optional<int> length,
                         std::function<WorldObj::Ptr()> const& obj_type ) {
        auto N = length.value_or( height - y);
        for(int j = 0; j < N; j++)
            set(x, y + j, obj_type());
    }

    void Grid::horz_wall(int x, int y, std::optional<int> length, std::function<WorldObj::Ptr()> const& obj_type) {
        auto N = length.value_or( width - x);
        for(int i = 0; i < N; i++)
            set(x+i, y, obj_type());
    }

    Grid Grid::rotate_left() const {
        Grid grid_(height, width);
        for(int j = 0; j < width; j++){
            for(int i = 0; i < height; i++){
                grid_.set(j, grid_.height - 1 - i, get(i, j));
            }
        }
        return grid_;
    }

    void Grid::wall_rect(const Rect &r) {
        auto [p, w, h] = r;
        auto [x, y] = p;
        auto wall = []() -> WorldObj::Ptr { return std::make_unique<Wall>(); };
        horz_wall(x, y, w, wall);
        horz_wall(x, y+h-1, w, wall);
        vert_wall(x, y, h, wall);
        vert_wall(x+w-1, y, h, wall);
    }

    Grid Grid::slice(const Rect &region) const {
        auto [p, w, h] = region;
        auto [topX, topY] = p;

         Grid _grid(w, h);
         for(int j = 0; j < h; j++){
             for(int i = 0; i < w; i++){
                 auto x = topX + i;
                 auto y = topY + j;

                 WorldObj::Ptr v;

                 if (x >= 0 and x < width and y >= 0 and y < height)
                    v = get(x, y);
                 else
                    v = std::shared_ptr<Wall>();

                 _grid.set(i, j, v);
             }
         }
         return _grid;
    }

    cv::Mat Grid::render( int tile_size, Point agent_pos, int agent_dir,
                       std::optional<Mask2D>  highlight_mask ) const {

        if(not highlight_mask)
            highlight_mask = Mask2D(width, std::vector(height, false) );

        auto width_px = width*tile_size;
        auto height_px = height*tile_size;

        cv::Mat img = cv::Mat::zeros(height_px, width_px, CV_8UC3);
        for(int j = 0; j < height; j++){
            for(int i = 0; i < width; i++){
                auto cell = get(i, j);
                auto agent_here = (agent_pos == Point{i, j});

                auto tile_img = render_tile(cell,
                                            agent_here ? std::optional(agent_dir) : std::nullopt,
                                            (*highlight_mask)[i][j],
                                            tile_size);
                auto ymin = j * tile_size;
                auto ymax = (j+1) * tile_size;
                auto xmin = i * tile_size;
                auto xmax = (i+1) * tile_size;

                tile_img.copyTo( img( cv::Range(ymin, ymax), cv::Range(xmin, xmax) ) );
            }
        }

        cv::cvtColor(img, img, cv::COLOR_RGB2BGR);
        return img;

    }


    cv::Mat Grid::render_tile(WorldObj::Ptr const& obj,
                           std::optional<int> agent_dir,
                           bool highlight,
                           int tile_size,
                           int subdivs) {

//        auto key = std::make_tuple(agent_dir, highlight, tile_size);
//        auto cacheKey = obj ? std::pair( std::make_optional(obj->encode()), key ): std::pair( std::nullopt, key );
//        if( Grid::tile_cache.contains(cacheKey) )
//            return Grid::tile_cache[cacheKey];

        cv::Mat img = cv::Mat::zeros( tile_size*subdivs, tile_size*subdivs, CV_8UC3 );

        fill_coords(img, point_in_rect({0, 0.031}, {0, 1}), {100, 100, 100} );
        fill_coords(img, point_in_rect({0, 1}, {0, 0.031}), {100, 100, 100} );

        if(obj)
            obj->render(img);

        if( agent_dir ){
            auto tri_fn = point_in_triangle( {0.12, 0.19}, {0.87, 0.50}, {0.12, 0.81});
            auto theta = 0.5*M_PI* double(agent_dir.value());
            auto tri_fn2 = rotate_fn( tri_fn, {0.5, 0.5}, theta);
            fill_coords(img, tri_fn2, {255, 0, 0});
        }

        if(highlight){
            highlight_img(img);
        }

        downsample(img, subdivs);

//        Grid::tile_cache[cacheKey] = img;
        return img;
    }

    cv::Mat Grid::encode( std::optional<Mask2D> vis_mask) const{

        if(not vis_mask){
            vis_mask = Mask2D(width, std::vector(height, true));
        }

        cv::Mat array = cv::Mat::zeros(width, height, CV_8UC3);

        for(int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                if( (*vis_mask)[i][j] ){
                    auto v = get(i, j);

                    auto& ptr = array.at<cv::Vec3b>(i, j);
                    if(not v){
                        ptr = {toIndex(Object::Empty), 0, 0 };
                    }else{
                        auto state = v->encode();
                        ptr = { state.type, state.color, state.state };
                    }
                }
            }
        }
        return array;
    }

    std::pair<Grid, Mask2D> Grid::decode( cv::Mat const& array){
        auto sz = array.size;
        auto[width, height] = std::tie(sz[0], sz[1]);
        auto channels = array.channels();

        Mask2D visMask( width, std::vector(height, true) );
        Grid grid_(width, height);
        for(int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                auto a = array.at<cv::Vec3b>(i, j);
                WorldObj::State state{a[0], a[1], a[2] };
                auto v = WorldObj::decode(state);
                grid_.set(i, j, v);
                visMask[i][j] = (state.type != toIndex(Object::Unseen));
            }
        }

        return {grid_, visMask};

    }

    Mask2D Grid::process_vis(Point agent_pos) {
        Mask2D mask(width, std::vector(height, false));
        mask[agent_pos.x][agent_pos.y] = true;

        for (int j = this->height - 1; j >= 0; j--) {
            for (int i = 0; i < this->width - 1; i++) {
                if (not mask[i][j]) {
                    continue;
                }

                auto cell = this->get(i, j);
                if (cell and not cell->seeBehind()) {
                    continue;
                }

                mask[i + 1][j] = true;
                if (j > 0) {
                    mask[i + 1][j - 1] = true;
                    mask[i][j - 1] = true;
                }
            }

            for (int i = this->width - 1; i > 0; i--) {
                if( not mask[i][j])
                    continue;

                auto cell = this->get(i, j);
                if (cell and not cell->seeBehind()) {
                    continue;
                }

                mask[i - 1][j] = true;
                if (j > 0) {
                    mask[i - 1][j - 1] = true;
                    mask[i][j - 1] = true;
                }
            }
        }

        for(int j =0; j < this->height; j++){
            for(int i =0; i < this->width; i++){
                if( not mask[i][j])
                    this->set(i, j, nullptr);
            }
        }

        return mask;

    }


    std::size_t Grid::KeyHasher::operator()(const Grid::TileCacheT &k) const
    {
        using boost::hash_value;
        using boost::hash_combine;

        // Start with a hash value of 0    .
        std::size_t seed = 0;

        // Modify 'seed' by XORing and bit-shifting in
        // one member of 'Key' after the other:

        auto s = k.first.value_or( WorldObj::State{});

        hash_combine(seed, hash_value(s.color));
        hash_combine(seed, hash_value(s.type));
        hash_combine(seed, hash_value(s.state));
        hash_combine(seed,hash_value( std::get<0>(k.second).value_or(0) ) );
        hash_combine(seed,hash_value( std::get<1>(k.second) ) );
        hash_combine(seed,hash_value( std::get<2>(k.second) ) );

        // Return the result.
        return seed;
    }

}