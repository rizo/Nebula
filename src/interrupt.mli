(** Hardware and software interrupts.

    There are two kinds of interrupts in the DCPU-16: signals to hardware
    devices, and signals {i from} hardware devices. The second kind is a signal
    to the DCPU-16 with a single word message.

    Signals to devices are launched in response to {i triggers} that are
    generated through instructions. These are either
    - from the {! Special_code.Hwi} instruction, which triggers an interrupt to the designated
      hardware device. Information is passed to devices through the value of the registers at the time that the signal is sent.
    - from {! Special_code.Int} instruction, which allows software to "simulate" a hardware device
      that has generated an interrupt with the desired message

    Hardware devices can generate interrupts for the DCPU-16 to process. These
    interrupts are queued by the processor.

    @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Prelude

(** A message from a hardware device to the DCPU-16.contents

    An interrupt always includes a {i message} consisting of a single {! word}. *)
type t = Message of message

(** A signal for a device.

    When processed, triggers are sent to hardware based on the device's assigned
    index or simulate a device that has generated a message. *)
module Trigger : sig
  type t =
    | Software of message
    | Hardware of device_index
end

