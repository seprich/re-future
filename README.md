# re-future
[![npm version][npm-image]][npm-uri]
[![Build Status][travis-image]][travis-uri]
[![Coverage Status][coveralls-image]][coveralls-uri]
[![Known Vulnerabilities][snyk-image]][snyk-uri]

ReasonML library to provide Future and ResultFuture monads as an alternative to Js.Promise.

Special focus has been given to provide complete and yet compact set of utility functions to work with both Future and ResultFuture entities. Besides test coverage this library also contains a very basic memory performance test to ensure that memory gets freed (garbage collected) when Future has used the binded functions and evaluation is done.

### Install
```bash
npm install @seprich/re-future
```

Edit to `bsconfig.json`:

```
"bs-dependencies": [..., "@seprich/re-future", ...],
```

### Future & ReasonFuture

This library contains two modules `Future` and `ResultFuture`. The `Future` is the most simple unit of evaluable value which presumably resolves at some point in the future. `ResultFuture` is a future that eventually contains `Belt.Result.t` result inside. This is the most useful replacement for Js.Promise because Js.Promise also contains the possibility of resolving to Ok value or rejection to Error value.

This library is namespaced; to simplify access do:
```reason
module ResultFuture = SeprichReFuture.ResultFuture;
```
or if you need both at the same time:
```reason
open SeprichReFuture;
```

## Module `Future`

```reason
type t('a);  // Future.t('a) evaluates to type 'a value in future.
```

#### Creating:

- `make` - Create a Future with callback e.g.:
  ```reason
  /* Resolves to "value" after 1 second: */
  let future = Future.make(setter => Js.Global.setTimeout(() => setter("value"), 1000) |> ignore);
  ```
- `fromValue` - Directly e.g.:
  ```reason
  let future = Future.fromValue("example");   /* simple, evaluates on creation. */
  ```

#### Conversions:

- `toPromise` - To promise - resulted promise is always success:
  ```reason
  Future.fromValue(42) |> Future.toPromise |> Js.Promise.then_(value => { Js.log(value); Js.Promise.resolve(()) }) |> ignore;
  ```

#### Effect functions do not affect the value of the future

- `effect` - Simple side effect function such as logging e.g.
  ```reason
  let future: Future.t(string) = /* ... implementation */
  future                                      /* e.g. evaluates to "test" */
  -> Future.effect(value => Js.log(value))    /* does not affect value */
  -> Future...                                /* still evaluates to "test" */
  ```
- `waitEffect` - Async side effect which must be waited upon before continuing e.g.
  ```reason
  let longProcess: string => Future.t(unit) = /* ... implementation */
  future                                      /* e.g. evaluates to "test" */
  -> Future.waitEffect(value => longProcess(value))
  -> Future...                                /* still evaluates to "test" */
  ```
- `get` - Operate with final results e.g.
  ```reason
  future
  -> Future.get(value => Js.log(value));      /* Final operation */
  ```

#### Functors

- `flatMap` bind a function that returns a Future into Future:
  ```reason
  let processing: string => Future.t(string) = /* ... implementation */
  future
  -> Future.flatMap(processing)
  -> Future...  /* value is now whatever was created by `processing` from the initial value */
  ```
- `map` e.g.:
  ```reason
  future.fromValue("Simple Example")
  -> Future.map(Js.String.toUpperCase)
  -> Future.get(Js.log)               /* -> "simple example" */
  ```

#### Combining Futures

- `all` - List of Futures to Future of List:
  ```reason
  Future.all([future1, future2, future3])
  -> Future.get([value1, value2, value3] => /* ... values evaluated from the list of futures */);
  ```
- `combine2` ... `combine8` - Combine heterogenously typed futures into a Future containing a tuple of values.
  ```reason
  Future.combine2((future1, future2))
  -> Future.get((value1, value2) => /* different kind of values from each future */)
  ```

## Module `ResultFuture`

```reason
type gt('a, 'e);          // Generic type of ResultFuture, where 'a is ok value type and 'e error value type
type t('a) = gt('a, exn); // ResultFuture.t where error type is exn (recommended for typical usage)
```
Most of the functions in the module `ResultFuture` are typed with the `gt` type in order to allow maximal flexibility. However
it is recommended that when using this lib the type `t` would be preferred.

#### Creating

- `make` - Using callbacks:
  ```reason
  let resultFuture = ResultFuture.make((_resolve, reject) => {
    Js.Global.setTimeout(() => reject(Failure("oh noes")), 1000);
  });  /* A ResultFuture which resolves into error after 1 second */
  ```
- `fromValue`:
  ```reason
  ResultFuture.fromValue(42)
  -> ResultFuture.getOk(Js.log)  /* "42" */
  ```
- `fromError`:
  ```reason
  ResultFuture.fromError(Failure("again"))
  -> ResultFuture.getError(Js.log)  /* outputs error to the console */
  ```
- `fromResult`:
  ```reason
  ResultFuture.fromResult(Belt.Result.Ok("check"))
  -> ResultFuture.getOk(Js.log)  /* "check" */
  ```

#### Conversions

- `fromJsPromiseDefault`:
  ```reason
  let promise = Js.Promise.resolve("quick and dirty");
  promise
  -> ResultFuture.fromJsPromiseDefault
  -> ResultFuture.getOk(Js.log)  /* "quick and dirty" */

  let promiseErr = Js.Promise.reject(Failure("oh no"))
  promiseErr
  -> ResultFuture.fromJsPromiseDefault
  -> ResultFuture.getError(Js.log)
  ```
- `toJsPromiseDefault`:
  ```reason
  future
  -> ResultFuture.toJsPromiseDefault
  -> Js.Promise.then_(value => /* do things */)
  ```
- `fromFutureResult: Future.t(Belt.Result.t('a, 'e)) => ResultFuture.gt('a, 'e)`
- `toFutureResult: ResultFuture.gt('a, 'e) => Future.t(Belt.Result.t('a, 'e))`

#### Effect functions do not affect the value

- `effectOk`, `effectError`, `effectResult` - execute simple side effect function. Similar to `Future.effect`.
   See `src/ResultFuture.rei` for signatures and unit tests for usage examples.
- `waitEffectOk`, `waitEffectError`, `waitEffectResult` - Similar to `Future.waitEffect`.
   See `src/ResultFuture.rei` for signatures and unit tests for usage examples.
- `getOk`
- `getError`
- `getResult`

#### Functors

- `flatMapOk`
- `flatMapError`
- `flatMapResult`
- `mapOk`
- `mapError`
- `mapOkResult`
- `mapErrorResult`
- `mapResult`

#### Combining ResultFutures

- `allOk`
- `allToFuture`
- `combineOk2` ... `combineOk8`
- `mapResult2` ... `mapResult8`



[npm-image]: https://img.shields.io/npm/v/@seprich/re-future.svg
[npm-uri]: https://www.npmjs.com/package/@seprich/re-future
[travis-image]: https://travis-ci.org/seprich/re-future.svg?branch=master
[travis-uri]: https://travis-ci.org/seprich/re-future
[coveralls-image]: https://coveralls.io/repos/github/seprich/re-future/badge.svg?branch=master
[coveralls-uri]: https://coveralls.io/github/seprich/re-future?branch=master
[snyk-image]: https://snyk.io/test/github/seprich/re-future/badge.svg
[snyk-uri]: https://snyk.io/test/github/seprich/re-future
