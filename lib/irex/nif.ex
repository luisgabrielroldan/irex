defmodule IRex.Nif do
  @on_load {:load_nif, 0}
  @compile {:autoload, false}

  @moduledoc false

  def load_nif() do
    nif_binary = Application.app_dir(:irex, "priv/irex_nif")

    :erlang.load_nif(to_charlist(nif_binary), 0)
  end

  def start_receiver(_pin_number, _process) do
    :erlang.nif_error(:nif_not_loaded)
  end

  def stop_receiver(_def) do
    :erlang.nif_error(:nif_not_loaded)
  end
end
