#pragma once
//
// Created by dewe on 7/16/22.
//

#include "unordered_set"
#include "vector"


static std::unordered_set<unsigned short> contiguous_ports;

std::vector<unsigned short> pick_unused_ports(int num_ports, int retry_interval_secs=1, int retry_attempts=5);

std::vector<unsigned short> pick_contiguous_unused_ports(int num_ports, int retry_interval_secs=1, int retry_attempts=5);

std::vector<unsigned short> return_ports(std::vector<unsigned short> const& ports);