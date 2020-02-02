type gt('a, 'e) = Future.t(Belt.Result.t('a, 'e));
type t('a) = gt('a, exn);

let make =
  delegateFn =>
    Future.make(setter => {
      let resolveCb = value => setter(Belt.Result.Ok(value));
      let rejectCb = value => setter(Belt.Result.Error(value));
      delegateFn(resolveCb, rejectCb);
    });

let fromValue = value => Future.fromValue(Belt.Result.Ok(value));
let fromError = error => Future.fromValue(Belt.Result.Error(error));
let fromResult = Future.fromValue;

let fromFutureResult = future => future;
let fromFuture = future => Future.map(future, value => Belt.Result.Ok(value));
let toFutureResult = future => future;
let toFutureIgnoreResult = future => Future.map(future, ignore);

let fromJsPromise =
  (jsPromise, errorConverter) =>
    Future.make(setter =>
      jsPromise
      |> Js.Promise.then_(value => Ok(value) -> setter -> Js.Promise.resolve)
      |> Js.Promise.catch(error => errorConverter(error) -> Error -> setter -> Js.Promise.resolve)
      |> ignore
    );

let toJsPromise =
  (future, errorConverter) =>
    Js.Promise.make((~resolve, ~reject) => {
      Future.effect(future, result => switch (result) {
        | Ok(value) => resolve(. value)
        | Error(error) => reject(. errorConverter(error))
      } |> ignore)
      |> ignore
    });

let convertPromiseErrorToExn: Js.Promise.error => exn =
  error => {
    /* Unfortunately Js.Promise.error is very useless type and following is necessary if we wish to be able to access it's contents */
    let retype: (. Js.Promise.error) => exn = [%raw {| function(e) { return e; } |}];
    /* It seems insane to first raise and then immediately catch an error, but we do this to generate magical javascript code.
     * On the javascript side bucklescript generates a function call `Caml_js_exceptions.internalToOCamlException(raw_error)`
     * into the `catch` segment. That function is the only one which is able to make sure that both ReasonML-originated and JS-originated
     * error objects are enriched with hidden/internal meta data needed by bucklescript. */
    try (raise(retype(. error))) {
      | error => error
    }
  };

let fromJsPromiseDefault = jsPromise => fromJsPromise(jsPromise, convertPromiseErrorToExn);
let toJsPromiseDefault = future => toJsPromise(future, x => x);

let mapOk =
  (future, fn) =>
    Future.map(future, result => switch (result) {
      | Ok(value) => Ok(fn(value))
      | Error(e) => Error(e)
    });

let mapError =
  (future, fn) =>
    Future.map(future, result => switch (result) {
      | Ok(value) => Ok(value)
      | Error(error) => Error(fn(error))
    });

let mapResult = Future.map;

let mapOkResult =
  (future, fn) =>
    Future.map(future, result => switch (result) {
      | Ok(value) => fn(value)
      | Error(error) => Error(error)
    });

let mapErrorResult =
  (future, fn) =>
    Future.map(future, result => switch (result) {
      | Ok(value) => Ok(value)
      | Error(error) => fn(error)
    });

let flatMapOk =
  (future, fn) =>
    Future.flatMap(future, result => switch (result) {
      | Ok(value) => fn(value)
      | Error(error) => Future.fromValue(Error(error))
    });

let flatMapError =
  (future, fn) =>
    Future.flatMap(future, result => switch (result) {
      | Ok(value) => Future.fromValue(Ok(value))
      | Error(error) => fn(error)
    });

let flatMapResult = Future.flatMap;

let effectOk =
  (future, fn) =>
    Future.effect(future, result => switch (result) {
      | Ok(value) => fn(value)
      | Error(_) => ()
    });

let effectError =
  (future, fn) =>
    Future.effect(future, result => switch (result) {
      | Ok(_) => ()
      | Error(error) => fn(error)
    });

let effectResult = Future.effect;

let waitEffectOk =
  (future, fn) =>
    Future.waitEffect(future, result => switch (result) {
      | Ok(value) => fn(value)
      | Error(_) => Future.fromValue(())
    });

let waitEffectError =
  (future, fn) =>
    Future.waitEffect(future, result => switch (result) {
      | Ok(_) => Future.fromValue(())
      | Error(error) => fn(error)
    });

let waitEffectResult = Future.waitEffect;

let allToFuture = Future.all;

let allOk: list(Future.t(Belt.Result.t('a, 'e))) => Future.t(Belt.Result.t(list('a), 'e)) =
  futures => {
    let reducer: (Belt.Result.t(list('a), 'e), Belt.Result.t('a, 'e)) => Belt.Result.t(list('a), 'e) =
      (accum, item) => switch(accum, item) {
        | (Ok(values), Ok(value)) => Ok(Belt.List.add(values, value))
        | (_, Error(error)) => Error(error)
        | (Error(error), _) => Error(error)
      };
    Future.all(futures)
    -> Future.map(results => Belt.List.reduceReverse(results, Belt.Result.Ok([]), reducer));
  };

let mapResult2 =
  (f1, f2, fn) =>
    f1 -> flatMapResult(r1 =>
      f2 -> mapResult(r2 => fn(r1, r2)));

let mapResult3 =
  (f1, f2, f3, fn) =>
    f1 -> flatMapResult(r1 =>
      f2 -> flatMapResult(r2 =>
        f3 -> mapResult(r3 => fn(r1, r2, r3))));

let mapResult4 =
  (f1, f2, f3, f4, fn) =>
    f1 -> flatMapResult(r1 =>
      f2 -> flatMapResult(r2 =>
        f3 -> flatMapResult(r3 =>
          f4 -> mapResult(r4 => fn(r1, r2, r3, r4)))));

let mapResult5 =
  (f1, f2, f3, f4, f5, fn) =>
    mapResult2(
      mapResult4(f1, f2, f3, f4, (a, b, c, d) => (a, b, c, d)),
      f5,
      ((r1, r2, r3, r4), r5) => fn(r1, r2, r3, r4, r5));

let mapResult6 =
  (f1, f2, f3, f4, f5, f6, fn) =>
    mapResult3(
      mapResult4(f1, f2, f3, f4, (a, b, c, d) => (a, b, c, d)),
      f5, f6,
      ((r1, r2, r3, r4), r5, r6) => fn(r1, r2, r3, r4, r5, r6));

let mapResult7 =
  (f1, f2, f3, f4, f5, f6, f7, fn) =>
    mapResult4(
      mapResult4(f1, f2, f3, f4, (a, b, c, d) => (a, b, c, d)),
      f5, f6, f7,
      ((r1, r2, r3, r4), r5, r6, r7) => fn(r1, r2, r3, r4, r5, r6, r7));

let mapResult8 =
  (f1, f2, f3, f4, f5, f6, f7, f8, fn) =>
    mapResult2(
      mapResult4(f1, f2, f3, f4, (a, b, c, d) => (a, b, c, d)),
      mapResult4(f5, f6, f7, f8, (a, b, c, d) => (a, b, c, d)),
      ((r1, r2, r3, r4), (r5, r6, r7, r8)) => fn(r1, r2, r3, r4, r5, r6, r7, r8));

let combineOk2 =
  (f1, f2) =>
    f1 -> flatMapOk(v1 =>
      f2 -> mapOk(v2 => (v1, v2)));

let combineOk3 =
  (f1, f2, f3) =>
    f1 -> flatMapOk(v1 =>
      f2 -> flatMapOk(v2 =>
        f3 -> mapOk(v3 => (v1, v2, v3))));

let combineOk4 =
  (f1, f2, f3, f4) =>
    f1 -> flatMapOk(v1 =>
      f2 -> flatMapOk(v2 =>
        f3 -> flatMapOk(v3 =>
          f4 -> mapOk(v4 => (v1, v2, v3, v4)))));

let combineOk5 =
  (f1, f2, f3, f4, f5) =>
    combineOk2(combineOk4(f1, f2, f3, f4), f5)
    -> mapOk((((r1, r2, r3, r4), r5)) => (r1, r2, r3, r4, r5));

let combineOk6 =
  (f1, f2, f3, f4, f5, f6) =>
    combineOk3(combineOk4(f1, f2, f3, f4), f5, f6)
    -> mapOk((((r1, r2, r3, r4), r5, r6)) => (r1, r2, r3, r4, r5, r6));

let combineOk7 =
  (f1, f2, f3, f4, f5, f6, f7) =>
    combineOk4(combineOk4(f1, f2, f3, f4), f5, f6, f7)
    -> mapOk((((r1, r2, r3, r4), r5, r6, r7)) => (r1, r2, r3, r4, r5, r6, r7));

let combineOk8 =
  (f1, f2, f3, f4, f5, f6, f7, f8) =>
    combineOk2(combineOk4(f1, f2, f3, f4), combineOk4(f5, f6, f7, f8))
    -> mapOk((((r1, r2, r3, r4), (r5, r6, r7, r8))) => (r1, r2, r3, r4, r5, r6, r7, r8));

let getOk =
  (future, fn) =>
    Future.get(future, result => switch (result) {
      | Ok(value) => fn(value)
      | Error(_) => ()
    });

let getError =
  (future, fn) =>
    Future.get(future, result => switch (result) {
      | Ok(_) => ()
      | Error(error) => fn(error) 
    });

let getResult = Future.get;
