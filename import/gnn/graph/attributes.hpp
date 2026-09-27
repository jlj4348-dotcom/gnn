#pragma once

#include <string>
#include <utility>
#include <vector>
#include <algorithm>
#include <iterator>

namespace gnn::graph {

#define SUCCESS 0

using entity_id   = std::string;
using entity_name = std::string;
using entity_iter = std::vector<entity_id>::iterator;


enum class node_type {
	UNKNOWN, PERCEPTRON, EDGE, LAYER, NETWORK
};

enum class node_role {
	INPUT, OUTPUT, HIDDEN
};

enum class activation_type {
	LINEAR, SIGMOID, TANH, RELU
};


}