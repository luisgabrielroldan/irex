defmodule IRex.ReceiverTest do
  use ExUnit.Case

  alias IRex.Receiver

  alias Circuits.GPIO

  test "start server" do
    {:ok, ref} = GPIO.open(0, :output)
    {:ok, pid} = Receiver.start_link(gpio: 1)

    GPIO.write(ref, 1)
    GPIO.write(ref, 1)
    :timer.sleep(1)
    GPIO.write(ref, 0)
    :timer.sleep(2)
    GPIO.write(ref, 1)
    # :timer.sleep(100)
    # GPIO.write(ref, 1)
    # GPIO.write(ref, 0)
    :timer.sleep(1)
    GPIO.write(ref, 0)

    :timer.sleep(1000)
  end

  defp usleep(timeout, start \\ nil) do
    start = start || System.monotonic_time(:microsecond)

    if System.monotonic_time(:microsecond) - start >= timeout do
      :ok
    else
      usleep(timeout, start)
    end
  end
end
