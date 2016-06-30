# wrp-c

C implementation of the Web Routing Protocol

[![Build Status](https://travis-ci.org/Comcast/wrp-c.svg?branch=master)](https://travis-ci.org/Comcast/wrp-c) [![codecov.io](http://codecov.io/github/Comcast/wrp-c/coverage.svg?branch=master)](http://codecov.io/github/Comcast/wrp-c?branch=master)
[![coverity](https://img.shields.io/coverity/scan/9155.svg)](https://scan.coverity.com/projects/wrp-c)

# Building and Testing Instructions

```
mkdir build
cd build
cmake ..
make
make test
make coverage
firefox index.html
```

# Coding Formatter Settings

Please format pull requests using the following command to keep the style consistent.

```
astyle -A10 -S -f -U -p -D -c -xC90 -xL
```
