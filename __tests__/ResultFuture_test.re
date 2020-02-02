open Jest;
open Expect;

let delay: (unit => 'a, int) => ResultFuture.gt('a, 'e) =
  (fn, timeoutMs) =>
    ResultFuture.make((resolve, reject) =>
      Js.Global.setTimeout(() => try (resolve(fn())) {
        | error => reject(error)
      }, timeoutMs) |> ignore);

let delayPromise: (unit => 'a, int) => Js.Promise.t('a) =
  (fn, timeoutMs) =>
    Js.Promise.make((~resolve, ~reject) =>
      Js.Global.setTimeout(() => try (resolve(. fn())) {
        | error => reject(. error)
      }, timeoutMs) |> ignore);

let jsError: unit => exn = [%raw {| function() { return new Error("js-error"); } |}];

exception TestException(string);
exception OtherTestException(string);

type errorsOfNinthHell = [
  | `Cuckoo
  | `ChocoLatte
  | `ChamoMilly
];

let assertOkEqual: ResultFuture.gt('a, 'e) => 'a => (assertion => unit) => unit =
  (future, expected, done_) =>
    ResultFuture.getOk(future, value => expect(value) |> toEqual(expected) |> done_);

let assertErrorEqual: ResultFuture.gt('a, 'e) => 'e => (assertion => unit) => unit =
  (future, expected, done_) =>
    ResultFuture.getError(future, error => expect(error) |> toEqual(expected) |> done_);

let curiousResultMapper: Belt.Result.t(int, exn) => Belt.Result.t(string, exn) =
  result => switch (result) {
    | Ok(number) =>
      if (number mod 2 == 1) Belt.Result.Ok("Ok for odd number")
      else Belt.Result.Error(TestException("Error for paired number"))
    | Error(TestException(msg)) =>
      if (Js.String.startsWith("!", msg)) Belt.Result.Error(TestException(msg))
      else Belt.Result.Ok(msg)
    | Error(_) => Belt.Result.Error(OtherTestException("something else"))
  };

describe("ResultFuture", () => {

  describe("Positive tests", () => {

    testAsync("`fromValue`", eval => {
      ResultFuture.fromValue("quark")
      -> assertOkEqual("quark", eval);
    });

    testAsync("`fromError`", eval => {
      ResultFuture.fromError(`ChocoLatte)
      -> assertErrorEqual(`ChocoLatte, eval);
    });

    testAsync("`getResult` of an ok result", eval => {
      ResultFuture.fromValue("quark")
      -> ResultFuture.getResult(result => expect(result) |> toEqual(Belt.Result.Ok("quark")) |> eval);
    });

    testAsync("`ignore`", eval => {
      ResultFuture.fromValue("quark")
      -> ResultFuture.ignore
      -> Future.get(value => expect(value) |> toEqual(()) |> eval);
    });

    testAsync("`make` ResultFuture which evaluates to Ok", eval => {
      ResultFuture.make(
        (resolve, _reject) => Js.Global.setTimeout(() => resolve("munchday"), 1) |> ignore
      )
      -> assertOkEqual("munchday", eval);
    });

    testAsync("`make` ResultFuture which evaluates to Error", eval => {
      ResultFuture.make(
        (_resolve, reject) => Js.Global.setTimeout(() => reject(`ChamoMilly), 1) |> ignore
      )
      -> assertErrorEqual(`ChamoMilly, eval);
    });

    testAsync("`fromJsPromiseDefault` test Ok", eval => {
      delayPromise(() => 6534, 1)
      -> ResultFuture.fromJsPromiseDefault
      -> assertOkEqual(6534, eval);
    });

    testAsync("`fromJsPromiseDefault` test exn-error", eval => {
      delayPromise(() => raise(TestException("testing")), 1)
      -> ResultFuture.fromJsPromiseDefault
      -> assertErrorEqual(TestException("testing"), eval);
    });

    testAsync("`fromJsPromiseDefault` test js-error", eval => {
      delayPromise(() => raise(jsError()), 1)
      -> ResultFuture.fromJsPromiseDefault
      -> ResultFuture.getError(error => switch (Js.Exn.asJsExn(error)) {
        | Some(jsExn) => switch (Js.Exn.message(jsExn)) {
          | Some(message) => expect(message) |> toEqual("js-error") |> eval
          | None => ()
        }
        | None => ()
      });
    });

    testAsync("`toJsPromiseDefault` test Ok", eval => {
      (
        delay(() => 172737, 1)
        -> ResultFuture.toJsPromiseDefault
      )
      |> Js.Promise.then_(value => {
        expect(value) |> toEqual(172737) |> eval;
        Js.Promise.resolve(());
      }) |> ignore;
    });

    testAsync("`toJsPromiseDefault` test exn-error", eval => {
      (
        delay(() => raise(TestException("picadilly circus")), 1)
        -> ResultFuture.toJsPromiseDefault
      )
      |> Js.Promise.catch(error => {
        expect(Js.String.make(error)) |> toEqual("ResultFuture_test-SeprichReFuture.TestException,4,picadilly circus") |> eval;
        Js.Promise.resolve(());
      }) |> ignore;
    });

    testAsync("`toJsPromiseDefault` test js-error", eval => {
      (
        delay(() => raise(jsError()), 1)
        -> ResultFuture.toJsPromiseDefault
      )
      |> Js.Promise.catch(error => {
        expect(Js.String.make(error)) |> toEqual("Caml_js_exceptions.Error,1,Error: js-error") |> eval;
        Js.Promise.resolve(());
      }) |> ignore;
    });

    testAsync("js-error survives a round trip unmangled", eval => {
      (
        delayPromise(() => raise(jsError()), 1)
        -> ResultFuture.fromJsPromiseDefault
        -> ResultFuture.toJsPromiseDefault
      )
      |> Js.Promise.catch(error => {
        expect(Js.String.make(error)) |> toEqual("Caml_js_exceptions.Error,1,Error: js-error") |> eval;
        Js.Promise.resolve(());
      }) |> ignore;
    });

    testAsync("exn-error survives a round trip unmangled", eval => {
      delay(() => raise(TestException("trafalgar square")), 1)
      -> ResultFuture.toJsPromiseDefault
      -> ResultFuture.fromJsPromiseDefault
      -> assertErrorEqual(TestException("trafalgar square"), eval);
    });
    
    testAsync("`mapOk`", eval => {
      delay(() => "question", 1)
      -> ResultFuture.mapOk(value => "¿" ++ value ++ "?")
      -> assertOkEqual("¿question?", eval);
    });

    testAsync("`mapError`", eval => {
      delay(() => raise(TestException("multi")), 1)
      -> ResultFuture.mapError(error => switch (error) {
        | TestException(txt) => OtherTestException(txt ++ "-part")
        | other => other
      })
      -> assertErrorEqual(OtherTestException("multi-part"), eval);
    });

    testAsync("`mapOkResult` to Ok", eval => {
      delay(() => "muppet", 1)
      -> ResultFuture.mapOkResult(value => Belt.Result.Ok(value ++ " deary"))
      -> assertOkEqual("muppet deary", eval);
    });

    testAsync("`mapOkResult` to Error", eval => {
      delay(() => "muppet", 1)
      -> ResultFuture.mapOkResult(value => Belt.Result.Error(TestException(value ++ " dreary")))
      -> assertErrorEqual(TestException("muppet dreary"), eval);
    });

    testAsync("`mapErrorResult` to Ok", eval => {
      delay(() => raise(TestException("damn")), 1)
      -> ResultFuture.mapErrorResult(error => switch (error) {
        | TestException(msg) => Belt.Result.Ok(msg ++ " :/")
        | other => Belt.Result.Error(other)
      })
      -> assertOkEqual("damn :/", eval);
    });

    testAsync("`mapErrorResult` to Error", eval => {
      delay(() => raise(TestException("damn")), 1)
      -> ResultFuture.mapErrorResult(error => switch (error) {
        | TestException(msg) => Belt.Result.Error(OtherTestException(msg ++ " spam"))
        | other => Belt.Result.Error(other)
      })
      -> assertErrorEqual(OtherTestException("damn spam"), eval);
    });

    testAsync("`mapResult` Ok to Ok", eval => {
      delay(() => 47, 1)
      -> ResultFuture.mapResult(curiousResultMapper)
      -> assertOkEqual("Ok for odd number", eval);
    });

    testAsync("`mapResult` Ok to Error", eval => {
      delay(() => 48, 1)
      -> ResultFuture.mapResult(curiousResultMapper)
      -> assertErrorEqual(TestException("Error for paired number"), eval);
    });

    testAsync("`mapResult` Error To Ok", eval => {
      delay(() => raise(TestException("blink")), 1)
      -> ResultFuture.mapResult(curiousResultMapper)
      -> assertOkEqual("blink", eval);
    });

    testAsync("`mapResult` Error To Error", eval => {
      delay(() => raise(TestException("!blink")), 1)
      -> ResultFuture.mapResult(curiousResultMapper)
      -> assertErrorEqual(TestException("!blink"), eval);
    });

    testAsync("`flatMapOk` to Ok", eval => {
      delay(() => "puppet", 1)
      -> ResultFuture.flatMapOk(value => delay(() => "Evil " ++ value ++ "eer", 1))
      -> assertOkEqual("Evil puppeteer", eval);
    });

    testAsync("`flatMapOk` to Error", eval => {
      delay(() => "puppet", 1)
      -> ResultFuture.flatMapOk(value => delay(() => raise(TestException("Evil " ++ value ++ "eer")), 1))
      -> assertErrorEqual(TestException("Evil puppeteer"), eval);
    });

    testAsync("`flatMapError` to Ok", eval => {
      delay(() => raise(TestException("cupboard")), 1)
      -> ResultFuture.flatMapError(error => switch (error) {
        | TestException(msg) => ResultFuture.fromValue("'" ++ msg)
        | _ => ResultFuture.fromError(error)
      })
      -> assertOkEqual("'cupboard", eval);
    });

    testAsync("`flatMapError` to Error", eval => {
      delay(() => raise(TestException("cupboard")), 1)
      -> ResultFuture.flatMapError(error => switch (error) {
        | TestException(msg) => ResultFuture.fromError(OtherTestException(msg))
        | _ => ResultFuture.fromError(error)
      })
      -> assertErrorEqual(OtherTestException("cupboard"), eval);
    });

    testAsync("`flatMapResult` 1", eval => {
      delay(() => 33, 1)
      -> ResultFuture.flatMapResult(result => ResultFuture.fromResult(curiousResultMapper(result)))
      -> assertOkEqual("Ok for odd number", eval);
    });

    testAsync("`flatMapResult` 2", eval => {
      delay(() => 34, 1)
      -> ResultFuture.flatMapResult(result => ResultFuture.fromResult(curiousResultMapper(result)))
      -> assertErrorEqual(TestException("Error for paired number"), eval);
    });

    testAsync("`flatMapResult` 3", eval => {
      delay(() => raise(TestException("blink")), 1)
      -> ResultFuture.flatMapResult(result => ResultFuture.fromResult(curiousResultMapper(result)))
      -> assertOkEqual("blink", eval);
    });

    testAsync("`flatMapResult` 4", eval => {
      delay(() => raise(TestException("!blink")), 1)
      -> ResultFuture.flatMapResult(result => ResultFuture.fromResult(curiousResultMapper(result)))
      -> assertErrorEqual(TestException("!blink"), eval);
    });

    testAsync("`effectOk` executes on Ok", eval => {
      let side = ref("initial value");
      delay(() => "hop hop", 1)
      -> ResultFuture.effectOk(value => { side := value ++ "!"; })
      -> ResultFuture.getOk(value => expect((value, side^)) |> toEqual(("hop hop", "hop hop!")) |> eval);
    });

    testAsync("`effectError` executes on Error", eval => {
      let side = ref("initial value");
      delay(() => raise(TestException("hop hop")), 1)
      -> ResultFuture.effectError(error => switch (error) {
        | TestException(msg) => side := msg ++ "!"
        | _ => ()
      })
      -> ResultFuture.getError(error => expect((error, side^)) |> toEqual((TestException("hop hop"), "hop hop!")) |> eval);
    });

    testAsync("`effectResult` 1", eval => {
      let side = ref("initial value");
      delay(() => "hop hop", 1)
      -> ResultFuture.effectResult(result => { side := Belt.Result.getExn(result) ++ "?" })
      -> ResultFuture.getOk(value => expect((value, side^)) |> toEqual(("hop hop", "hop hop?")) |> eval);
    });

    testAsync("`effectResult` 2", eval => {
      let side = ref("initial value");
      delay(() => raise(TestException("hop hop")), 1)
      -> ResultFuture.effectResult(result => switch (result) {
        | Error(TestException(msg)) => { side := msg ++ "!" }
        | _ => ()
      })
      -> ResultFuture.getError(error => expect((error, side^)) |> toEqual((TestException("hop hop"), "hop hop!")) |> eval);
    });

    testAsync("`waitEffectOk` executes on Ok", eval => {
      let side = ref("initial value");
      delay(() => "yellow", 1)
      -> ResultFuture.waitEffectOk(value => delay(() => { side := value }, 5) -> ResultFuture.ignore)
      -> ResultFuture.getOk(value => expect((value, side^)) |> toEqual(("yellow", "yellow")) |> eval);
    });

    testAsync("`waitEffectError` executes on Error", eval => {
      let side = ref("initial value");
      delay(() => raise(TestException("blue")), 1)
      -> ResultFuture.waitEffectError(error => switch (error) {
        | TestException(msg) => delay(() => { side := msg }, 5) -> ResultFuture.ignore
        | _ => Future.fromValue(())
      })
      -> ResultFuture.getError(error => expect((error, side^)) |> toEqual((TestException("blue"), "blue")) |> eval );
    });

    testAsync("`waitEffectResult` 1", eval => {
      let side = ref("initial value");
      delay(() => "green", 1)
      -> ResultFuture.waitEffectResult(result => delay(() => { side := Belt.Result.getExn(result) }, 5) -> ResultFuture.ignore)
      -> ResultFuture.getOk(value => expect((value, side^)) |> toEqual(("green", "green")) |> eval);
    });

    testAsync("`waitEffectResult` 2", eval => {
      let side = ref("initial value");
      delay(() => raise(TestException("blue")), 1)
      -> ResultFuture.waitEffectResult(result => switch (result) {
        | Error(TestException(msg)) => delay(() => { side := msg }, 5) -> ResultFuture.ignore
        | _ => Future.fromValue(())
      })
      -> ResultFuture.getError(error => expect((error, side^)) |> toEqual((TestException("blue"), "blue")) |> eval );
    });
  });

  describe("Negative tests", () => {

    testAsync("`mapOk` ignored on Error", eval => {
      delay(() => raise(TestException("my my")), 1)
      -> ResultFuture.mapOk(value => "¿" ++ value ++ "?")
      -> assertErrorEqual(TestException("my my"), eval);
    });

    testAsync("`mapError` ignored on Ok", eval => {
      delay(() => "hello there", 1)
      -> ResultFuture.mapError(_ => OtherTestException("exception"))
      -> assertOkEqual("hello there", eval);
    });

    testAsync("`mapOkResult` ignored on Error", eval => {
      delay(() => raise(TestException("my my")), 1)
      -> ResultFuture.mapOkResult(value => Belt.Result.Error(OtherTestException(value)))
      -> assertErrorEqual(TestException("my my"), eval);
    });

    testAsync("`mapErrorResult` ignored on Ok", eval => {
      delay(() => "hello there", 1)
      -> ResultFuture.mapErrorResult(error => Belt.Result.Ok(Js.String.make(error)))
      -> assertOkEqual("hello there", eval);
    });

    testAsync("`flatMapOk` ignored on Error", eval => {
      delay(() => raise(TestException("my my")), 1)
      -> ResultFuture.flatMapOk(value => delay(() => value ++ " miracle!", 1))
      -> assertErrorEqual(TestException("my my"), eval);
    });

    testAsync("`flatMapError` ignored on Ok", eval => {
      delay(() => "hello there", 1)
      -> ResultFuture.flatMapError(error => delay(() => Js.String.make(error) ++ " miracle healing!", 1))
      -> assertOkEqual("hello there", eval);
    });

    testAsync("`effectOk` does not execute on Error", eval => {
      let side = ref("initial");
      delay(() => raise(TestException("my my")), 1)
      -> ResultFuture.effectOk(value => { side := value })
      -> ResultFuture.getError(error => expect((error, side^)) |> toEqual((TestException("my my"), "initial")) |> eval);
    });

    testAsync("`effectError` does not execute on Ok", eval => {
      let side = ref("initial");
      delay(() => "hello there", 1)
      -> ResultFuture.effectError(error => { side := Js.String.make(error) })
      -> ResultFuture.getOk(value => expect((value, side^)) |> toEqual(("hello there", "initial")) |> eval);
    });

    testAsync("`waitEffectOk` does not execute on Error", eval => {
      let side = ref("initial");
      delay(() => raise(TestException("my my")), 1)
      -> ResultFuture.waitEffectOk(value => delay(() => { side := value }, 5) -> ResultFuture.ignore)
      -> ResultFuture.getError(error => expect((error, side^)) |> toEqual((TestException("my my"), "initial")) |> eval);
    });

    testAsync("`waitEffectError` does not execute on Ok", eval => {
      let side = ref("initial");
      delay(() => "hello there", 1)
      -> ResultFuture.waitEffectError(error => delay(() => { side := Js.String.make(error) }, 5) -> ResultFuture.ignore)
      -> ResultFuture.getOk(value => expect((value, side^)) |> toEqual(("hello there", "initial")) |> eval);
    });

    testAsync("`getOk` not executed on error", eval => {
      let side = ref("initial");
      ResultFuture.fromError(TestException("kuckoo"))
      -> ResultFuture.getOk(value => { side := value ++ "<- is possible?" });
      delay(() => (), 5)
      -> ResultFuture.getResult(_ => expect(side^) |> toEqual("initial") |> eval);
    });

    testAsync("`getError` not executed on ok", eval => {
      let side = ref("initial");
      ResultFuture.fromValue("kuckoo")
      -> ResultFuture.getError(error => { side := Js.String.make(error) })
      delay(() => (), 5)
      -> ResultFuture.getResult(_ => expect(side^) |> toEqual("initial") |> eval);
    });
  });
});
