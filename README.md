# Pojobuf

[![Standard](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B20) [![License](https://img.shields.io/badge/license-MIT-blue.svg)](https://opensource.org/licenses/MIT)

A C++ header-only library for building and parsing JSON-type structures in flat buffers.

The name is a play on [protobuf](https://protobuf.dev/) combining it with [POJO](https://en.wikipedia.org/wiki/Plain_old_Java_object), where POJO is retronymmed to mean Plain Ol' JSON Object. Yes, yes. Very meta.

This library borrows a lot of ideas and code from [sajson](https://github.com/chadaustin/sajson) by Chad Austin. For an in-depth comparison see [doc/sajson.md](doc/sajson.md)

While the library can be used for JSON parsing, that is not its only goal. Pojobuf values can also hold TOML, CBOR, most of YAML, and many more self-describing data formats.

## POJO

In this library and the documentation by "POJO" we mean a Plain Ol' JSON Object. In short a tree-like value where each leaf node is one of:

* null
* a string
* a number
* a boolean

And each non-leaf node is either an array of nodes, or a string-key to value map. The key-value map is called an object as per JSON (and JavaScript) terminology.

## License

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](https://opensource.org/licenses/MIT)

This software is distributed under the MIT Software License.

See accompanying file LICENSE or copy [here](https://opensource.org/licenses/MIT).

Copyright &copy; 2012-2017 [Chad Austin](http://github.com/chadaustin)

Copyright &copy; 2026 [Borislav Stanimirov](http://github.com/iboB)
