open Jest;
open Expect;

let delay: (unit => 'a, int) => Future.t('a) =
  (fn, timeoutMs) =>
    Future.make(setter => Js.Global.setTimeout(() => setter(fn()), timeoutMs) |> ignore);

let assertEqual: Future.t('a) => 'a => (assertion => unit) => unit =
  (future, expected, done_) =>
    Future.get(future, value => expect(value) |> toEqual(expected) |> done_);


describe("Future", () => {
  
  testAsync("Simple sync value", test => {
    Future.fromValue("testing 123")
    -> Future.map(s => s ++ "456")
    -> assertEqual("testing 123456", test)
  });

  testAsync("Simple async value", test => {
    delay(() => "another 345", 5)
    -> Future.map(s => s ++ "678")
    -> assertEqual("another 345678", test)
  });

  testAsync("Mix sync and async in chain 1", test => {
    Future.fromValue(17)
    -> Future.flatMap(v => delay(() => v + 4, 5))
    -> Future.map(v => v * 2)
    -> assertEqual(42, test)
  });

  testAsync("Mix sync and async in chain 2", test => {
    delay(() => 17, 5)
    -> Future.map(v => v + 4)
    -> Future.flatMap(v => delay(() => v * 2, 5))
    -> assertEqual(42, test)
  });

  testAsync("`make` must not allow multiple resolving attempts", test => {
    let nastyResolver = (setter) => {
      setter("kuckoo");
      setter("changed my mind");
    };
    let expected = Failure("Future value can be set only once - subsequent setter calls rejected");
    expect(() => Future.make(nastyResolver)) |> toThrowException(expected) |> test;
  });

  testAsync("`effect` passes on original value and executes its argument function", test => {
    let side = ref("some");
    delay(() => "flower power", 5)
    -> Future.effect(v => { side := "extra " ++ v ++ "!"})
    -> Future.get(v => expect((v, side^)) |> toEqual(("flower power", "extra flower power!")) |> test)
  });

  testAsync("`waitEffect` executes its async argument function and passes on original value", test => {
    let side = ref("some");
    delay(() => "flower power", 5)
    -> Future.waitEffect(v => delay(() => { side := "Count " ++ string_of_int(String.length(v)) }, 10))
    -> Future.get(v => expect((v, side^)) |> toEqual(("flower power", "Count 12")) |> test)
  });

  testAsync("`all`", test => {
    Future.all([
      delay(() => 594, 15),
      Future.fromValue(262),
      delay(() => 681, 2),
      delay(() => 2, 23),
      Future.fromValue(0),
      delay(() => 92, 29),
    ])
    -> assertEqual([594, 262, 681, 2, 0, 92], test)
  });

  testAsync("`map2`", test => {
    Future.map2(delay(() => 123, 17), delay(() => "testing", 11), (a, b) => (b, a))
    -> assertEqual(("testing", 123), test)
  });

  testAsync("`map3`", test => {
    Future.map3(delay(() => 123, 17), Future.fromValue("more"), delay(() => "testing", 11), (a, b, c) => (b, c, a))
    -> assertEqual(("more", "testing", 123), test)
  });

  testAsync("`map4`", test => {
    Future.map4(
      delay(() => 123, 17),
      Future.fromValue("more"),
      delay(() => "testing", 11),
      delay(() => "or less", 3),
      (a, b, c, d) => (b, c, d, a),
    )
    -> assertEqual(("more", "testing", "or less", 123), test)
  });

  testAsync("`map5`", test => {
    Future.map5(
      delay(() => 123, 17),
      Future.fromValue("more"),
      delay(() => "testing", 11),
      delay(() => true, 7) -> Future.map(v => !v),
      delay(() => "or less", 3),
      (a, b, c, d, e) => (b, c, d, e, a),
    )
    -> assertEqual(("more", "testing", false, "or less", 123), test)
  });

  testAsync("`map6`", test => {
    Future.map6(
      delay(() => 123, 17),
      Future.fromValue("more"),
      delay(() => "testing", 11),
      delay(() => true, 7) -> Future.map(v => !v),
      delay(() => "or less", 3),
      Future.fromValue(34.235),
      (a, b, c, d, e, f) => (b, c, d, e, a, f),
    )
    -> assertEqual(("more", "testing", false, "or less", 123, 34.235), test)
  });

  testAsync("`toPromise`", test => {
    let promise = delay(() => "should be converted", 5) -> Future.toPromise;
    promise
    |> Js.Promise.then_(value => {
        expect(value) |> toEqual("should be converted") |> test;
        Js.Promise.resolve(());
      })
    |> ignore;
  });

});
