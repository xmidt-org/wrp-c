<!--
SPDX-FileCopyrightText: 2016-2021 Comcast Cable Communications Management, LLC
SPDX-License-Identifier: Apache-2.0
-->
# wrp-c

C implementation of the Web Routing Protocol

[![Build Status](https://github.com/xmidt-org/wrp-c/workflows/CI/badge.svg)](https://github.com/xmidt-org/wrp-c/actions)
[![codecov.io](http://codecov.io/github/xmidt-org/wrp-c/coverage.svg?branch=main)](http://codecov.io/github/xmidt-org/wrp-c?branch=main)
[![Coverity](https://img.shields.io/coverity/scan/23254.svg)](https://scan.coverity.com/projects/xmidt-org-wrp-c)
[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=xmidt-org_wrp-c&metric=alert_status)](https://sonarcloud.io/dashboard?id=xmidt-org_wrp-c)
[![Language Grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/xmidt-org/wrp-c.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/xmidt-org/wrp-c/context:cpp)
[![Apache V2 License](http://img.shields.io/badge/license-Apache%20V2-blue.svg)](https://github.com/xmidt-org/wrp-c/blob/main/LICENSE.txt)
[![GitHub Release](https://img.shields.io/github/release/xmidt-org/wrp-c.svg)](CHANGELOG.md)

This is a simple library that converts between c structures and msgpack encoded
structures, with a few helper functions thrown in.

## Dependencies

- [cutils](https://github.com/xmidt-org/cutils)
- [ludocode/mpack](https://github.com/ludocode/mpack)

## Building and Testing Instructions

```
meson build
meson compile -C build
cd build
ninja test coverage
firefox ./meson-logs/coveragereport/index.html
```
