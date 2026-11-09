# Deterministic test rig for the crew box (IronPython 2.7, inside the Renode
# monitor). It replaces the rest of the intercom:
#
#   - CCU emulator on LPUART3: one downlink frame per millisecond of virtual
#     time (sequence number, vehicle speed, optional command), which can be
#     stopped (CCU dead), thinned (every n-th frame dropped) or corrupted
#   - uplink decoder: box state, BIT bits and the rotating telemetry item
#   - bypass relay model: the relay is energised while its drive (GPIO1_IO09,
#     the green LED) toggled within the last RELAY_HOLD_MS (charge pump)
#
# Robot keywords drive it with `python "rig...."` monitor commands.

import clr

clr.AddReference("System")
from System import Action  # noqa: E402

from Antmicro.Renode.Time import TimeInterval  # noqa: E402

SYNC0, SYNC1, T_DOWN, T_UP = 0xA5, 0x5A, 0x01, 0x81
RELAY_HOLD_MS = 2.0


def crc16(data):
    crc = 0xFFFF
    for b in data:
        crc ^= (b << 8) & 0xFFFF
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1021) & 0xFFFF if crc & 0x8000 else (crc << 1) & 0xFFFF
    return crc


def down_frame(seq, speed_ckmh, cmd=0, arg=0, mode=0, flags=0):
    p = [seq & 0xFF, mode, flags, (speed_ckmh >> 8) & 0xFF, speed_ckmh & 0xFF, cmd,
         (arg >> 8) & 0xFF, arg & 0xFF, 0]
    body = [len(p), T_DOWN] + p
    c = crc16(body)
    return [SYNC0, SYNC1] + body + [c >> 8, c & 0xFF]


class Rig(object):
    def __init__(self, machine):
        self.m = machine
        self.uart = machine["sysbus.lpuart3"]
        self.uart.CharReceived += self._on_char
        self.seq = 0
        self.speed_ckmh = 0
        self.alive = True
        self.drop_every = 0
        self.corrupt_every = 0
        self.cmd = None
        self.sent = 0
        self.last_sent_ms = -1.0
        self.buf = []
        self.need = 0
        self.ups = 0
        self.up_crc_bad = 0
        self.state = -1
        self.bits = 0
        self.tele = {}
        self.state_log = []
        self.toggles = 0
        self.last_toggle_ms = -1.0
        led = machine["sysbus.gpio1.green_led"]
        led.StateChanged += self._on_led
        self._schedule()

    # ---- time ----------------------------------------------------------------------
    def now_ms(self):
        return self.m.ElapsedVirtualTime.TimeElapsed.TotalMilliseconds

    # ---- CCU emulator --------------------------------------------------------------------
    def _schedule(self):
        self.m.ScheduleAction(TimeInterval.FromMicroseconds(1000), Action[TimeInterval](self._tick))

    def _tick(self, _i):
        self._schedule()
        if not self.alive:
            return
        self.seq = (self.seq + 1) & 0xFF
        if self.drop_every and (self.seq % self.drop_every) != 0:
            return
        if self.cmd is not None:
            cmd, arg = self.cmd
            self.cmd = None
        else:
            cmd, arg = 0, 0
        f = down_frame(self.seq, self.speed_ckmh, cmd, arg)
        if self.corrupt_every and (self.seq % self.corrupt_every) == 0:
            f[7] ^= 0x10
        for b in f:
            self.uart.WriteChar(b)
        self.sent += 1
        self.last_sent_ms = self.now_ms()

    def ccu(self, alive):
        self.alive = bool(alive)
        return self.now_ms()

    def set_speed(self, kmh):
        self.speed_ckmh = int(round(float(kmh) * 100))

    def command(self, cmd, arg=0):
        self.cmd = (int(cmd), int(arg) & 0xFFFF)

    def thin(self, every):
        """Send only every n-th frame (0 = all)."""
        self.drop_every = int(every)

    def corrupt(self, every):
        self.corrupt_every = int(every)

    # ---- uplink decoder --------------------------------------------------------------------
    def _on_char(self, c):
        b = int(c) & 0xFF
        buf = self.buf
        if len(buf) == 0:
            if b == SYNC0:
                buf.append(b)
            return
        if len(buf) == 1:
            if b == SYNC1:
                buf.append(b)
            elif b != SYNC0:
                del buf[:]
            return
        buf.append(b)
        if len(buf) == 3:
            self.need = 4 + b + 2
        if len(buf) >= 4 and len(buf) == self.need:
            body = buf[2:-2]
            if crc16(body) == ((buf[-2] << 8) | buf[-1]) and buf[3] == T_UP:
                p = buf[4:-2]
                st = p[2]
                if st != self.state:
                    self.state_log.append((self.now_ms(), st))
                self.state = st
                self.bits = p[3]
                self.tele[p[5]] = (p[6] << 8) | p[7]
                self.ups += 1
            else:
                self.up_crc_bad += 1
            del buf[:]

    def tele_item(self, i):
        return self.tele.get(int(i), -1)

    def first_time_in(self, state, after_ms=0.0):
        for t, s in self.state_log:
            if s == int(state) and t >= float(after_ms):
                return t
        return -1.0

    def transitions_after(self, after_ms):
        return len([1 for t, _ in self.state_log if t >= float(after_ms)])

    # ---- bypass relay ------------------------------------------------------------------------
    def _on_led(self, _led, _state):
        self.toggles += 1
        self.last_toggle_ms = self.now_ms()

    def relay_energised(self):
        return self.last_toggle_ms >= 0 and (self.now_ms() - self.last_toggle_ms) < RELAY_HOLD_MS

    def relay_drop_ms(self):
        """Time the relay dropped (last toggle + hold), or -1 while it is energised."""
        if self.relay_energised():
            return -1.0
        return self.last_toggle_ms + RELAY_HOLD_MS

    # ---- firmware symbols ----------------------------------------------------------------------
    def _sym(self, name):
        return self.m["sysbus"].GetSymbolAddress(name)

    def read_u32(self, name):
        return int(self.m["sysbus"].ReadDoubleWord(self._sym(name)))

    def write_u32(self, name, value):
        self.m["sysbus"].WriteDoubleWord(self._sym(name), int(value) & 0xFFFFFFFF)
