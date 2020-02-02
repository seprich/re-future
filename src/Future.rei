type t('a);

let make: (('a => unit) => unit) => t('a);
let fromValue: 'a => t('a);

let map:     (t('a), 'a => 'b) => t('b);
let flatMap: (t('a), 'a => t('b)) => t('b);

let effect:     (t('a), 'a => unit) => t('a);
let waitEffect: (t('a), 'a => t(unit)) => t('a);

let all: list(t('a)) => t(list('a));

let combine2: (t('a), t('b)) => t(('a, 'b));
let combine3: (t('a), t('b), t('c)) => t(('a, 'b, 'c));
let combine4: (t('a), t('b), t('c), t('d)) => t(('a, 'b, 'c, 'd));
let combine5: (t('a), t('b), t('c), t('d), t('e)) => t(('a, 'b, 'c, 'd, 'e));
let combine6: (t('a), t('b), t('c), t('d), t('e), t('f)) => t(('a, 'b, 'c, 'd, 'e, 'f));
let combine7: (t('a), t('b), t('c), t('d), t('e), t('f), t('g)) => t(('a, 'b, 'c, 'd, 'e, 'f, 'g));
let combine8: (t('a), t('b), t('c), t('d), t('e), t('f), t('g), t('h)) => t(('a, 'b, 'c, 'd, 'e, 'f, 'g, 'h));

let get: (t('a), 'a => unit) => unit;

let toPromise: t('a) => Js.Promise.t('a);
