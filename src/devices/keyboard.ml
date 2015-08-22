(** Simulated keyboard. *)

open Functional
open Prelude

open Unsigned

type t = {
  key_buffer : uint8 option;
  interrupt_message : word option;
  interrupt_sent : bool;
}

let make = {
  key_buffer = None;
  interrupt_message = None;
  interrupt_sent = false;
}

let on_interaction device_input memory t =
  let open Device.Input in

  IO.unit begin
    match device_input.key_code with
    | None -> t
    | Some key -> { t with key_buffer = Some key; interrupt_sent = false; }
  end

let on_tick t =
  IO.unit begin
    match (t.key_buffer, t.interrupt_message) with
    | (Some key, Some message) when not t.interrupt_sent -> begin
        ({ t with interrupt_sent = true }, Some (Interrupt.Message message))
      end
    | _ -> (t, None)
  end

let on_interrupt t =
  let open Program in
  let open Program.Functor in
  let open Program.Monad in

  IO.unit begin
    read_register Reg.A |> map Word.to_int >>= function
    | 0 -> begin (* Clear keyboard buffer. *)
        Return { t with key_buffer = None }
      end
    | 1 -> begin (* Store key. *)
        write_register
          Reg.C
          (match t.key_buffer with
           | None -> word 0
           | Some key -> Word.of_int (UInt8.to_int key)) >>= fun () ->
        Return t
      end
    | 2 -> begin (* Check key. *)
        read_register Reg.B >>= fun b ->
        write_register
          Reg.C
          (match t.key_buffer with
           | None -> word 0
           | Some key -> begin
               let k = Word.of_int (UInt8.to_int key) in
               if k = b then word 1 else word 0
             end) >>= fun () ->
        Return t
      end
    | 3 -> begin (* Enable interrupts. *)
        read_register Reg.B >>= fun b ->
        Return {
          t with
          interrupt_message = if b = word 0 then None else Some b;
        }
      end
    | _ -> Return t
  end

let info =
  Device.Info.{
    id = (word 0x30cf, word 0x7406);
    manufacturer = (word 0, word 0);
    version = word 1;
  }
