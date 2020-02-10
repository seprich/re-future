# re-future
[![npm version][npm-image]][npm-uri]
[![Build Status][travis-image]][travis-uri]
[![Coverage Status][coveralls-image]][coveralls-uri]
[![Known Vulnerabilities][snyk-image]][snyk-uri]

ReasonML library to provide Future and ResultFuture monads as an alternative to Js.Promise.

Special focus has been given to provide complete and yet compact set of utility functions to work with both Future and ResultFuture entities. Besides test coverage this library also contains a very basic memory performance test to ensure that memory gets freed (garbage collected) when Future has used the binded functions and evaluation is done.

### Install
```sh
npm install @rejs/re-future
```

Edit to `bsconfig.json`:

```
"bs-dependencies": [..., "@rejs/re-future", ...],
```

### Future & ReasonFuture

This library contains two modules `Future` and `ResultFuture`. The `Future` is the most simple unit of evaluable value which presumably resolves at some point in the future. `ResultFuture` is a future that eventually contains `Belt.Result.t` result inside. This is the most useful replacement for Js.Promise because Js.Promise also contains the possibility of resolving to Ok value or rejection to Error value.

This library is namespaced; to simplify access do:
```reasonml
module ResultFuture = RejsReFuture.ResultFuture;
```
or if you need both at the same time:
```reasonml
open RejsReFuture;
```

## Module `Future`

```reasonml
type t('a);  // Future.t('a) evaluates to type 'a value in future.
```

#### Creating:

- `make` - Create a Future with callback e.g.:
  ```reasonml
  // Resolves to "value" after 1 second:
  let future = Future.make(setter => Js.Global.setTimeout(() => setter("value"), 1000) |> ignore);
  ```
- `fromValue` - Directly e.g.:
  ```reasonml
  let future = Future.fromValue("example");   // simple, evaluates on creation.
  ```

#### Conversions:

- `toPromise` - To promise - resulted promise is always success:
  ```reasonml
  Future.fromValue(42) |> Future.toPromise |> Js.Promise.then_(value => { Js.log(value); Js.Promise.resolve(()) }) |> ignore;
  ```

#### Effect functions do not affect the value of the future

- `effect` - Simple side effect function such as logging e.g.
  ```reasonml
  let future: Future.t(string) = /* ... implementation */
  future                                      // e.g. evaluates to "test"
  -> Future.effect(value => Js.log(value))    // does not affect value
  -> Future...                                // still evaluates to "test"
  ```
- `waitEffect` - Async side effect which must be waited upon before continuing e.g.
  ```reasonml
  let longProcess: string => Future.t(unit) = /* ... implementation */
  future                                      // e.g. evaluates to "test"
  -> Future.waitEffect(value => longProcess(value))
  -> Future...                                // still evaluates to "test"
  ```
- `get` - Operate with final results e.g.
  ```reasonml
  future
  -> Future.get(value => Js.log(value));      // Final operation
  ```

#### Functors

- `flatMap` bind a function that returns a Future into Future:
  ```reasonml
  let processing: string => Future.t(string) = /* ... implementation */
  future
  -> Future.flatMap(processing)
  -> Future...  /* value is now whatever was created by `processing` from the initial value */
  ```
- `map` e.g.:
  ```reasonml
  future.fromValue("Simple Example")
  -> Future.map(Js.String.toLowerCase)
  -> Future.get(Js.log)               // -> "simple example"
  ```

#### Combining Futures

- `all` - List of Futures to Future of List:
  ```reasonml
  Future.all([future1, future2, future3])
  -> Future.get([value1, value2, value3] => /* ... values evaluated from the list of futures */);
  ```
- `combine2` ... `combine8` - Combine heterogenously typed futures into a Future containing a tuple of values.
  ```reasonml
  Future.combine2((future1, future2))
  -> Future.get((value1, value2) => /* different kind of values from each future */)
  ```

## Module `ResultFuture`

```reasonml
type gt('a, 'e);          // Generic type of ResultFuture,
                          // where 'a is ok value type and 'e error value type
type t('a) = gt('a, exn); // ResultFuture.t where error type is exn (recommended for typical usage)
```
Most of the functions in the module `ResultFuture` are typed with the `gt` type in order to allow maximal flexibility. However
it is recommended that when using this lib the type `t` would be preferred.

#### Creating

- `make` - Using callbacks:
  ```reasonml
  let resultFuture = ResultFuture.make((_resolve, reject) => {
    Js.Global.setTimeout(() => reject(Failure("oh noes")), 1000);
  });  // A ResultFuture which resolves into error after 1 second
  ```
- `fromValue`:
  ```reasonml
  ResultFuture.fromValue(42)
  -> ResultFuture.getOk(Js.log)  // "42"
  ```
- `fromError`:
  ```reasonml
  ResultFuture.fromError(Failure("again"))
  -> ResultFuture.getError(Js.log)  // outputs error to the console
  ```
- `fromResult`:
  ```reasonml
  ResultFuture.fromResult(Belt.Result.Ok("check"))
  -> ResultFuture.getOk(Js.log)  // "check"
  ```

#### Conversions

- `fromJsPromiseDefault`:
  ```reasonml
  let promise = Js.Promise.resolve("quick and dirty");
  promise
  -> ResultFuture.fromJsPromiseDefault
  -> ResultFuture.getOk(Js.log)  // "quick and dirty"

  let promiseErr = Js.Promise.reject(Failure("oh no"));
  promiseErr
  -> ResultFuture.fromJsPromiseDefault
  -> ResultFuture.getError(Js.log)
  ```
- `toJsPromiseDefault`:
  ```reasonml
  let resultFuture = ResultFuture.fromValue("to js");
  resultFuture
  -> ResultFuture.toJsPromiseDefault
  -> Js.Promise.then_(value => { Js.log(value); Js.Promise.resolve(()); })
  // Prints "to js" to console

  let resultFutureErr = Result.fromError(Failure("ocaml error"));
  resultFuture
  -> ResultFuture.toJsPromiseDefault
  -> Js.Promise.catch(error => { Js.log(error); Js.Promise.resolve(()); });
  // The ocaml error gets printed in its "raw" javascript form
  ```
- `fromFutureResult`:
  ```reasonml
  Future.fromValue(Belt.Result.Ok("example"))
  -> ResultFuture.fromFutureResult
  -> ResultFuture.getOk(Js.log);     // "example"
  ```
- `toFutureResult`:
  ```reasonml
  ResultFuture.fromValue(345)
  -> ResultFuture.toFutureResult
  -> Future.get((result: Belt.Result.t(int, 'e)) => Js.log(result)); // [345]
  ```

#### Effect functions do not affect the value

- `effectOk` - effect for ok value
  ```reasonml
  ResultFuture.fromValue(345)
  -> ResultFuture.effectOk(value => Js.log("Value: " ++ Js.Int.toString(value)))
  -> ResultFuture.mapOk(value => 2 * value) //... etc.
  ```
- `effectError` - Like effectOk but for error only.
- `effectResult` - effect for any result:
  ```reasonml
  ResultFuture.fromValue("test")
  -> ResultFuture.effectResult(result => Js.log("Error or value: " ++ Js.String.make(result)))
  -> ResultFuture.getOk(Js.log); // "test"
  ```
- `waitEffectOk`, `waitEffectError`, `waitEffectResult` - Similar to `Future.waitEffect`.
   See `src/ResultFuture.rei` for signatures and unit tests for usage examples.
- `getOk` - Get ok value for "final" operation. (getOk returns unit.)
  ```reasonml
  resultFuture
  -> ResultFuture.getOk(value => {
    Js.log("Value " ++ value ++ " handled successfully");
  });
  ```
- `getError` - Final error handling
  ```reasonml
  resultFuture
  -> ResultFuture.effectOk(value => {
    Js.log("Value " ++ value ++ " handled successfully"); 
  })
  -> ResultFuture.getError(error => {
    Js.log("Handling failed with: " ++ Js.String.make(error));
  });
  ```
- `getResult`:
  ```reasonml
  resultFuture
  -> ResultFuture.getResult(result => switch(result) {
    | Ok(value) => Js.log("Jay! Success: " ++ Js.String.make(value))
    | Error(error) => Js.log("Nay! Error: " ++ Js.String.make(error)) 
  });
  ```

#### Functors

- `flatMapOk`:
  ```reasonml
  let longProcessWhichMayFail: string => ResultFuture.t((int, string)) =
    param =>
      ResultFuture.make((resolve, _reject) => {
        let valueTuple = (1000, "¿" ++ param ++ "?");
        Js.Global.setTimeout(() => resolve(valueTuple), 1000);
      });
  ResultFuture.fromValue("hello")
  -> ResultFuture.flatMapOk(longProcessWhichMayFail)
  -> ResultFuture.getOk(((number, text)) => {
    // Notice that long process returns tuple inside of ResultFuture.t, ^ number and text
    Js.log(number); // 1000
    Js.log(text);   // "¿hello?"
  });
  ```
- `flatMapError` - the binded Fn takes an error and evaluates to a ResultFuture.gt.
- `flatMapResult`:
  ```reasonml
  let longProcess: Belt.Result.t(string, exn) => ResultFuture.t(string) =
    result => switch (result) {
      | Ok(value) => ResultFuture.fromValue(value ++ " confirmed")
      | Error(error) => ResultFuture.fromError(error)
    };
  ResultFuture.fromValue("kangaroo")
  -> ResultFuture.flatMapResult(longProcess)
  -> ResultFuture.getOk(Js.log);  // "kangaroo confirmed"
  ```
- `mapOk`
  ```reasonml
  ResultFuture.fromValue("simple")
  -> ResultFuture.mapOk(value => value ++ " manipulation")
  -> ResultFuture.getOk(Js.log);  // "simple manipulation"
  ```
- `mapError` - like mapOk, but for error
- `mapOkResult`:
  ```reasonml
  let twister: int => Belt.Result.t(int, exn) =
    number => switch (number mod 2) {
      | 0 => Error(Failure("not allowed"))
      | 1 => Ok(number)
    };
  ResultFuture.fromValue(13)
  -> ResultFuture.mapOkResult(twister)
  -> ResultFuture.getResult(Js.log);   // [13]
  ResultFuture.fromValue(14)
  -> ResultFuture.mapOkResult(twister)
  -> ResultFuture.getResult(Js.log);   // [[["Failure",-2],"not allowed"]]
  ```
- `mapErrorResult` - map a Fn which takes an error and evaluates as a Result.
- `mapResult` - map a Fn which takes a Result and evaluates as a Result.

#### Combining ResultFutures

- `allOk`
- `allToFuture`
- `combineOk2` ... `combineOk8`
- `mapResult2` ... `mapResult8`



[npm-image]: https://img.shields.io/npm/v/@rejs/re-future.svg
[npm-uri]: https://www.npmjs.com/package/@rejs/re-future
[travis-image]: https://travis-ci.org/seprich/re-future.svg?branch=master
[travis-uri]: https://travis-ci.org/seprich/re-future
[coveralls-image]: https://coveralls.io/repos/github/seprich/re-future/badge.svg?branch=master
[coveralls-uri]: https://coveralls.io/github/seprich/re-future?branch=master
[snyk-image]: https://snyk.io/test/github/seprich/re-future/badge.svg
[snyk-uri]: https://snyk.io/test/github/seprich/re-future
