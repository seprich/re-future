
[@bs.val][@bs.scope "global"] external runGc : unit => unit = "gc";

type memoryInfo = {
  rss: float,
  heapTotal: float,
  heapUsed: float,
  [@bs.as "external"] _external: float,
};
[@bs.val][@bs.scope "process"] external memoryUsage: unit => memoryInfo = "memoryUsage";
type buffer;
[@bs.val][@bs.scope "Buffer"] external allocBuffer: int => buffer = "alloc";


let delay: (unit => 'a, int) => Future.t('a) =
  (fn, timeoutMs) =>
    Future.make(setter => Js.Global.setTimeout(() => setter(fn()), timeoutMs) |> ignore);

let printUsedMemory: unit => unit =
  () => {
    let inMB: float => string = bytes => Js.Float.toString(Js.Math.round(bytes /. 10485.76) /. 100.0) ++ " MB";
    let profile = memoryUsage();
    Js.log(Js.String.repeat(40, "#"));
    Js.log("## Used Heap:     ~ " ++ inMB(profile.heapUsed));
    Js.log("## Used External: ~ " ++ inMB(profile._external));
  };

let alloc10MB =
  () => allocBuffer(1024 * 1024 * 10);

let pretendUse: buffer => unit = [%raw {| function(_x) {} |}];

let allocAndPrint =
  (_) => {
    let m = alloc10MB();
    runGc();
    printUsedMemory();
    delay(() => {
      pretendUse(m);
      "done";
    }, 100);
  };

let runMemoryProfilingAllocateAndFree: unit => Future.t(unit) =
  () => {
    Js.log("\nMemory Allocation and Freeing Test - Total memory allocation should stay stable after 2nd printout");
    printUsedMemory();
    let future = allocAndPrint()
      -> Future.flatMap(allocAndPrint)
      -> Future.flatMap(allocAndPrint)
      -> Future.flatMap(allocAndPrint)
      -> Future.flatMap(allocAndPrint)
      -> Future.flatMap(allocAndPrint);
    future -> Future.map(ignore) -> Future.effect(() => Js.log("DONE: Memory Allocation and Freeing Test"));
  };

let runMemoryProfilingHeapOfFunctions: unit => Future.t(unit) =
  () => {
    let futile = Future.make(_resolve => ()); // Future which never resolves :)
    let rec piler = (future, times) => if (times > 0) piler(Future.flatMap(future, allocAndPrint), times - 1) else future;
    let log = txt => Js.log(Js.String.repeat(40, "#") ++ "\n## " ++ txt ++ ":");

    Js.log("\nMemory Usage, when piling functions for future execution without resolving anything");
    log("Initial");
    printUsedMemory();
    log("Piling 5 functions");
    let futile2 = piler(futile, 5);
    printUsedMemory();
    log("Piling 10 more functions");
    let futile3 = piler(futile2, 10);
    printUsedMemory();
    log("Piling 100 more functions");
    let futile4 = piler(futile3, 100);
    printUsedMemory();
    log("Piling 1000 more functions");
    let futile5 = piler(futile4, 1000);
    printUsedMemory();
    Js.log("## (Just ignore :" ++ Js.String.make(futile5) ++ ")");
    Js.log("DONE: Memory Usage");
    Future.fromValue(());
  };

let runAll: unit => Js.Promise.t(unit) =
  () => {
    Future.fromValue(())
    -> Future.flatMap(runMemoryProfilingAllocateAndFree)
    -> Future.flatMap(runMemoryProfilingHeapOfFunctions)
    -> Future.toPromise;
  };

runAll();
