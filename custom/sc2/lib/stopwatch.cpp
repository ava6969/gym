//
// Created by dewe on 7/16/22.
//
#include <algorithm>
#include <ranges>
#include <numeric>
#include <tabulate/table.hpp>
#include "stopwatch.h"

namespace sc2{


    std::ostream & operator<<(std::ostream& os, Stat const& s){
        if(s.num == 0)
            os << "num=0";
        else{
            os << "sum: " << s.sum
               << ", avg: " << s.avg()
               << ", dev: " << s.dev()
               << ", min: " << s.min
               << ", max: " << s.max
               << ", num: " << s.num;
        }
        return os;
    }

    Stat Stat::parse(std::string const& s){
        if( s == "num=0"){
            return {};
        }

        std::vector<float> parts;
        auto result = split(s, ", ");
        std::ranges::transform(result, std::back_inserter(parts), [](auto const& p){
            return std::stof(split(p, ":")[1]);
        });

        return Stat{parts};
    }

    void Stat::merge(Stat const &other) {
        num += other.num;
        min = std::min(min, other.min);
        max = std::max(max, other.max);
        sum += other.sum;
        sum_sq += other.sum_sq;
    }

    Stat::Stat(float summation, float average,
               float standard_deviation, float minimum, float maximum, float number) {
        if (number > 0) {
            num = number;
            min = minimum;
            max = maximum;
            sum = summation;
            sum_sq = number * (std::pow(standard_deviation, 2) + std::pow(average, 2));
        }
    }

    StopWatchContext::StopWatchContext(StopWatch* stopwatch,
                                       const std::string &name):m_sw(stopwatch) {
        stopwatch->push(name);
    }

    void StopWatchContext::exit(){
        if(m_sw)
            m_sw->add( m_sw->pop(), elapsed(*start) );
        else
            throw std::runtime_error("StopWatchContext::exit(): m_sw pointer has been released\n");
    }

    TracingStopWatchContext::TracingStopWatchContext(StopWatch* stopwatch,
                                                     const std::string &name): StopWatchContext(stopwatch, name) {
    }

    void TracingStopWatchContext::enter() {
        StopWatchContext::enter();
        if(m_sw)
            log(LOG_STREAM(">>> " << m_sw->cur_stack() ) );
        else
            throw std::runtime_error("TracingStopWatchContext::enter(): m_sw pointer has been released\n");
    }

    void TracingStopWatchContext::exit() {
        if(m_sw){
            log(LOG_STREAM("<<< " << m_sw->cur_stack() << ": " << elapsed(*start) << "f secs."));
            StopWatchContext::exit();
        }
        else
            throw std::runtime_error("TracingStopWatchContext::exit(): m_sw pointer has been released\n");
    }

    StopWatch::StopWatch(bool enabled, bool _trace){
        if(_trace){
            this->trace();
        }else if( enabled ){
            this->enable();
        }else{
            this->disable();
        }
    }

    std::string StopWatch::cur_stack( ){
        std::stringstream ss;
        std::ranges::copy(local, std::ostream_iterator<std::string>(ss, "."));
        auto result = ss.str();
        result.pop_back();
        return result;
    }

    StopWatch StopWatch::parse(std::string const& s){
        StopWatch stopWatch;
        std::vector<std::string> lines;
        boost::split(lines, s, boost::is_any_of("/n"));

        for(auto const& line: lines){
            if(line.ends_with(' ')){
                std::vector<std::string> parts;
                boost::split(parts, line, boost::is_any_of(" "));
                auto name = parts[0];
                if( name != "%"){
                    std::vector<float> rest;
                    std::transform(std::begin(parts)+2, std::end(parts),
                                   std::back_inserter(rest), [](auto const& v){
                                return std::stof(v);
                    });
                    stopWatch._times[parts[0]].merge(Stat(rest));
                }
            }
        }
        return stopWatch;
    }

    std::string StopWatch::str(float threshold){

        if(_times.empty())
            return "";

        auto total = std::accumulate(std::begin(_times), std::end(_times), 0.f, []( auto accum, auto const& entry){
            return ( entry.first.find('.') == std::string::npos) ? entry.second.Sum() + accum : accum;
        });
        tabulate::Table table;
        table.add_row({"", "% total", "sum", "avg", "dev", "min", "max", "num"});
        for(auto const& [k, v] : _times){
            auto percent = 100* v.Sum() / ( total == 0 ? 1 : total) ;
            if( percent > threshold){
                table.add_row({
                    k, std::to_string(percent), std::to_string(v.Sum()), std::to_string(v.avg()),
                    std::to_string(v.dev()), std::to_string(v.Min()), std::to_string(v.Max()),
                    std::to_string(v.Num()) });
            }
        }
        return table.str();
    }

}