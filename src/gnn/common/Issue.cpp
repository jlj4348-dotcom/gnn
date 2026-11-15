#include "gnn/common/Issue.hpp"

namespace gnn {

const char* toString(Severity severity) {
    switch (severity) {
        case Severity::Info: return "info";
        case Severity::Warning: return "warning";
        case Severity::Error: return "error";
    }
    return "unknown";
}

std::string formatIssue(const Issue& issue) {
    std::string text = "[";
    text += toString(issue.severity);
    text += "] ";
    text += issue.code;
    if (!issue.message.empty()) {
        text += ": ";
        text += issue.message;
    }
    if (!issue.targets.empty()) {
        text += " (대상: ";
        for (std::size_t i = 0; i < issue.targets.size(); ++i) {
            if (i > 0) text += ", ";
            text += std::to_string(issue.targets[i]);
        }
        text += ")";
    }
    return text;
}

}  // namespace gnn
