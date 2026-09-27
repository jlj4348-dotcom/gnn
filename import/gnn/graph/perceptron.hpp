#pragma once

#include "Node.hpp"


namespace gnn::graph {


class Perceptron : public Node {
private:
	activation_type act_type;

public:
	Perceptron(entity_id id, node_role role = node_role::HIDDEN)
		: Node(std::move(id), node_type::PERCEPTRON, role), act_type(activation_type::RELU) {
		//constructor
	};
	~Perceptron() = default;

	activation_type get_act_type() const {
		return act_type;
	};

	void set_act_type(activation_type act_type) {
		this->act_type = act_type;
	};
};


}