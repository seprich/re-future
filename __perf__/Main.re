
[@bs.val][@bs.scope "global"] external runGc : unit => unit = "gc";

type memoryInfo = {
  rss: float,
  heapTotal: float,
  heapUsed: float,
  [@bs.as "external"] _external: float,
};
[@bs.val][@bs.scope "process"] external memoryUsage: unit => memoryInfo = "memoryUsage";
[@bs.val][@bs.scope "process"] external processExit: unit => unit = "exit";
type buffer;
[@bs.val][@bs.scope "Buffer"] external allocBuffer: int => buffer = "alloc";
[@bs.val][@bs.scope "Buffer"] external bufferLength: buffer => float = "length";

/*
type triggerFn = unit => unit;

let createTriggerableFuture: (unit => 'b) => (Future.t('b), triggerFn) =
  fn => {
    let trigger = ref(() => ());
    let future = Future.make(setter => {
      trigger := (() => setter(fn()));
    });
    (future, trigger^);
  };
*/

let delay: (unit => 'a, int) => Future.t('a) =
  (fn, timeoutMs) =>
    Future.make(setter => Js.Global.setInterval(() => setter(fn()), timeoutMs) |> ignore);

let inMB: float => string = bytes => Js.Float.toString(Js.Math.round(bytes /. 10485.76) /. 100.0) ++ " MB";

let printStep: int => unit = n => Js.log(Js.String.slice(~from=0, ~to_=40, "#### " ++ Js.Int.toString(n) ++ " " ++ Js.String.repeat(40, "#")));
let printUsedHeap: unit => unit =
  () => {
    let profile = memoryUsage();
    Js.log("## Used Heap:     ~ " ++ inMB(profile.heapUsed));
    Js.log("## Used External: ~ " ++ inMB(profile._external));
  };

let alloc10MB =
  () => allocBuffer(1024 * 1024 * 10);

let pretendUse: buffer => unit = [%raw {| function(_x) {} |}];

let allocAndPrint =
  _x => {
    let m = alloc10MB();
    runGc();
    printStep(3);
    printUsedHeap();
    //pretendUse(m);
    delay(() => {
      pretendUse(m);
      "done";
    }, 1000);
  };


let runMemoryProfiling =
  () => {
    printStep(0);
    printUsedHeap();
    let future = allocAndPrint()
      -> Future.flatMap(allocAndPrint)
      -> Future.flatMap(allocAndPrint)
      -> Future.flatMap(allocAndPrint)
      -> Future.flatMap(allocAndPrint)
      -> Future.flatMap(allocAndPrint)
      -> Future.flatMap(allocAndPrint)
      -> Future.flatMap(allocAndPrint)
      -> Future.flatMap(allocAndPrint)
      -> Future.flatMap(allocAndPrint);
    future;
  };

runMemoryProfiling() -> Future.get(_ => {
  Js.log("DONE")
  processExit();
});
