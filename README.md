# wrp-c

C implementation of the Web Routing Protocol

[![Build Status](https://travis-ci.com/xmidt-org/wrp-c.svg?branch=master)](https://travis-ci.com/xmidt-org/wrp-c)
[![codecov.io](http://codecov.io/github/xmidt-org/wrp-c/coverage.svg?branch=master)](http://codecov.io/github/xmidt-org/wrp-c?branch=master)
[![Coverity](https://img.shields.io/coverity/scan/9155.svg)]("https://scan.coverity.com/projects/comcast-wrp-c)
[![Apache V2 License](http://img.shields.io/badge/license-Apache%20V2-blue.svg)](https://github.com/xmidt-org/wrp-c/blob/master/LICENSE.txt)
[![GitHub release](https://img.shields.io/github/release/xmidt-org/wrp-c.svg)](CHANGELOG.md)

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
