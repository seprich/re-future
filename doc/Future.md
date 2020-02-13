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
  