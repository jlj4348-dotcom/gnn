#pragma once

#include "attributes.hpp"
#include "node.hpp"


namespace gnn::graph {


class perceptron : public node {
private:
	activation_type act_type;

	entity_id layer_id = "";

public:
	perceptron(entity_id id, entity_name name, node_role role, activation_type act_type)
		: node(std::move(id), std::move(name), node_type::PERCEPTRON, role), act_type(act_type) {
		//constructor
	};
	perceptron(entity_id id, entity_name name, node_role role, activation_type act_type, entity_id layer_id)
		: node(std::move(id), std::move(name), node_type::PERCEPTRON, role), act_type(act_type), layer_id(std::move(layer_id)) {
		//constructor
	};
	~perceptron() = default;

	activation_type get_act_type() const {
		return act_type;
	};
	const entity_id& get_layer_id() const {
		return layer_id;
	};

	void set_act_type(activation_type act_type) {
		this->act_type = act_type;
	};
	void set_layer_id(const entity_id& layer_id) {
		this->layer_id = layer_id;
	};
};


}