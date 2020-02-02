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

let ignore = future => Future.map(future, ignore);
