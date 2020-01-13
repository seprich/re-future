type t('a) = {
  mutable value: option('a),
  mutable fnChain: list('a => unit),
};

let make =
  delegateFn => {
    let setter: t('a) => 'a => unit =
      (promise, value) => {
        promise.value = Some(value);
        //promise.fnChain -> Belt.List.reverse -> Belt.List.forEach(fn => fn(value));
        promise.fnChain -> Belt.List.reverse -> Belt.List.forEach(fn => Js.Global.setTimeout(() => fn(value), 0));
        promise.fnChain = [];
      }
    let promise = { value: None, fnChain: [] };
    delegateFn(setter(promise));
    promise;
  };

let fromValue = value => { value: Some(value), fnChain: [] };

let addChainFn_: (t('a), 'a => unit) => unit  =
  (future, chainFn) =>
    switch (future.value) {
      | Some(v) => chainFn(v)
      | None => {
        future.fnChain = [chainFn, ...future.fnChain]
      }
    };

let flatMap =
  (future, fn) =>
    make(setter =>
      addChainFn_(future, v => addChainFn_(fn(v), setter)));

let map = (future, fn) => flatMap(future, v => fromValue(fn(v)));

let effect =
  (future, fn) => {
    addChainFn_(future, fn);
    future;
  };

let waitEffect = (future, fn) => flatMap(future, fn) -> flatMap(() => future);

let all =
  listOfFutures =>
    Belt.List.reduceReverse(listOfFutures, fromValue([]), (accumulator, future) => flatMap(future, result => map(accumulator, results => [result, ...results])));

let map2 =
  (f1, f2, fn) =>
     f1 -> flatMap(r1 =>
       f2 -> map(r2 => fn(r1, r2)));

let map3 =
  (f1, f2, f3, fn) =>
    f1 -> flatMap(r1 =>
      f2 -> flatMap(r2 =>
        f3 -> map(r3 => fn(r1, r2, r3))));

let map4 =
  (f1, f2, f3, f4, fn) =>
    f1 -> flatMap(r1 =>
      f2 -> flatMap(r2 =>
        f3 -> flatMap(r3 =>
          f4 -> map(r4 => fn(r1, r2, r3, r4)))));

let map5 =
  (f1, f2, f3, f4, f5, fn) =>
    f1 -> flatMap(r1 =>
      f2 -> flatMap(r2 =>
        f3 -> flatMap(r3 =>
          f4 -> flatMap(r4 =>
            f5 -> map(r5 => fn(r1, r2, r3, r4, r5))))));

let map6 =
  (f1, f2, f3, f4, f5, f6, fn) =>
    f1 -> flatMap(r1 =>
      f2 -> flatMap(r2 =>
        f3 -> flatMap(r3 =>
          f4 -> flatMap(r4 =>
            f5 -> flatMap(r5 =>
              f6 -> map(r6 => fn(r1, r2, r3, r4, r5, r6)))))));

let get = addChainFn_;
