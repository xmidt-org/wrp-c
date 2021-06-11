<!--
SPDX-FileCopyrightText: 2017-2021 Comcast Cable Communications Management, LLC
SPDX-License-Identifier: Apache-2.0
-->
# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/)
and this project adheres to [Semantic Versioning](http://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [v1.1.3]
- Major re-write of the msgpack handling code that
    - Reduces calls to malloc
    - Increases test coverage
    - Does more strict boundary checking for types
    - Reduces function complexity

### Fixed
- [Issue #103](https://github.com/xmidt-org/wrp-c/issues/103) where multiple partner_ids are supported now
- [Issue #105](https://github.com/xmidt-org/wrp-c/issues/105) where memory bounds are not respected
- [Issue #108](https://github.com/xmidt-org/wrp-c/issues/105) where va_copy() could leak memory


## [v1.1.2] - 2021-05-07
- Improve the cmake files so they are easier to share
- Add a `wrp-c_ver.h` file to help with versioning.

## [v1.1.1] - 2021-05-07
- Improve cmake files so dependencies can be found and specified vs. always downloaded
- Improve the tag code to leverage the new cmake file
- Updated to use the spdx license pattern

## [v1.1.0] - 2021-05-01
- Migrate to new CI infrastructure
- Fixed memory leak
- Fixed memory leak coverity found
- Added support for Mac / Apple build
- Modified wrp dest parse function to support both source/dest parsing
- Added NULL checks for malloc
- Added 'accept' wrp field 

## [v1.0.0] - 2017-12-13
### Added
- Initial stable release.

## [v0.0.1] - 2016-06-15
### Added
- Initial creation

[Unreleased]: https://github.com/xmidt-org/wrp-c/compare/v1.1.3...HEAD
[v1.1.3]: https://github.com/xmidt-org/wrp-c/compare/v1.1.2...v1.1.3
[v1.1.2]: https://github.com/xmidt-org/wrp-c/compare/v1.1.1...v1.1.2
[v1.1.1]: https://github.com/xmidt-org/wrp-c/compare/v1.1.0...v1.1.1
[v1.1.0]: https://github.com/xmidt-org/wrp-c/compare/v1.0.0...v1.1.0
[v1.0.0]: https://github.com/xmidt-org/wrp-c/compare/40cd45f5ce6723fa8d4aaf6e66fc3e3302758ec4...v1.0.0
