# wrp-c

C implementation of the Web Routing Protocol

[![Build Status](https://github.com/xmidt-org/wrp-c/workflows/CI/badge.svg)](https://github.com/xmidt-org/wrp-c/actions)
[![codecov.io](http://codecov.io/github/xmidt-org/wrp-c/coverage.svg?branch=main)](http://codecov.io/github/xmidt-org/wrp-c?branch=main)
[![Coverity](https://img.shields.io/coverity/scan/9155.svg)]("https://scan.coverity.com/projects/comcast-wrp-c)
[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=xmidt-org_wrp-c&metric=alert_status)](https://sonarcloud.io/dashboard?id=xmidt-org_wrp-c)
[![Language Grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/xmidt-org/wrp-c.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/xmidt-org/wrp-c/context:cpp)
[![Apache V2 License](http://img.shields.io/badge/license-Apache%20V2-blue.svg)](https://github.com/xmidt-org/wrp-c/blob/main/LICENSE.txt)
[![GitHub Release](https://img.shields.io/github/release/xmidt-org/wrp-c.svg)](CHANGELOG.md)


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
