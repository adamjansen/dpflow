#pragma once

#include <string>

namespace dpflow {

  /**
   * @brief A class for saying hello in multiple languages
   */
  class Greeter {
    std::string name;

  public:
    /**
     * @brief Creates a new dpflow
     * @param name the name to greet
     */
    Greeter(std::string name);

    /**
     * @brief Creates a localized string containing the greeting
     * @return a string containing the greeting
     */
    std::string greet() const;
  };

}  // namespace dpflow
