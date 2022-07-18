//
// Created by dewe on 7/16/22.
//

#include <numeric>
#include <tabulate/table.hpp>
#include "stopwatch.h"

namespace sc2{

    StopWatchContext::StopWatchContext(StopWatch &stopwatch,
                                       const std::string &name):sw(stopwatch) {
        sw.push(name);
    }

    void StopWatchContext::exit(){
        sw.add( sw.pop(),
                std::chrono::duration<float>( std::chrono::high_resolution_clock::now() - start ).count() );
    }

    StopWatch::StopWatch(bool enabled, bool trace){

        if(trace){
            this->trace();
        }else if( enabled ){
            this->enable();
        }else{
            this->disable();
        }
    }

    StopWatch* StopWatch::instance(){
        if(inst)
            return inst.get();
        inst.reset(new StopWatch(false));
        return inst.get();
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
            auto percent = 100* v.Sum() / ( std::max<float>(total, 1) );
            if( percent > threshold){
                table.add_row({
                    k, std::to_string(percent), std::to_string(v.Sum()), std::to_string(v.avg()),
                    std::to_string(v.dev()), std::to_string(v.Min()), std::to_string(v.Max()),
                    std::to_string(v.Num()) });
            }
        }
        return table.str();
    }


    TracingStopWatchContext::TracingStopWatchContext(StopWatch &stopwatch,
                                                     const std::string &name): StopWatchContext(stopwatch, name) {
    }

    void TracingStopWatchContext::enter() {
        StopWatchContext::enter();
        log( ">>> " +  sw.cur_stack() );
    }

    void TracingStopWatchContext::exit() {
        auto tm = std::chrono::duration<float>( std::chrono::high_resolution_clock::now() - start ).count();
        std::stringstream ss;
        ss << "<<< " <<  sw.cur_stack() << ": " << tm << "f secs.";
        log(ss.str());
        StopWatchContext::exit();
    }

}