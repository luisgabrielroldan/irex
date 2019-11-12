# IRex

Simple library to read an IR sensor.

*This is a work in progress!*

## Usage

```elixir
# Start the receiver on GPIO 18
iex(1)> {:ok, pid} = IRex.Receiver.start_link(gpio: 18)
{:ok, #PID<0.169.0>}

# Receive signals
iex(2)> flush
{:recv_event,
 [2657, 899, 438, 908, 434, 456, 435, 457, 435, 889, 881, 463, 436, 457, 436,
  457, 435, 457, 435, 457, 436, 457, 435, 457, 436, 456, 436, 457, 879, 906,
  881, 457, 438, 456, 436, 904, 437, 457, 435]}
:ok

# Stop receiver
iex(3)> IRex.Receiver.stop pid
:ok
```

## Installation

If [available in Hex](https://hex.pm/docs/publish), the package can be installed
by adding `irex` to your list of dependencies in `mix.exs`:

```elixir
def deps do
  [
    {:irex, "~> 0.1.0"}
  ]
end
```

Documentation can be generated with [ExDoc](https://github.com/elixir-lang/ex_doc)
and published on [HexDocs](https://hexdocs.pm). Once published, the docs can
be found at [https://hexdocs.pm/irex](https://hexdocs.pm/irex).

