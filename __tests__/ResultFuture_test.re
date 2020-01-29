open Jest;
open Expect;

let delay: (unit => 'a, int) => ResultFuture.gt('a, 'e) =
  (fn, timeoutMs) =>
    ResultFuture.make((resolve, reject) =>
      Js.Global.setTimeout(() => try (resolve(fn())) {
        | error => reject(error)
      }, timeoutMs) |> ignore);

let jsError: unit => exn = [%raw {| function() { return new Error("js-error"); } |}];

exception TestException(string);

type errorsOfNinthHell = [
  | `Cuckoo
  | `ChocoLatte
  | `ChamoMilly
];

describe("ResultFuture", () => {

  testAsync("`fromValue`", eval => {
    ResultFuture.fromValue("quark")
    -> ResultFuture.getOk(value => expect(value) |> toEqual("quark") |> eval);
  });

  testAsync("`fromError`", eval => {
    ResultFuture.fromError(`ChocoLatte)
    -> ResultFuture.getError(error => expect(error) |> toEqual(`ChocoLatte) |> eval);
  });

  testAsync("`getResult` of an ok result", eval => {
    ResultFuture.fromValue("quark")
    -> ResultFuture.getResult(result => expect(result) |> toEqual(Belt.Result.Ok("quark")) |> eval);
  });

  testAsync("`make` ResultFuture which evaluates to Ok", eval => {
    ResultFuture.make(
      (resolve, _reject) => Js.Global.setTimeout(() => resolve("munchday"), 5) |> ignore
    )
    -> ResultFuture.getOk(value => expect(value) |> toEqual("munchday") |> eval);
  });

  testAsync("`make` ResultFuture which evaluates to Error", eval => {
    ResultFuture.make(
      (_resolve, reject) => Js.Global.setTimeout(() => reject(`ChamoMilly), 5) |> ignore
    )
    -> ResultFuture.getError(error => expect(error) |> toEqual(`ChamoMilly) |> eval);
  });

  
});
