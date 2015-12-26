(** Property-based testing libraries.

    @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Functional

module L = Lazy_list

module type RANDOM_ENGINE = sig
  (** State of the random engine. *)
  type t

  (** Generate a random {! int64 } and the updated state. *)
  val next_int64 : t -> t * int64
end

module Default_random_engine = struct
  type t = Random.State.t

  let next_int64 t =
    Random.set_state t;
    let x = Random.int64 Int64.max_int in
    (Random.get_state (), x)

  let with_seed seed =
    Random.init seed;
    Random.get_state ()
end

module Gen = struct
  module type S = sig
    type 'a t

    type engine

    val sample : engine -> 'a t -> engine * 'a

    val unit : 'a -> 'a t

    val int : int t

    val alpha : char t

    val non_negative_int : int t

    val choose_int : low:int -> high:int -> int t

    val bool : bool t

    val pair : 'a t -> ('a * 'a) t

    val list_of_n : int -> 'a t -> 'a list t

    val list_of : 'a t -> (int -> 'a list t)

    val string_of_n : int -> char t -> string t

    val string_of : char t -> (int -> string t)

    val union : 'a t -> 'a t -> 'a t

    val choice : 'a t list -> 'a t option

    val where : ('a -> bool) -> 'a t -> 'a t

    module Functor_instance : Functor_class.S with type 'a t = 'a t

    module Functor : module type of Functor_class.Extend (Functor_instance)

    module Monad_instance : Monad_class.S with type 'a t = 'a t

    module Monad : module type of Monad_class.Extend (Monad_instance)
  end

  module Make (R : RANDOM_ENGINE) : (S with type engine = R.t) = struct
    type engine = R.t

    module Random = State.Make (R)

    open Random.Monad
    open Random.Functor

    type 'a t = 'a Random.t

    let sample engine t =
      Random.run engine t

    let unit a =
      Random.unit a

    let int =
      Random.lift R.next_int64 |> map Int64.to_int

    let non_negative_int =
      int |> map abs

    let choose_int ~low ~high =
      non_negative_int |> map (fun x -> (x mod (high - low)) + low)

    let bool =
      int |> map (fun x -> x mod 2 = 0)

    let pair g =
      g >>= fun x ->
      g >>= fun y ->
      unit (x, y)

    let fill n a =
      let rec loop accum i =
        if i >= n then List.rev accum
        else loop (a :: accum) (i + 1)
      in
      loop [] 0

    let string_of_char ch =
      String.make 1 ch

    let list_of_n n t =
      Random.Monad.sequence (fill n t)

    let list_of t =
      fun n ->
        list_of_n n t

    let string_of_n n g =
      fill n (map string_of_char g)
      |> sequence
      |> map (String.concat "")

    let string_of g =
      fun n ->
        string_of_n n g

    let union x y =
      bool >>= function
      | true -> x
      | false -> y

    let choice gs =
      gs |> L.of_list |> L.reduce (fun g lg -> union g (Lazy.force lg))

    let rec where p t =
      t >>= fun a -> if p a then unit a else where p t

    let alpha =
      let lowercase = Char.code 'a' in
      let uppercase = Char.code 'A' in

      union
        (choose_int lowercase (lowercase + 26))
        (choose_int uppercase (uppercase + 26))
      |> map Char.chr

    module Functor_instance = Random.Functor_instance

    module Functor = Random.Functor

    module Monad_instance = Random.Monad_instance

    module Monad = Random.Monad
  end
end

type failed_case = string

type failing_sample = string

type test_cases = int

type success_count = int

type max_size = int

module Result = struct
  type t =
    | Falsified of failed_case option * failing_sample option * success_count
    | Proved
    | Passed of success_count

  let falsified = function
    | Falsified _ -> true
    | Proved | Passed _ -> false
end

module Prop = struct
  module type S = sig
    type t

    type engine

    type 'a generator

    val pass : t

    val proved : t

    val ( && ) : t -> t -> t

    val check : ?label:string -> (unit -> bool) -> t

    val for_all : ?label:string -> ?show:('a -> string)
      -> 'a generator
      -> ('a -> bool)
      -> t

    val for_all_sizes : ?label:string
      -> (int -> 'a generator)
      -> ('a -> bool)
      -> t

    val run : ?test_cases:test_cases -> ?max_size:max_size -> engine -> t -> Result.t
  end

  module Make (G : Gen.S)
    : (S with type engine = G.engine
          and type 'a generator = 'a G.t) =
  struct
    type engine = G.engine

    type 'a generator = 'a G.t

    type t = engine -> test_cases -> max_size -> Result.t

    let pass : t =
      fun _ _ _ -> Result.Passed 0

    let proved : t =
      fun _ _ _ -> Result.Proved

    let check ?label p =
      fun _ _ _ ->
        if (p ()) then Result.Proved
        else Result.Falsified (label, None, 0)

    let random_values gen engine =
      L.unfold engine (fun engine -> Some (G.sample engine gen))

    let run ?(test_cases = 100) ?(max_size = 100) engine t =
      t engine test_cases max_size

    let pand p1 p2 =
      fun engine test_cases max_size ->
        let r = p1 |> run ~test_cases ~max_size engine in
        if Result.falsified r then r else p2 |> run ~test_cases ~max_size engine

    let ( && ) p1 p2 =
      pand p1 p2

    let for_all ?label ?show gen p =
      fun engine n _ ->
        L.zip (L.enum_from 0) (random_values gen engine)
        |> L.take n
        |> L.map (fun (index, a) ->
            if p a then Result.Passed index
            else
              Result.Falsified (
                label,
                (match show with None -> None | Some f -> Some (f a)),
                index))
        |> L.find Result.falsified
        |> Option.get_or_else (lazy (Result.Passed n))

    let for_all_sizes ?label sgen p =
      let module Infix = struct let ( && ) = pand end in

      fun engine test_cases max_size ->
        let cases_per_size = (test_cases + (max_size - 1)) / max_size in
        let props =
          L.enum_from 0
          |> L.take ((min test_cases max_size) + 1)
          |> L.map (fun i -> for_all ?label (sgen i) p)
        in
        let combined =
          props
          |> L.map (fun p ->
              fun engine _ max_size -> p |> run ~test_cases:cases_per_size ~max_size engine)
          |> L.fold_right (lazy pass) (fun a lb -> Infix.(a && (Lazy.force lb)))
        in
        combined |> run ~test_cases ~max_size engine
  end
end

module Simple_gen = struct
  include Gen.Make (Default_random_engine)

  let sample_io t =
    IO.lift begin fun () ->
      let seed = Random.bits () in
      t |> sample (Default_random_engine.with_seed seed) |> snd
    end
end

module Simple_prop = struct
  include Prop.Make (Simple_gen)

  let run_io ?test_cases ?max_size t =
    IO.lift begin fun () ->
      let seed = Random.bits () in
      t |> run ?test_cases ?max_size (Default_random_engine.with_seed seed)
    end
end

module Suite = struct
  type t =
    | Single of Simple_prop.t
    | Group of string * t list

  let rec to_props = function
    | Single p -> [p]
    | Group (_, ps) -> List.map to_props ps |> List.flatten
end

module Runner = struct
  open IO.Monad

  module Outcome = struct
    type t =
      | All_passed
      | Failures
  end

  let exec ?test_cases ?max_size s =
    let sprintf = Printf.sprintf in

    IO_ref.make Outcome.All_passed >>= fun status_ref ->

    let rec loop depth = function
      | Suite.Single prop -> begin
          IO.put_string (String.make depth ' ') >>= fun () ->
          Simple_prop.run_io ?test_cases ?max_size prop >>= function
          | Result.Falsified (message, case, n) -> begin
              IO_ref.set Outcome.Failures status_ref >>= fun () ->
              IO.put_string
                (sprintf "! Falsified%s after %d passed tests%s\n"
                   (match case with None -> " " | Some a -> " by `" ^ a ^ "`")
                   n
                   (match message with None -> "" | Some m -> ": \"" ^ m ^ "\" "))
            end
          | Result.Proved -> IO.put_string "+ OK. Proved property.\n"
          | Result.Passed success_count -> begin
              IO.put_string (sprintf "+ OK. Passed %d tests.\n" success_count)
            end
        end
      | Suite.Group (label, props) -> begin
          let header = String.make depth ' ' ^ String.make depth ':' in
          IO.put_string (header ^ " " ^ label ^ "\n") >>= fun () ->
          IO.Monad.sequence_unit (List.map (loop (depth + 1)) props) >>= fun () ->
          IO.put_string "\n"
        end
    in
    loop 1 s >>= fun () -> IO_ref.get status_ref
end

module Dsl = struct
  module Engine = Default_random_engine

  module Gen = Simple_gen

  module Prop = Simple_prop

  module Suite = Suite

  module Outcome = Runner.Outcome

  let check ?label p =
    Suite.Single (Simple_prop.check ?label p)

  let for_all ?label ?show gen p =
    Suite.Single (Simple_prop.for_all ?label ?show gen p)

  let for_all_sizes ?label sgen p =
    Suite.Single (Simple_prop.for_all_sizes ?label sgen p)

  let group label ps =
    Suite.Group (label, ps)

  let run ?test_cases ?max_size s =
    Runner.exec ?test_cases s

  let run_and_terminate ?test_cases ?max_size s =
    let open IO.Monad in

    run ?test_cases ?max_size s >>= function
    | Outcome.Failures -> IO.terminate 1
    | Outcome.All_passed -> IO.terminate 0

  let show_int =
    string_of_int
end

module Examples = struct
  open Dsl

  let int =
    group "int" [
      for_all ~label:"zero"
        Gen.int
        (fun x -> x + 0 = x);

      for_all ~label:"associate"
        Gen.(int |> pair)
        (fun (x, y) -> x + y = y + x);

      check ~label:"add"
        (fun () -> 2 + 3 = 5)
    ]
end
