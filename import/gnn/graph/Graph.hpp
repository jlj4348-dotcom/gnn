#pragma once

#include "Node.hpp"


namespace gnn::graph {


class Graph : public Node {
private:
	std::vector<entity_id> nodes;

public:
	Graph(entity_id id, node_role role = node_role::HIDDEN)
		: Node(std::move(id), node_type::NETWORK, role) {
		//constructor
	};
	~Graph() = default;

	const std::vector<entity_id>& get_nodes() const {
		return nodes;
	};

	void add_node(const entity_id& node) {
		nodes.push_back(node);
	};
	void remove_node(const entity_id& node);
	void remove_all_nodes() {
		nodes.clear();
	};


	int add_perceptron_to_layer(const entity_id& node, const entity_id& layer);
	int remove_perceptron_from_layer(const entity_id& node);
	int group_perceptrons(const entity_id nodes[]);
	int ungroup_layer(const entity_id& layer);
};


}
