#include "gnn/common/GraphKind.hpp"

namespace gnn {

const char* toString(GraphKind kind) {
    switch (kind) {
        case GraphKind::Design: return "design";
        case GraphKind::Training: return "training";
        case GraphKind::Inference: return "inference";
    }
    return "unknown";
}

bool parseGraphKind(const std::string& text, GraphKind& kind) {
    if (text == "design") kind = GraphKind::Design;
    else if (text == "training") kind = GraphKind::Training;
    else if (text == "inference") kind = GraphKind::Inference;
    else return false;
    return true;
}

}  // namespace gnn
