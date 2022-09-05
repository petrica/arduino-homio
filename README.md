# Homio Library

[![Coverage Status](https://coveralls.io/repos/github/petrica/arduino-homio/badge.svg?branch=main)](https://coveralls.io/github/petrica/arduino-homio?branch=main)

Home automation library using nRF24 and ESP32 chips.

# Concept

A RFC proposal is available here https://www.martinescu.com/posts/homio/request-for-comment/

# Helpers

Debug data using:

```c++
std::cerr << "================" << testing::PrintToString(variable); 
```