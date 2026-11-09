"""Latency budgets (Wasson 2016 ch. 31: allocation, design-to value, margin).

Every element is a worst-case value with its basis.  Converter delays from
TI SLAA557 [R37]; network bound from the AVB class A figure [R36]; the
others follow from the design (frame length, bus bit rate).
"""

from __future__ import annotations

from dataclasses import dataclass

from . import data

# Requirements (design-to limits) and where they come from
LIMITS_MS = {
    "sidetone": (10.0, "half of the 20 ms own-voice 'disturbing' delay [R20][R21]"),
    "intercom": (20.0, "co-located crew; own-voice disturbance limit 20 ms used as a proxy [R20][R21] (A)"),
    "radio_box": (10.0, "share of the 150 ms G.114 mouth-to-ear budget given to the intercom [R24] (A)"),
}


@dataclass
class Element:
    name: str
    ms: float
    basis: str


def converters(fs, filt="A"):
    return (
        Element("ADC decimation filter", data.ADC_GD[filt] / fs * 1e3, "%d/fs [R37]" % data.ADC_GD[filt]),
        Element("DAC interpolation filter", data.DAC_GD[filt] / fs * 1e3, "%d/fs [R37]" % data.DAC_GD[filt]),
    )


def sidetone_path(fs=16000.0, frame_ms=1.0, filt="A"):
    adc, dac = converters(fs, filt)
    return [
        adc,
        Element("input DMA frame", frame_ms, "one frame is collected before processing"),
        Element("processing", frame_ms, "worst case: processing may use the whole frame"),
        Element("output DMA frame", frame_ms, "one frame queued for the codec"),
        dac,
    ]


def intercom_path(arch, fs=16000.0, frame_ms=1.0, bitrate=10e6, n_boxes=6, filt="A"):
    adc, dac = converters(fs, filt)
    head = [adc, Element("input DMA frame", frame_ms, "frame collection")]
    tail = [Element("output DMA frame", frame_ms, "frame queued for the codec"), dac]
    if arch == "A1":
        return [Element("analog bus", 0.0, "wire, propagation < 1 us")]
    if arch == "A2":
        # point-to-point RS-485 per box (star), TDM frame = one audio frame
        up_bits = (4 + 9 + 2 * int(fs * frame_ms / 1e3) + 2) * 10  # 8N1 bytes
        dn_bits = (4 + 9 + 4 * 2 * int(fs * frame_ms / 1e3) + 2) * 10
        return head + [
            Element("box processing", frame_ms, "frame worst case"),
            Element("uplink slot wait", frame_ms, "TDM: up to one frame"),
            Element("uplink transmission", up_bits / bitrate * 1e3, "%d bits at %.0f Mbit/s" % (up_bits, bitrate / 1e6)),
            Element("CCU mixing", frame_ms, "one frame, all boxes aligned"),
            Element("downlink transmission", dn_bits / bitrate * 1e3, "%d bits" % dn_bits),
            Element("box jitter / concealment buffer", frame_ms, "one frame held for loss concealment"),
        ] + tail
    if arch == "A3":
        return head + [
            Element("box processing + packetising", frame_ms, "frame worst case"),
            Element("network transit, bounded", 2.0, "AVB/TSN class A bound, 7 hops [R36]"),
            Element("presentation-time alignment", 0.0, "included in the 2 ms bound"),
            Element("distributed mixing in the listener's box", frame_ms, "one frame"),
        ] + tail
    raise ValueError(arch)


def total(elems):
    return sum(e.ms for e in elems)


def table(elems, limit_ms):
    t = total(elems)
    return {
        "elements": [(e.name, round(e.ms, 3), e.basis) for e in elems],
        "total_ms": round(t, 3),
        "limit_ms": limit_ms,
        "margin_ms": round(limit_ms - t, 3),
        "margin_pct": round(100.0 * (limit_ms - t) / limit_ms, 1),
    }
