#include "gnn/graph/Layer.hpp"

using namespace gnn::graph;


void Layer::remove_perceptron(const entity_id& perceptron) {

	entity_iter it = std::find(perceptrons.begin(), perceptrons.end(), perceptron);

	if (it == perceptrons.end()) {
		return;
	}

	std::iter_swap(it, perceptrons.end() - 1);
	perceptrons.pop_back();
};
