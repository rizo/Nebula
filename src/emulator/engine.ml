(** Manage the running DCPU-16 computer.

    @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Common

open Functional

module Cs = Computer_state

type state = Computer_state.t

(** The processor was asked to complete an invalid operation. *)
exception Invalid_operation of Invalid_operation.t * Computer_state.t

module type CONTROL = sig
  type t

  val yield : state -> t -> t IO.t
end

module type CONTROL_WITH_INITIAL_STATE = sig
  include CONTROL

  val initial_state : t
end

let control_with_initial_state
    (type a)
    (module C : CONTROL with type t = a)
    (default : a)
  =
  (module struct
    include C
    let initial_state = default
  end : CONTROL_WITH_INITIAL_STATE)

module type HOOK = sig
  val invoke : state -> state IO.t
end

module type S = sig
  module Control : CONTROL_WITH_INITIAL_STATE

  val launch : suspend_every:Duration.t -> state -> 'a IO.t
end

module Make
    (Clock : Clock.S)
    (Control : CONTROL_WITH_INITIAL_STATE)
    (Cycle : HOOK)
    (Suspend : HOOK)
  : S =
struct
  module Control = Control

  let ready_to_suspend ~suspend_every last_suspension_time now =
    let elapsed = Duration.of_nanoseconds Time_stamp.(now - last_suspension_time) in
    elapsed >= suspend_every

  let launch ~suspend_every c =
    let open IO.Monad in

    let suspend_when_ready last_suspension_time now c =
      if ready_to_suspend ~suspend_every last_suspension_time now then
        Suspend.invoke c |> IO.Functor.map (fun c -> (now, c))
      else
        IO.unit (last_suspension_time, c)
    in

    let rec loop last_suspension_time control_state c =
      match c.Cs.state_error with
      | Some err -> IO.throw (Invalid_operation (err, c))
      | None -> begin
          Control.yield c control_state >>= fun control_state ->
          Cycle.invoke c >>= fun c ->
          Clock.get_time >>= fun now ->

          (suspend_when_ready
            last_suspension_time
            now
            c) >>= fun (last_suspension_time, c) ->
          loop last_suspension_time control_state c
        end
    in
    Clock.get_time >>= fun now ->
    loop now Control.initial_state c
end
