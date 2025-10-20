#include "signal.h"
#include <sstream>

Signal::Signal(const std::string& name, int width, SignalType type, bool is_signed)
    : name_(name)
    , width_(width)
    , type_(type)
    , is_signed_(is_signed)
{
}

std::string Signal::getSlice(int high, int low) const {
    std::ostringstream oss;
    oss << name_ << "[" << high << ":" << low << "]";
    return oss.str();
}

std::string Signal::getBit(int index) const {
    std::ostringstream oss;
    oss << name_ << "[" << index << "]";
    return oss.str();
}

std::string Signal::getDeclaration() const {
    std::ostringstream oss;

    // Type keyword
    switch (type_) {
        case SignalType::INPUT:
            oss << "input ";
            break;
        case SignalType::OUTPUT:
            oss << "output ";
            break;
        case SignalType::WIRE:
            oss << "wire ";
            break;
        case SignalType::REG:
            oss << "reg ";
            break;
    }

    // Signed modifier
    if (is_signed_) {
        oss << "signed ";
    }

    // Width
    if (width_ > 1) {
        oss << "[" << (width_ - 1) << ":0] ";
    }

    // Name
    oss << name_;

    return oss.str();
}
