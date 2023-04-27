#include <fmt/format.h>
#include <dpflow/dpflow.h>

using namespace dpflow;

Greeter::Greeter(std::string _name) : name(std::move(_name)) {}

std::string Greeter::greet() const {
      return fmt::format("Hello, {}!", name);
}
