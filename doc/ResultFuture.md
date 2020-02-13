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

- `allOk` - List of ResultFuture's to a ResultFuture containing a list
  ```reasonml
  ResultFuture.allOk([rf1, rf2, rf3])  // rf1.. rf3 result futures of same type
  -> ResultFuture.effectOk(([r1, r2, r3]) => { /* All successful results r1 .. r3 */ })
  -> ResultFuture.getError(error => { /* if even one was error this gets executed (and effectOk is not executed) */ });
  ```
- `allToFuture`:
  ```reasonml
  ResultFuture.allToFuture([rf1, rf2, rf3]) // rf1.. rf3 result futures of same type
  -> Future.get(listOfResults =>
    listOfResults -> Belt.List.map(result => {
      Js.log(result);   // Each result, Ok or Error
    });
  );
  ```
- `combineOk2` ... `combineOk8`:
  ```reasonml
  ResultFuture.combineOk3((rf1, rf2, rf3))  // Heterogenous result futures rf1.. rf3
  -> ResultFuture.effectOk(((okValue1, okValue2, okValue3)) => { /* different types of Ok values */ })
  -> ResultFuture.getError(error => { /* if even one was error */ });
  ```
- `mapResult2` ... `mapResult8`:
  ```reasonml
  ResultFuture.mapResult3(
    ResultFuture.fromValue(23),
    ResultFuture.fromValue("test"),
    ResultFuture.fromValue(true),
    (r1, r2, r3) => switch (r1, r2, r3) {
      | (Ok(n), Ok(msg), Ok(_)) => Ok(msg ++ ": " ++ Js.String.make(n))
      | (_, _, _) => Error(Failure("Some Error"))
    }
  )
  -> ResultFuture.getOk(Js.log);  // prints "test: 23"
  ```
