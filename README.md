# re-future
[![npm version][npm-image]][npm-uri]
[![Build Status][travis-image]][travis-uri]
[![Coverage Status][coveralls-image]][coveralls-uri]
[![Known Vulnerabilities][snyk-image]][snyk-uri]

ReasonML library to provide `Future` and `ResultFuture` monads as an alternative to Js.Promise.

The `Future` is the most simple unit of evaluable value which can resolve at some point in the future. `ResultFuture` is a future that eventually contains `Belt.Result.t` result inside. This is the most useful replacement for Js.Promise because Js.Promise also contains the possibility of resolving to Ok value or rejection to Error value.

Special focus has been given to provide complete and yet compact set of utility functions to work with both Future and ResultFuture entities. Besides test coverage this library also contains a very basic memory performance test to ensure that memory gets freed (garbage collected) when Future has used the binded functions and evaluation is done.

## Install
```sh
npm install @rejs/re-future
```

Edit to `bsconfig.json`:

```
"bs-dependencies": [..., "@rejs/re-future", ...],
```

This library is namespaced; to simplify access do:

```reason
module ResultFuture = RejsReFuture.ResultFuture;
// or
open RejsReFuture;
```
Or you can open globally in your `bsconfig.json`:
```
"bsc-flags": [ "-open RejsReFuture" ]
```

## SDK Documentation and Examples

* [Future](./doc/Future.md)
* [ResultFuture](./doc/ResultFuture.md)

More usage examples also in the \_\_tests\_\_/

The most compact references of `Future` and `ResultFuture` are the interface files:
* [Future.rei](./src/Future.rei)
* [ResultFuture.rei](./src/ResultFuture.rei)

[npm-image]: https://img.shields.io/npm/v/@rejs/re-future.svg
[npm-uri]: https://www.npmjs.com/package/@rejs/re-future
[travis-image]: https://travis-ci.org/seprich/re-future.svg?branch=master
[travis-uri]: https://travis-ci.org/seprich/re-future
[coveralls-image]: https://coveralls.io/repos/github/seprich/re-future/badge.svg?branch=master
[coveralls-uri]: https://coveralls.io/github/seprich/re-future?branch=master
[snyk-image]: https://snyk.io/test/github/seprich/re-future/badge.svg
[snyk-uri]: https://snyk.io/test/github/seprich/re-future
