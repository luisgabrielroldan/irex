defmodule IRex.Receiver do
  use GenServer

  @type receiver_config :: [
          gpio: integer(),
          frame_timeout: integer()
        ]

  @default_frame_timeout 15000
  @watcher_interval 1

  defmodule State do
    defstruct gpio: nil,
              client: nil,
              buffer: [],
              frame_timeout: nil
  end

  @spec start_link(config :: receiver_config()) :: {:ok, pid()} | {:eror, term()}
  def start_link(config) do
    client = self()
    gpio_pin = Keyword.fetch!(config, :gpio)
    frame_timeout = Keyword.get(config, :frame_timeout, @default_frame_timeout)

    GenServer.start_link(__MODULE__, [gpio_pin, client, frame_timeout])
  end

  @spec stop(pid) :: :ok
  def stop(pid),
    do: GenServer.stop(pid)

  @impl true
  def init([gpio_pin, client, frame_timeout]) do
    Process.flag(:trap_exit, true)
    Process.monitor(client)

    {:ok, gpio} = IRex.Nif.start_receiver(gpio_pin, 1, self())

    Process.send_after(self(), :watcher, @watcher_interval)
    {:ok, %State{gpio: gpio, client: client, frame_timeout: frame_timeout, buffer: []}}
  end

  @impl true
  def terminate(_reason, state) do
    finalize(state)
  end

  @impl true
  def handle_info({:recv_event, time, val}, state) do
    {:noreply, %{state | buffer: [{time / 1000, val} | state.buffer]}}
  end

  def handle_info(:watcher, state) do
    Process.send_after(self(), :watcher, @watcher_interval)

    handle_watcher(state)
  end

  def handle_info({:DOWN, _ref, :process, client, _reason}, %{client: client} = state) do
    Process.exit(self(), :client_exit)

    {:noreply, state}
  end

  def handle_watcher(%{buffer: []} = state),
    do: {:noreply, state}

  # If the last was a pulse hold on... HODOOOOOOOR!
  def handle_watcher(%{buffer: [{_, 1} | _]} = state),
    do: {:noreply, state}

  def handle_watcher(%{buffer: [{last_time, 0} | _], frame_timeout: timeout} = state) do
    if System.system_time(:microsecond) - last_time > timeout do
      case timing_decode(state.buffer) do
        {:ok, data} ->
          send(state.client, {:irex_receive, data})

        _ ->
          nil
      end

      {:noreply, %{state | buffer: []}}
    else
      {:noreply, state}
    end
  end

  defp finalize(state) do
    :ok == IRex.Nif.stop_receiver(state.gpio)
  end

  # This function decodes in reverse! Dont kill me please xD
  defp timing_decode(buffer) do
    buffer
    |> Enum.chunk_every(2)
    |> Enum.reduce_while({[], nil}, fn
      [{space_start, 0}, {mark_start, 1}], {acc, nil} ->
        mark = trunc(space_start - mark_start)
        {:cont, {[mark | acc], mark_start}}

      [{space_start, 0}, {mark_start, 1}], {acc, next_mark} ->
        mark = trunc(space_start - mark_start)
        next_space = trunc(next_mark - space_start)

        {:cont, {[mark, next_space | acc], mark_start}}

      _, _ ->
        {:error, :timing_decode}
    end)
    |> case do
      {data, _} when is_list(data) ->
        {:ok, data}

      error ->
        error
    end
  end
end
