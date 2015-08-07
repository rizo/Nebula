open IO.Monad
open Prelude

type t = {
  last_interrupt_time : float;
}

let make =
  IO.now |> IO.Functor.map (fun time -> { last_interrupt_time = time })

let clock_period =
  3.0

let visit c t =
  IO.now |> IO.Functor.map (fun time ->
      let since_last_interrupt = time -. t.last_interrupt_time in
      if since_last_interrupt >= clock_period then
        (Computer.{
            c with
            interrupt_ctrl =
              Interrupt_control.receive (Interrupt.Message (word 7)) c.interrupt_ctrl
          },
         { last_interrupt_time = time })
      else (c, t))

let info =
  Device.Info.{
    id = (word 0x30cf, word 0x7406);
    manufacturer = (word 0, word 0);
    version = word 1;
}
