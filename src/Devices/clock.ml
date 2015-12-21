(** Simulated hardware clock.

    @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Common

open Functional

module P = Program

type t = {
  on : bool;
  divider : word;
  elapsed_ticks : word;
  last_tick_time : Time_stamp.t;
  interrupt_message : word option
}

(** The simulated clock period, in nanoseconds. *)
let base_clock_period =
  Duration.of_nanoseconds 16666666L

let effective_period t =
  Int64.mul
    (Word.to_int t.divider |> Int64.of_int)
    (Duration.nanoseconds base_clock_period)

let make : Device.t IO.t =
  Precision_clock.get_time |> IO.Functor.map begin fun time ->
    object (self)
      val state = {
        on = true;
        divider = word 1;
        elapsed_ticks = word 0;
        last_tick_time = time;
        interrupt_message = None
      }

      method on_tick =
        let open IO.Monad in
        let open IO.Functor in

        if not state.on then IO.unit (self, None)
        else
          Precision_clock.get_time |> map begin fun time ->
            let since_last_tick = Time_stamp.(time - state.last_tick_time) in

            if since_last_tick >= effective_period state then
              ({< state = {
                   state with
                   last_tick_time = time;
                   elapsed_ticks = Word.(state.elapsed_ticks + word 1)
                 } >},
               state.interrupt_message |> Option.Functor.map (fun m -> Interrupt.Message m))
            else
              (self, None)
          end

      method on_interaction device_input memory =
        IO.unit self

      method on_interrupt =
        let open Program.Functor in
        let open Program.Monad in

        IO.unit begin
          P.read_register Reg.A |> map Word.to_int >>= function
          | 0 -> begin
              P.read_register Reg.B >>= fun b ->
              P.Return begin
                {< state =
                     if b = word 0 then { state with on = false }
                     else { state with on = true; divider = b } >}
              end
            end
          | 1 -> P.write_register Reg.C state.elapsed_ticks >>= fun () -> P.Return self
          | 2 -> begin
              P.read_register Reg.B >>= fun b ->
              P.Return begin
                {< state =
                     { state with interrupt_message = if b = word 0 then None else Some b } >}
              end
            end
          | _ -> P.Return self
        end

      method info =
        Device.Info.{
          id = (word 0x12d0, word 0xb402);
          manufacturer = (word 0, word 0);
          version = word 1
        }
    end
end
