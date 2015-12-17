(** Simulated keyboard.

    @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Common

open Functional

open Unsigned

module P = Program

type t = {
  key_buffer : uint8 option;
  interrupt_message : word option;
  interrupt_sent : bool;
}

let make : Device.t =
  object (self)
    val state = {
      key_buffer = None;
      interrupt_message = None;
      interrupt_sent = false;
    }

    method on_interaction device_input memory =
      let open Device.Input in

      IO.unit begin
        match device_input.key_code with
        | None -> self
        | Some key -> {< state = { state with key_buffer = Some key; interrupt_sent = false } >}
      end

    method on_tick =
      IO.unit begin
        match (state.key_buffer, state.interrupt_message) with
        | (Some key, Some message) when not state.interrupt_sent -> begin
            ({< state = { state with interrupt_sent = true } >}, Some (Interrupt.Message message))
          end
        | _ -> (self, None)
      end

    method on_interrupt =
      let open Program.Functor in
      let open Program.Monad in

      IO.unit begin
        P.read_register Reg.A |> map Word.to_int >>= function
        | 0 -> begin (* Clear keyboard buffer. *)
            P.Return {< state = { state with key_buffer = None } >}
          end
        | 1 -> begin (* Store key. *)
            P.write_register
              Reg.C
              (match state.key_buffer with
               | None -> word 0
               | Some key -> Word.of_int (UInt8.to_int key)) >>= fun () ->
            P.Return self
          end
        | 2 -> begin (* Check key. *)
            P.read_register Reg.B >>= fun b ->
            P.write_register
              Reg.C
              (match state.key_buffer with
               | None -> word 0
               | Some key -> begin
                   let k = Word.of_int (UInt8.to_int key) in
                   if k = b then word 1 else word 0
                 end) >>= fun () ->
            P.Return self
          end
        | 3 -> begin (* Enable interrupts. *)
            P.read_register Reg.B >>= fun b ->
            P.Return begin
              {< state =
                   { state with
                     interrupt_message = if b = word 0 then None else Some b
                   }
              >}
            end
          end
        | _ -> P.Return self
      end

    method info =
      Device.Info.{
        id = (word 0x30cf, word 0x7406);
        manufacturer = (word 0, word 0);
        version = word 1;
      }
  end
