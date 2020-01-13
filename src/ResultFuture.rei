type gt('a, 'e);
type t('a) = gt('a, exn);

let fromJsPromiseDefault:  Js.Promise.t('a) => t('a);
let toJsPromiseDefault:    t('a) => Js.Promise.t('a);

let fromJsPromise: (Js.Promise.t('a), Js.Promise.error => 'e) => gt('a, 'e);
let toJsPromise:   (gt('a, 'e), 'e => exn) => Js.Promise.t('a);

let fromValue:  'a => gt('a, 'e);
let fromError:  'e => gt('a, 'e);
let fromResult: Belt.Result.t('a, 'e) => gt('a, 'e);

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

let getOk:     (gt('a, 'e), 'a => unit) => unit;
let getError:  (gt('a, 'e), 'e => unit) => unit;
let getResult: (gt('a, 'e), Belt.Result.t('a, 'e) => unit) => unit;
