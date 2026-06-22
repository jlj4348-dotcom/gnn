


class Network {
private:
	size_t num_layers;


public:
	size_t get_num_layers() const {
		return num_layers;
	}
};


Network operator+(const Network& n1, const Network& n2) {
	return Network(n1.get_num_layers() + n2.get_num_layers());
}