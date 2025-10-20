#ifndef SIGNAL_H
#define SIGNAL_H

#include <string>
#include <memory>

enum class SignalType {
    INPUT,
    OUTPUT,
    WIRE,
    REG
};

class Signal {
public:
    Signal(const std::string& name, int width, SignalType type, bool is_signed = false);

    std::string getName() const { return name_; }
    int getWidth() const { return width_; }
    SignalType getType() const { return type_; }
    bool isSigned() const { return is_signed_; }

    // Get bit slice
    std::string getSlice(int high, int low) const;
    std::string getBit(int index) const;

    // Get declaration string
    std::string getDeclaration() const;

    // Get usage string
    std::string getUsage() const { return name_; }

private:
    std::string name_;
    int width_;
    SignalType type_;
    bool is_signed_;
};

using SignalPtr = std::shared_ptr<Signal>;

#endif // SIGNAL_H
