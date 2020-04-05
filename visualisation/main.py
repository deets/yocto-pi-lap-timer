import threading
import queue
import re
import rx.subject

from bokeh.models import ColumnDataSource
from bokeh.plotting import curdoc, figure
from bokeh.layouts import column, row
from bokeh.models import FactorRange, Range1d
from bokeh.models import Select

from nanomsgnode import PropellerNodeController, CHANNEL_NAMES

RSSI_HISTORY = 200


class PropellerTimestampProcessor:

    CPUFREQ = 80_000_000

    def __init__(self):
        self._last_ts = None
        self.signal = rx.subject.Subject()

    def __call__(self, timestamp):
        if self._last_ts is not None:
            diff = (timestamp + 2**32 - self._last_ts) % 2**32
            self.signal.on_next(diff / self.CPUFREQ)
        self._last_ts = timestamp


class Visualisation:

    def __init__(self, node):
        self._node = node
        self._timestamp_processor = PropellerTimestampProcessor()
        self._lines_q = queue.Queue()

        number_of_vtx = node.configuration()[0]
        self._laptime_format = "<I" + "I" * number_of_vtx

        doc = self._doc = curdoc()

        scanner_data = dict(
            channel_names=CHANNEL_NAMES,
            left=list(range(0, 80, 2)),
            right=list(range(1, 81, 2)),
            bottom=[0] * 40,
            readings=list([20] for _ in range(1, 41))
        )

        self._scanner_source = ColumnDataSource(
                data=scanner_data,
        )
        # the aspict ratio is maintained over
        # the scaling parameter.
        scanner_figure = figure(
            width=600,
            height=100,
            x_range=FactorRange(*CHANNEL_NAMES),
            y_range=Range1d(0, 1024),
        )
        scanner_figure.vbar(
            x="channel_names",
            top="readings", width=0.9,
            alpha=0.5,
            source=self._scanner_source,
        )

        mode_selector = Select(
            value="idle",
            options=["idle", "scanner", "laptimer"],
        )

        mode_selector.on_change(
            'value',
            lambda attr, old, new: setattr(node, "mode", new)
        )
        laptime_row, self._laptime_sources = self._setup_laptimer(number_of_vtx)
        time_diff_graph = self._setup_timediff_graph(self._timestamp_processor.signal)
        layout = column(
            children=[
                mode_selector,
                scanner_figure,
                laptime_row,
                time_diff_graph,
            ],
            sizing_mode="scale_width"
        )
        doc.add_root(layout)

    def start_background_reader(self):
        t = threading.Thread(target=self._background_task)
        t.daemon = True
        t.start()

    def _setup_timediff_graph(self, signal):
        p = figure(
            y_axis_label="time delta",
            width=600,
            height=50,
        )
        data = dict(
            x=[i for i in range(RSSI_HISTORY)],
            timedelta=[0] * RSSI_HISTORY
            )
        td_source = ColumnDataSource(
            data=data
        )
        p.circle(x='x', y="timedelta", source=td_source)

        def update_td(tdiff):
            timedelta = td_source.data['timedelta']
            x = td_source.data['x']
            x = x[1:] + [x[-1] + 1]
            timedelta = timedelta[1:]
            timedelta.append(tdiff * 1000)  # convert to ms
            patch = dict(
                x=[(slice(0, RSSI_HISTORY), x)],
                timedelta=[(slice(0, RSSI_HISTORY), timedelta)],
            )
            td_source.patch(patch)

        signal.subscribe(on_next=update_td)
        return p

    def _scanning(self, results):
        parts = results.decode("ascii").split(":")
        m = re.match(r"(\d+)", parts[0])
        if m:
            count = int(m.group(1))
            assert len(parts) == count * 2 + 2
            payload = parts[1:-1]
            readings = self._scanner_source.data["readings"]
            for channel, value in zip(payload[::2], payload[1::2]):
                channel, value = int(channel), int(value)
                readings[channel] = value

            # a patch actually consists of positions
            # and new values as list of tuples
            patch = dict(
                readings=[(slice(0, 40), readings)],
            )
            self._scanner_source.patch(patch)

    def _setup_laptimer(self, number_of_vtx):
        laptime_sources = []
        laptime_rows = []
        for i in range(number_of_vtx):
            p = figure(y_axis_label="rssi", y_range=Range1d(0, 1024))
            data = dict(
                x=[i for i in range(RSSI_HISTORY)],
                rssi=[100 * (i + 1)] * RSSI_HISTORY
            )
            lt_source = ColumnDataSource(
                data=data
            )
            p.circle(x='x', y="rssi", source=lt_source)
            laptime_sources.append(lt_source)
            tuner = Select(
                value=CHANNEL_NAMES[0],
                options=list(CHANNEL_NAMES)
                )
            tuner.on_change(
                'value',
                lambda attr, old, new, rtc=i: self._node.tune(rtc, new)
            )
            # ensure our default state is correct
            self._node.tune(i, CHANNEL_NAMES[0])
            vtx_column = column(children=[tuner, p], sizing_mode="scale_width")
            laptime_rows.append(vtx_column)

        return (
            row(children=laptime_rows, sizing_mode="stretch_both"),
            laptime_sources,
        )

    def _laptime(self, results):
        timestamp, *entries = results.split(":")
        self._timestamp_processor(int(timestamp, 16))
        for number, entry in enumerate(entries):
            value = int(entry, 16)
            lt_source = self._laptime_sources[number]
            rssi = lt_source.data['rssi'][1:]
            rssi.append(value)
            patch = dict(rssi=[(slice(0, len(rssi)), rssi)])
            lt_source.patch(patch)

    def _background_task(self):
        while True:
            for line in self._node.readlines():
                self._lines_q.put(line)
            self._doc.add_next_tick_callback(self._process_lines)

    def _process_lines(self):
        # For some reason we get multiple callbacks
        # in the mainloop. So instead of relying on
        # one callback per line, we gather them
        # and process as many of them as we find.
        for _ in range(self._lines_q.qsize()):
            line = self._lines_q.get()
            command = line[0]
            results = line[1:]
            callback = {
                "s": self._scanning,
                "l": self._laptime,
             }.get(command, lambda results: print("unknown:", command))
            callback(results)


def main():
    node_controller = PropellerNodeController("tcp://fpv-laptimer.local:5000")
    visualisation = Visualisation(node_controller)
    visualisation.start_background_reader()


main()
