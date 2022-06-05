#pragma once
//
// Created by dewe on 6/2/22.
//

#include "object.h"

namespace mg{

    struct Grid {

        using TileCacheT = std::pair< std::optional<WorldObj::State>, std::tuple< std::optional<int>, bool, int > >;

        struct KeyHasher{
            std::size_t operator()(const TileCacheT& k) const;
        };

        Grid( int width, int height): grid(height * width), width(width), height(height){
            if(width < 3 or height < 3)
                throw std::runtime_error("width and height must be >= 3 ");
        }
        Grid()=default;
        Grid( Grid const& _clone);
        Grid( Grid&& _moved) noexcept;
        Grid& operator=( Grid const& _clone);
        Grid& operator=( Grid && _moved) noexcept;

        bool contains( const WorldObj::Ptr&  key);
        bool contains( std::pair< std::optional<Color>, int> const& key );

        bool operator==(Grid const& other) const;
        bool operator!=(Grid const& other) const { return not (*this == other); }

        [[nodiscard]] inline constexpr int index(int i, int j) const{
            assert(i>=0 and i < width);
            assert(j>=0 and j < height);
            return j*width + i;
        }

        inline void set(int i, int j, WorldObj::Ptr const& v ){
            grid[ index(i, j) ] = v;
        }

        inline void set(Point pos, WorldObj::Ptr const& v ){
            grid[ index(pos.x, pos.y) ] = v;
        }

        [[nodiscard]] inline auto get(int i, int j) const{
            return  grid[ index(i, j) ];
        }

        [[nodiscard]] inline auto get(Point pos) const{
            return  grid[ index(pos.x, pos.y) ];
        }

        void horz_wall( int x, int y, std::optional<int> length=std::nullopt, std::function<WorldObj::Ptr()> const& type =
        []() -> WorldObj::Ptr { return std::make_unique<Wall>(); });
        void vert_wall( int x, int y, std::optional<int> length=std::nullopt, std::function<WorldObj::Ptr()> const& type =
        []() -> WorldObj::Ptr { return std::make_unique<Wall>(); });
        void wall_rect( Rect const& region);
        [[nodiscard]] Grid rotate_left() const;
        [[nodiscard]] Grid slice(Rect const& region) const;

        static  cv::Mat render_tile( WorldObj::Ptr const& obj,
                                 std::optional<int> agent_dir=std::nullopt,
                                 bool highlight=false,
                                 int tile_size=TILE_PIXELS,
                                 int subdivs=3);

        [[nodiscard]] cv::Mat render( int tile_size,
                                      Point agent_pos,
                                      int agent_dir,
                                      std::optional<Mask2D> highlight_mask=std::nullopt ) const;

        [[nodiscard]] cv::Mat encode( std::optional<Mask2D> vis_mask=std::nullopt) const;
        static std::pair<Grid, Mask2D> decode(cv::Mat const& array);
        Mask2D process_vis(Point agent_pos);

        [[nodiscard]] inline auto getHeight() const { return height; }
        [[nodiscard]] inline auto getWidth() const { return width; }

    private:
        int width{}, height{};
        std::vector< WorldObj::Ptr > grid;

        static inline std::unordered_map<TileCacheT, cv::Mat, KeyHasher> tile_cache={};

    };
}