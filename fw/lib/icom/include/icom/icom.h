/*
 * Crew-box core library: bus framing, link supervision with loss concealment,
 * operating-mode state machine with fail-safe bypass, priority mixer with
 * local sidetone and output limiter, speed-referenced harmonic ANR.
 *
 * Pure C99, no heap, no OS calls; all state lives in caller-owned structs.
 * Requirement IDs (CB-xx) refer to fw/SRS.md.
 * SPDX-License-Identifier: MIT
 */
#ifndef ICOM_ICOM_H
#define ICOM_ICOM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* ---- Audio frame ------------------------------------------------------- */

#define ICOM_FS_HZ 16000U   /* sample rate (wideband speech, CB-01) */
#define ICOM_N 16U          /* samples per 1 ms frame */
#define ICOM_NCH 4U         /* downlink channels */
#define ICOM_CH_ALARM 0U    /* vehicle warning tones: highest priority */
#define ICOM_CH_RADIO1 1U
#define ICOM_CH_RADIO2 2U
#define ICOM_CH_ICOM 3U     /* intercom mix from the CCU */

/* One frame of every downlink channel. */
typedef struct {
	int16_t s[ICOM_NCH][ICOM_N];
} icom_chans_t;

/* ---- Bus frames (CB-10..CB-13) ------------------------------------------- */

#define ICOM_SYNC0 0xA5U
#define ICOM_SYNC1 0x5AU
#define ICOM_T_DOWN 0x01U
#define ICOM_T_UP 0x81U
#define ICOM_HDR_LEN 4U      /* sync0 sync1 len type */
#define ICOM_CRC_LEN 2U
#define ICOM_DOWN_FIX 9U     /* seq mode flags speed(2) cmd arg(2) nch */
#define ICOM_UP_FIX 9U       /* seq box state bits ptt tele_id tele(2) nch */
#define ICOM_MAX_PAYLOAD (ICOM_DOWN_FIX + (ICOM_NCH * ICOM_N * 2U))
#define ICOM_MAX_FRAME (ICOM_HDR_LEN + ICOM_MAX_PAYLOAD + ICOM_CRC_LEN)

/* CCU system modes carried in the downlink */
#define ICOM_CCU_NORMAL 0U
#define ICOM_CCU_REBROADCAST 1U
#define ICOM_CCU_MAINT 2U

/* downlink commands */
#define ICOM_CMD_NONE 0U
#define ICOM_CMD_MAINT_ENTER 1U
#define ICOM_CMD_MAINT_EXIT 2U
#define ICOM_CMD_SHAT_ROT 3U  /* rotate S_hat by arg (deg, signed), MAINT only */
#define ICOM_CMD_ANR_RESET 4U

typedef struct {
	uint8_t seq;
	uint8_t ccu_mode;
	uint8_t flags;       /* bit i: channel i carries audio */
	uint16_t speed_ckmh; /* vehicle speed, 0.01 km/h */
	uint8_t cmd;
	int16_t arg;
	uint8_t nch;         /* channels in this frame, 0..ICOM_NCH */
	int16_t pcm[ICOM_NCH][ICOM_N];
} icom_down_t;

typedef struct {
	uint8_t seq;
	uint8_t box_id;
	uint8_t state;
	uint8_t bits;        /* BIT result bits ICOM_BIT_* */
	uint8_t ptt;
	uint8_t tele_id;
	uint16_t tele;
	uint8_t nch;         /* 0 or 1 (microphone) */
	int16_t pcm[ICOM_N];
} icom_up_t;

uint16_t icom_crc16(const uint8_t *p, uint32_t n);
uint32_t icom_down_encode(const icom_down_t *d, uint8_t *buf, uint32_t cap);
uint32_t icom_up_encode(const icom_up_t *u, uint8_t *buf, uint32_t cap);

typedef enum { ICOM_RX_NONE = 0, ICOM_RX_DOWN, ICOM_RX_UP } icom_rx_kind_t;

typedef struct {
	uint8_t buf[ICOM_MAX_FRAME];
	uint32_t pos;
	uint32_t need;
	uint32_t crc_errors;
	uint32_t format_errors;
	uint32_t frames;
} icom_parser_t;

void icom_parser_init(icom_parser_t *p);
/* Feed one byte. Returns the kind of frame completed by this byte (decoded
 * into *down or *up), or ICOM_RX_NONE. */
icom_rx_kind_t icom_parser_feed(icom_parser_t *p, uint8_t byte, icom_down_t *down, icom_up_t *up);

/* ---- Link supervision and loss concealment (CB-14..CB-16) ---------------- */

#define ICOM_PLC_FRAMES 3U    /* frames repeated with fade before muting */

typedef struct {
	bool have_last;
	uint8_t last_seq;
	uint32_t missed_run;  /* consecutive ticks without a valid frame */
	uint32_t good_run;    /* consecutive ticks with a valid frame */
	uint32_t lost_total;  /* frames lost: gaps in the sequence numbers */
	uint32_t concealed;
	int16_t last_pcm[ICOM_NCH][ICOM_N];
	uint8_t last_flags;
} icom_link_t;

void icom_link_init(icom_link_t *l);
/* Called once per 1 ms tick. frame = the valid downlink received in this tick
 * or NULL. Writes the audio to use this tick into pcm (concealed or muted when
 * the frame is missing) and returns the channel-active flags. */
uint8_t icom_link_tick(icom_link_t *l, const icom_down_t *frame, icom_chans_t *pcm);

/* ---- Modes and states (CB-20..CB-29) -------------------------------------- */

typedef enum {
	ICOM_ST_INIT = 0,
	ICOM_ST_NORMAL = 1,
	ICOM_ST_DEGRADED = 2,
	ICOM_ST_BYPASS = 3,
	ICOM_ST_MAINT = 4
} icom_state_t;

#define ICOM_BIT_CODEC 0x01U   /* critical: codec / audio path */
#define ICOM_BIT_SUPPLY 0x02U  /* critical: local supply out of range */
#define ICOM_BIT_ANR 0x04U     /* non-critical: ANR disabled by its guard */
#define ICOM_BIT_CRITICAL (ICOM_BIT_CODEC | ICOM_BIT_SUPPLY)

#define ICOM_T_INIT_MS 2000U     /* link must come up within this after POST */
#define ICOM_T_LOSS_MS 20U       /* link loss -> bypass */
#define ICOM_T_RECOVER_MS 2000U  /* continuous good link to leave bypass */
#define ICOM_T_MAINT_MS 600000U  /* maintenance timeout */

typedef enum {
	ICOM_WHY_NONE = 0,
	ICOM_WHY_LINK = 1,
	ICOM_WHY_CRITICAL = 2,
	ICOM_WHY_NO_LINK_AT_INIT = 3
} icom_why_t;

typedef struct {
	icom_state_t state;
	icom_why_t why;
	uint32_t t_in_state;
	bool latched;          /* bypass for an internal critical fault: no recovery */
	bool relay_phase;      /* dynamic relay drive: toggles while digital path is on */
	uint32_t transitions;
} icom_modes_t;

typedef struct {
	bool link_ok;          /* a valid frame this tick */
	uint32_t missed_run;
	uint32_t good_run;
	uint8_t bits;
	uint8_t cmd;
	uint16_t speed_ckmh;
} icom_mode_in_t;

void icom_modes_init(icom_modes_t *m);
icom_state_t icom_modes_tick(icom_modes_t *m, const icom_mode_in_t *in);
/* Relay drive output for this tick: toggles in NORMAL/DEGRADED/MAINT (the
 * relay's charge pump keeps it energised only while it toggles); constant low
 * in INIT and BYPASS, so a stalled CPU drops the relay to the bypass path. */
bool icom_modes_relay_drive(icom_modes_t *m);

/* ---- Mixer (CB-30..CB-35) ------------------------------------------------- */

typedef struct {
	float vol[ICOM_NCH];   /* user volume per channel, linear */
	float sidetone;        /* local sidetone gain, linear */
	float duck;            /* gain applied to lower-priority channels */
	float limit;           /* output peak limit, full scale = 1.0 */
	float env;             /* limiter gain state */
	uint32_t hold[ICOM_NCH];
	float act_thr;         /* activity threshold (frame mean square) */
	uint32_t hold_ms;
} icom_mix_t;

void icom_mix_init(icom_mix_t *x);
/* Mixes one frame. pcm: downlink channels; flags: active-channel bits from
 * the link; mic: local microphone (for sidetone); anti: anti-noise to add
 * (float, full scale 1.0) or NULL. Writes out (int16). Returns a bitmask of
 * channels considered active this frame. */
uint8_t icom_mix_frame(icom_mix_t *x, const icom_chans_t *pcm, uint8_t flags,
		       const int16_t mic[ICOM_N], const float anti[ICOM_N], int16_t out[ICOM_N]);

/* ---- Speed-referenced harmonic ANR (CB-40..CB-47) ------------------------- */

#define ICOM_ANR_HMAX 4U
#define ICOM_SH_N 33U          /* S_hat table: 0..1000 Hz in 31.25 Hz steps */
#define ICOM_SH_STEP_HZ 31.25f

typedef struct {
	/* parameters */
	float pitch_m;
	uint32_t n_harm;
	float mu;
	float f_min_hz;
	float f_max_hz;
	float v_min_kmh;
	float y_max;           /* anti-noise clamp, full scale */
	float w_max;           /* weight magnitude guard */
	float sh_re[ICOM_SH_N];
	float sh_im[ICOM_SH_N];
	/* state */
	float phase;           /* radians, wrapped to [0, 2pi) */
	float w[ICOM_ANR_HMAX][2];
	float ms_e;            /* running mean square of the error */
	float ms_ref;          /* error mean square when adaptation started */
	uint32_t t_on;         /* samples since enable */
	uint32_t t_bad;        /* samples with the error above the guard */
	uint32_t t_obs;        /* samples observed with the ANR off (warm-up) */
	bool enabled;
	bool fault;
	float ms_d_est;        /* last ms_e measured with ANR off (for telemetry) */
} icom_anr_t;

void icom_anr_init(icom_anr_t *a, float pitch_m, uint32_t n_harm, float mu);
/* Fill S_hat from a pure delay + gain model (commissioning default). */
void icom_anr_set_path(icom_anr_t *a, float gain, float delay_s);
/* Rotate every S_hat entry by deg (maintenance / fault-injection test). */
void icom_anr_rotate(icom_anr_t *a, float deg);
void icom_anr_reset(icom_anr_t *a);
/* Process one frame: err = error-microphone samples (full scale 1.0),
 * speed in km/h; writes the anti-noise for the next output frame. */
void icom_anr_frame(icom_anr_t *a, const float err[ICOM_N], float speed_kmh, float anti[ICOM_N]);
/* Attenuation estimate in dB*10 (ms_d_est / ms_e), for telemetry. */
uint16_t icom_anr_atten_x10(const icom_anr_t *a);

/* ---- Crew box (top level) -------------------------------------------------- */

typedef struct {
	uint8_t box_id;
	uint8_t seq;
	icom_parser_t parser;
	icom_link_t link;
	icom_modes_t modes;
	icom_mix_t mix;
	icom_anr_t anr;
	icom_down_t rx;
	icom_up_t up;
	icom_up_t rx_up;        /* uplink frames seen on the bus (ignored) */
	bool rx_valid;
	bool up_audio;          /* send microphone audio in the uplink (default true) */
	uint8_t bits;
	uint8_t tele_rr;
	uint16_t speed_cckmh;
	uint32_t max_cycles;
} icom_box_t;

void icom_box_init(icom_box_t *b, uint8_t box_id);
/* Feed received bus bytes (any number, any time). */
void icom_box_rx(icom_box_t *b, const uint8_t *p, uint32_t n);
/* One 1 ms tick: mic and err are this frame's codec inputs (int16), out the
 * earphone frame. bits: local BIT results. Returns the uplink frame length
 * written to upbuf: 0 in INIT; a telemetry-only frame (no microphone audio)
 * in BYPASS, so that the CCU still sees the box state. */
uint32_t icom_box_tick(icom_box_t *b, const int16_t mic[ICOM_N], const int16_t err[ICOM_N], uint8_t bits,
		       int16_t out[ICOM_N], uint8_t *upbuf, uint32_t cap);

#endif /* ICOM_ICOM_H */
