#pragma once

#include "Attributes.hpp"


namespace gnn::graph {

class Node {
private:
	entity_id id;
	entity_name name = "";
	entity_id parent = "";
	
	node_type type;
	node_role role;

	std::vector<entity_id> edges;

public:
	Node(entity_id id, node_type type = node_type::UNKNOWN, node_role role = node_role::HIDDEN)
		: id(std::move(id)), type(type), role(role) {
		//constructor
	};
	~Node() = default;

	const entity_id& get_id() const {
		return id;
	};
	const entity_name& get_name() const {
		return name;
	};
	const entity_id& get_parent() const {
		return parent;
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
	void set_parent(const entity_id& parent) {
		this->parent = parent;
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
