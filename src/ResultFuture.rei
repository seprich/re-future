type gt('a, 'e);
type t('a) = gt('a, exn);

let make: (('a => unit, 'e => unit) => unit) => gt('a, 'e);

let fromJsPromiseDefault:  Js.Promise.t('a) => t('a);
let toJsPromiseDefault:    t('a) => Js.Promise.t('a);

let fromJsPromise: (Js.Promise.t('a), Js.Promise.error => 'e) => gt('a, 'e);
let toJsPromise:   (gt('a, 'e), 'e => exn) => Js.Promise.t('a);

let fromValue:  'a => gt('a, 'e);
let fromError:  'e => gt('a, 'e);
let fromResult: Belt.Result.t('a, 'e) => gt('a, 'e);

let fromFutureResult:     Future.t(Belt.Result.t('a, 'e)) => gt('a, 'e);
let fromFuture:           Future.t('a) => gt('a, 'e);
let toFutureResult:       gt('a, 'e) => Future.t(Belt.Result.t('a, 'e));
let toFutureIgnoreResult: gt('a, 'e) => Future.t(unit);

let getOk:     (gt('a, 'e), 'a => unit) => unit;
let getError:  (gt('a, 'e), 'e => unit) => unit;
let getResult: (gt('a, 'e), Belt.Result.t('a, 'e) => unit) => unit;

let mapOk:          (gt('a, 'e), 'a => 'b) => gt('b, 'e);
let mapError:       (gt('a, 'e), 'e => 'x) => gt('a, 'x);
let mapOkResult:    (gt('a, 'e), 'a => Belt.Result.t('b, 'e)) => gt('b, 'e);
let mapErrorResult: (gt('a, 'e), 'e => Belt.Result.t('a, 'x)) => gt('a, 'x);
let mapResult:      (gt('a, 'e), Belt.Result.t('a, 'e) => Belt.Result.t('b, 'x)) => gt('b, 'x);

let flatMapOk:     (gt('a, 'e), 'a => gt('b, 'e)) => gt('b, 'e);
let flatMapError:  (gt('a, 'e), 'e => gt('a, 'x)) => gt('a, 'x);
let flatMapResult: (gt('a, 'e), Belt.Result.t('a, 'e) => gt('b, 'x)) => gt('b, 'x);

let effectOk:     (gt('a, 'e), 'a => unit) => gt('a, 'e);
let effectError:  (gt('a, 'e), 'e => unit) => gt('a, 'e);
let effectResult: (gt('a, 'e), Belt.Result.t('a, 'e) => unit) => gt('a, 'e);

let waitEffectOk:     (gt('a, 'e), 'a => Future.t(unit)) => gt('a, 'e);
let waitEffectError:  (gt('a, 'e), 'e => Future.t(unit)) => gt('a, 'e);
let waitEffectResult: (gt('a, 'e), Belt.Result.t('a, 'e) => Future.t(unit)) => gt('a, 'e);

let allOk:       list(gt('a, 'e)) => gt(list('a), 'e);
let allToFuture: list(gt('a, 'e)) => Future.t(list(Belt.Result.t('a, 'e)));

let combineOk2: (gt('a, 'z), gt('b, 'z)) => gt(('a, 'b), 'z);
let combineOk3: (gt('a, 'z), gt('b, 'z), gt('c, 'z)) => gt(('a, 'b, 'c), 'z);
let combineOk4: (gt('a, 'z), gt('b, 'z), gt('c, 'z), gt('d, 'z)) => gt(('a, 'b, 'c, 'd), 'z);
let combineOk5: (gt('a, 'z), gt('b, 'z), gt('c, 'z), gt('d, 'z), gt('e, 'z)) => gt(('a, 'b, 'c, 'd, 'e), 'z);
let combineOk6: (gt('a, 'z), gt('b, 'z), gt('c, 'z), gt('d, 'z), gt('e, 'z), gt('f, 'z)) => gt(('a, 'b, 'c, 'd, 'e, 'f), 'z);
let combineOk7: (gt('a, 'z), gt('b, 'z), gt('c, 'z), gt('d, 'z), gt('e, 'z), gt('f, 'z), gt('g, 'z)) => gt(('a, 'b, 'c, 'd, 'e, 'f, 'g), 'z);
let combineOk8: (gt('a, 'z), gt('b, 'z), gt('c, 'z), gt('d, 'z), gt('e, 'z), gt('f, 'z), gt('g, 'z), gt('h, 'z)) => gt(('a, 'b, 'c, 'd, 'e, 'f, 'g, 'h), 'z);

let mapResult2: (
    gt('a, 'm),
    gt('b, 'n),
    (Belt.Result.t('a, 'm), Belt.Result.t('b, 'n)) => Belt.Result.t('x, 'z)
  ) => gt('x, 'z);
let mapResult3: (
    gt('a, 'm),
    gt('b, 'n),
    gt('c, 'o),
    (Belt.Result.t('a, 'm), Belt.Result.t('b, 'n), Belt.Result.t('c, 'o)) => Belt.Result.t('x, 'z)
  ) => gt('x, 'z);
let mapResult4: (
    gt('a, 'm),
    gt('b, 'n),
    gt('c, 'o),
    gt('d, 'p),
    (Belt.Result.t('a, 'm), Belt.Result.t('b, 'n), Belt.Result.t('c, 'o), Belt.Result.t('d, 'p)) => Belt.Result.t('x, 'z)
  ) => gt('x, 'z);
let mapResult5: (
    gt('a, 'm),
    gt('b, 'n),
    gt('c, 'o),
    gt('d, 'p),
    gt('e, 'q),
    (Belt.Result.t('a, 'm), Belt.Result.t('b, 'n), Belt.Result.t('c, 'o), Belt.Result.t('d, 'p),
     Belt.Result.t('e, 'q)) => Belt.Result.t('x, 'z)
  ) => gt('x, 'z);
let mapResult6: (
    gt('a, 'm),
    gt('b, 'n),
    gt('c, 'o),
    gt('d, 'p),
    gt('e, 'q),
    gt('f, 'r),
    (Belt.Result.t('a, 'm), Belt.Result.t('b, 'n), Belt.Result.t('c, 'o), Belt.Result.t('d, 'p),
     Belt.Result.t('e, 'q), Belt.Result.t('f, 'r)) => Belt.Result.t('x, 'z)
  ) => gt('x, 'z);
let mapResult7: (
    gt('a, 'm),
    gt('b, 'n),
    gt('c, 'o),
    gt('d, 'p),
    gt('e, 'q),
    gt('f, 'r),
    gt('g, 's),
    (Belt.Result.t('a, 'm), Belt.Result.t('b, 'n), Belt.Result.t('c, 'o), Belt.Result.t('d, 'p),
     Belt.Result.t('e, 'q), Belt.Result.t('f, 'r), Belt.Result.t('g, 's)) => Belt.Result.t('x, 'z)
  ) => gt('x, 'z);
let mapResult8: (
    gt('a, 'm),
    gt('b, 'n),
    gt('c, 'o),
    gt('d, 'p),
    gt('e, 'q),
    gt('f, 'r),
    gt('g, 's),
    gt('h, 't),
    (Belt.Result.t('a, 'm), Belt.Result.t('b, 'n), Belt.Result.t('c, 'o), Belt.Result.t('d, 'p),
     Belt.Result.t('e, 'q), Belt.Result.t('f, 'r), Belt.Result.t('g, 's), Belt.Result.t('h, 't)) => Belt.Result.t('x, 'z)
  ) => gt('x, 'z);
