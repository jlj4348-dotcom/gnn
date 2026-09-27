#include "gnn/graph/Graph.hpp"

using namespace gnn::graph;


void Graph::remove_node(const entity_id& node) {

	entity_iter it = std::find(nodes.begin(), nodes.end(), node);

	if (it == nodes.end()) {
		return;
	}

	std::iter_swap(it, nodes.end() - 1);
	nodes.pop_back();
};


int Graph::add_perceptron_to_layer(const entity_id& node, const entity_id& layer) {

	return SUCCESS;
}

int Graph::remove_perceptron_from_layer(const entity_id& node) {

	return SUCCESS;
}

int Graph::group_perceptrons(const entity_id nodes[]) {

	return SUCCESS;
}

int Graph::ungroup_layer(const entity_id& layer) {

	return SUCCESS;
}
