open Prelude

type t =
  | Binary of Code.t * Address.t * Address.t
  | Unary of Special_code.t * Address.t

let rec execute = function
  | Binary (code, address_b, address_a) ->
    Computer_state.of_program (execute_binary code address_b address_a)

  | Unary (code, address_a) ->
    execute_unary code address_a

and execute_binary code address_b address_a =
  let open Code in
  let open Program in
  let open Program.Monad in
  match code with
  | Set -> Address.get address_a >>= fun a -> Address.set a address_b

and execute_unary code address_a =
  let open Computer in
  let open Computer_state in
  let open Computer_state.Monad in
  let open Program in
  let open Special_code in

  let state_a = of_program (Address.get address_a) in

  match code with
  | Jsr ->
    state_a >>= fun a ->
    of_program begin
      let open Program.Monad in
      read_special Special.PC >>= push >>= fun () ->
      write_special Special.PC a
    end

  | Int ->
    let open Interrupt_control in
    state_a >>= fun a ->
    modify begin function
        { interrupt_ctrl; _ } as c ->
        { c with
          interrupt_ctrl =
            trigger (Interrupt.Trigger.Software a) interrupt_ctrl
        }
    end

  | Ias ->
    let open Interrupt_control in
    state_a >>= fun a ->
    Computer_state.write_special Special.IA a >>= fun () ->
    if a != word 0 then
      modify begin function
          { interrupt_ctrl; _ } as c ->
          { c with
            interrupt_ctrl = enable_queuing interrupt_ctrl }
      end
    else
      unit ()
