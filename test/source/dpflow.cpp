#include <doctest/doctest.h>
#include <dpflow/dpflow.h>
#include <dpflow/version.h>

#include <string>

TEST_CASE("DPFlow") {
  using namespace dpflow;

  Greeter greeter("Tests");

  CHECK(greeter.greet() == "Hello, Tests!");
}

TEST_CASE("Greeter version") {
  static_assert(std::string_view(DPFLOW_VERSION) == std::string_view("1.0"));
  CHECK(std::string(DPFLOW_VERSION) == std::string("1.0"));
}
