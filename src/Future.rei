type t('a);

let make: (('a => unit) => unit) => t('a);
let fromValue: 'a => t('a);

let map:     (t('a), 'a => 'b) => t('b);
let flatMap: (t('a), 'a => t('b)) => t('b);

let effect:     (t('a), 'a => unit) => t('a);
let waitEffect: (t('a), 'a => t(unit)) => t('a);

let all: list(t('a)) => t(list('a));

let map2: (t('a), t('b), ('a, 'b) => 'r) => t('r);
let map3: (t('a), t('b), t('c), ('a, 'b, 'c) => 'r) => t('r);
let map4: (t('a), t('b), t('c), t('d), ('a, 'b, 'c, 'd) => 'r) => t('r);
let map5: (t('a), t('b), t('c), t('d), t('e), ('a, 'b, 'c, 'd, 'e) => 'r) => t('r);
let map6: (t('a), t('b), t('c), t('d), t('e), t('f), ('a, 'b, 'c, 'd, 'e, 'f) => 'r) => t('r);

let get: (t('a), 'a => unit) => unit;

let toPromise: t('a) => Js.Promise.t('a);
