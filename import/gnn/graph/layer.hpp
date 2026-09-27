#pragma once

#include "Node.hpp"


namespace gnn::graph {


class Layer : public Node {
private:
	std::vector<entity_id> perceptrons;

public:
	Layer(entity_id id, node_role role = node_role::HIDDEN)
		: Node(std::move(id), node_type::LAYER, role) {
		//constructor
	}
	~Layer() = default;

	const std::vector<entity_id>& get_perceptrons() const {
		return perceptrons;
	};

	void add_perceptron(const entity_id& perceptron) {
		perceptrons.push_back(perceptron);
	};
	void remove_perceptron(const entity_id& perceptron);
	void remove_all_perceptrons() {
		perceptrons.clear();
	};
};


}