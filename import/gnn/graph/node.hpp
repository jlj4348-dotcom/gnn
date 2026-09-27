#pragma once

#include "attributes.hpp"


namespace gnn::graph {

class node {
private:
	entity_id id;
	entity_name name;
	
	node_type type;
	node_role role;

	std::vector<entity_id> edges;

public:
	node(entity_id id, entity_name name, node_type type = node_type::UNKNOWN, node_role role = node_role::HIDDEN)
		: id(std::move(id)), name(std::move(name)), type(type), role(role) {
		//constructor
	};
	~node() = default;

	const entity_id& get_id() const {
		return id;
	};
	const entity_name& get_name() const {
		return name;
	};
	const node_type get_type() const {
		return type;
	};
	const node_role get_role() const {
		return role;
	};
	const std::vector<entity_id>& get_edges() const {
		return edges;
	};

	void set_name(const entity_name& name) {
		this->name = name;
	};
	void set_role(const node_role role) {
		this->role = role;
	};
	void add_edge(const entity_id& edge) {
		edges.push_back(edge);
	};
	void remove_edge(const entity_id& edge);
	void remove_all_edges() {
		edges.clear();
	};
};

}