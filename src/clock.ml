open Prelude

type t = {
  on : bool;
  divider : word;
  elapsed_ticks : word;
  last_tick_time : int;
  interrupt_message : word option;
}

let base_clock_period =
  16666666

let make =
  Precision_clock.get_time |> IO.Functor.map (fun time -> {
        on = true;
        divider = word 1;
        elapsed_ticks = word 0;
        last_tick_time = time;
        interrupt_message = None;
      })

let on_interaction t =
  IO.unit t

let on_tick t =
  let open IO.Monad in
  let open IO.Functor in
  if not t.on then IO.unit (t, None)
  else
    Precision_clock.get_time |> map (fun time ->
        let since_last_tick = time - t.last_tick_time in
        let effective_period = (Word.to_int t.divider) * base_clock_period in

        if since_last_tick >= effective_period then
          ({ t with
             last_tick_time = time;
             elapsed_ticks = Word.(t.elapsed_ticks + word 1);
           },
           t.interrupt_message |> Option.Functor.map (fun m -> Interrupt.Message m ))
        else
          (t, None))

let on_interrupt message t =
  let open Program in
  let open Program.Functor in
  let open Program.Monad in
  read_register Reg.A |> map Word.to_int >>= function
  | 0 -> begin
      read_register Reg.B >>= fun b ->
      Return (if b = word 0 then { t with on = false } else { t with on = true; divider = b; })
    end
  | 1 -> write_register Reg.C t.elapsed_ticks >>= fun () -> Return t
  | 2 -> begin
      read_register Reg.B >>= fun b ->
      Return { t with interrupt_message = (if b = word 0 then None else Some b) }
    end
  | _ -> Return t

let info =
  Device.Info.{
    id = (word 0x30cf, word 0x7406);
    manufacturer = (word 0, word 0);
    version = word 1;
}
