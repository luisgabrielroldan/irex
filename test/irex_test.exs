defmodule IRexTest do
  use ExUnit.Case
  doctest IRex

  test "greets the world" do
    assert IRex.hello() == :world
  end
end
