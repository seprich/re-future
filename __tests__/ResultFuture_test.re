open Jest;
open Expect;

let delay: (unit => 'a, int) => Future.t('a) =
  (fn, timeoutMs) =>
    Future.make(setter => Js.Global.setTimeout(() => setter(fn()), timeoutMs) |> ignore);

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

  
});
