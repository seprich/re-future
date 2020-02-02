open Jest;
open Expect;

let delay: (unit => 'a, int) => Future.t('a) =
  (fn, timeoutMs) =>
    Future.make(setter => Js.Global.setTimeout(() => setter(fn()), timeoutMs) |> ignore);

let assertEqual: Future.t('a) => 'a => (assertion => unit) => unit =
  (future, expected, done_) =>
    Future.get(future, value => expect(value) |> toEqual(expected) |> done_);


describe("Future", () => {

  describe("Positive tests", () => {
  
    testAsync("Simple sync value", eval => {
      Future.fromValue("testing 123")
      -> Future.map(s => s ++ "456")
      -> assertEqual("testing 123456", eval);
    });

    testAsync("Simple async value", eval => {
      delay(() => "another 345", 5)
      -> Future.map(s => s ++ "678")
      -> assertEqual("another 345678", eval);
    });

    testAsync("Mix sync and async in chain 1", eval => {
      Future.fromValue(17)
      -> Future.flatMap(v => delay(() => v + 4, 5))
      -> Future.map(v => v * 2)
      -> assertEqual(42, eval);
    });

    testAsync("Mix sync and async in chain 2", eval => {
      delay(() => 17, 1)
      -> Future.map(v => v + 4)
      -> Future.flatMap(v => delay(() => v * 2, 5))
      -> assertEqual(42, eval);
    });

    testAsync("`effect` passes on original value and executes its argument function", eval => {
      let side = ref("some");
      delay(() => "flower power", 1)
      -> Future.effect(v => { side := "extra " ++ v ++ "!"})
      -> Future.get(v => expect((v, side^)) |> toEqual(("flower power", "extra flower power!")) |> eval);
    });

    testAsync("`waitEffect` executes its async argument function and passes on original value", eval => {
      let side = ref("some");
      delay(() => "flower power", 1)
      -> Future.waitEffect(v => delay(() => { side := "Count " ++ string_of_int(String.length(v)) }, 10))
      -> Future.get(v => expect((v, side^)) |> toEqual(("flower power", "Count 12")) |> eval);
    });

    testAsync("`all`", eval => {
      Future.all([
        delay(() => 594, 15),
        Future.fromValue(262),
        delay(() => 681, 2),
        delay(() => 2, 23),
        Future.fromValue(0),
        delay(() => 92, 29),
      ])
      -> assertEqual([594, 262, 681, 2, 0, 92], eval);
    });

    testAsync("`combine2`", eval => {
      Future.combine2(delay(() => 123, 17), delay(() => "testing", 11))
      -> assertEqual((123, "testing"), eval);
    });

    testAsync("`combine3`", eval => {
      Future.combine3(delay(() => 123, 17), Future.fromValue("more"), delay(() => "testing", 11))
      -> assertEqual((123, "more", "testing"), eval);
    });

    testAsync("`combine4`", eval => {
      Future.combine4(
        delay(() => 123, 17),
        Future.fromValue("more"),
        delay(() => "testing", 11),
        delay(() => "or less", 3),
      )
      -> assertEqual((123, "more", "testing", "or less"), eval);
    });

    testAsync("`combine5`", eval => {
      Future.combine5(
        delay(() => 123, 17),
        Future.fromValue("more"),
        delay(() => "testing", 11),
        delay(() => true, 7) -> Future.map(v => !v),
        delay(() => "or less", 3),
      )
      -> assertEqual((123, "more", "testing", false, "or less"), eval);
    });

    testAsync("`combine6`", eval => {
      Future.combine6(
        delay(() => 123, 17),
        Future.fromValue("more"),
        delay(() => "testing", 11),
        delay(() => true, 7) -> Future.map(v => !v),
        delay(() => "or less", 3),
        Future.fromValue(34.235),
      )
      -> assertEqual((123, "more", "testing", false, "or less", 34.235), eval);
    });

    testAsync("`combine7`", eval => {
      Future.combine7(
        delay(() => 123, 17),
        Future.fromValue("more"),
        delay(() => "testing", 11),
        delay(() => true, 7) -> Future.map(v => !v),
        delay(() => "or less", 3),
        Future.fromValue(34.235),
        Future.fromValue(Failure("kuckoo")),
      )
      -> assertEqual((123, "more", "testing", false, "or less", 34.235, Failure("kuckoo")), eval);
    });

    testAsync("`combine8`", eval => {
      Future.combine8(
        delay(() => 123, 17),
        Future.fromValue("more"),
        delay(() => "testing", 11),
        delay(() => true, 7) -> Future.map(v => !v),
        delay(() => "or less", 3),
        Future.fromValue(34.235),
        Future.fromValue(Failure("kuckoo")),
        delay(() => true, 2),
      )
      -> assertEqual((123, "more", "testing", false, "or less", 34.235, Failure("kuckoo"), true), eval);
    });

    testAsync("`toPromise`", eval => {
      let promise = delay(() => "should be converted", 5) -> Future.toPromise;
      promise
      |> Js.Promise.then_(value => {
          expect(value) |> toEqual("should be converted") |> eval;
          Js.Promise.resolve(());
        })
      |> ignore;
    });
  });

  describe("Negative tests", () => {

    testAsync("`make` must not allow multiple resolving attempts", eval => {
      let nastyResolver = (setter) => {
        setter("kuckoo");
        setter("changed my mind");
      };
      let expected = Failure("Future value can be set only once - subsequent setter calls rejected");
      expect(() => Future.make(nastyResolver)) |> toThrowException(expected) |> eval;
    });
  });
});
