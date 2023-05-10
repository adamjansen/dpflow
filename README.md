# Data Panel C++ library

This repo contains Data Panel's C++ library and several applications that use it.

## Features

## Usage

### Build and run the standalone target

Use the following command to build the executable

```bash
cmake -S. -Bbuild
cmake --build build
```

### Build the documentation

The documentation is built using Doxygen.
To manually build documentation, call the following command.

```bash
cmake --build build --target doxygen-docs
# view the docs
xdg-open build/docs/html/index.html
```

To build the documentation locally, you will need Doxygen, jinja2 and Pygments installed on your system.
