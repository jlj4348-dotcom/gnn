#include "gnn/graph/node.hpp"

using namespace gnn::graph;


void node::remove_edge(const entity_id& edge) {

	entity_iter it = std::find(edges.begin(), edges.end(), edge);

	if (it == edges.end()) {
		return;
	}

	std::iter_swap(it, edges.end() - 1);
	edges.pop_back();
};
