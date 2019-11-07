defmodule IRex do
  def start_receiver(gpio_pin \\ 18) do
    IRex.Nif.start_receiver(gpio_pin, self())
  end
end
