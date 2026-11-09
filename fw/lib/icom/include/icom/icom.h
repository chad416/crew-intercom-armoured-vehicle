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
#define ICOM_CMD_BYPASS_TEST 5U /* emergency-path self-test, MAINT only (CB-50) */

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
#define ICOM_BIT_BYPASS 0x08U  /* non-critical: emergency path failed its self-test */
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

/* ---- Emergency-path self-test (CB-50..CB-53) --------------------------------
 * In MAINT the box releases its relay, plays a 1 kHz tone into the emergency
 * line through the relay's rest contact and reads it back on the loopback ADC
 * input (aux). A stuck relay or a dead emergency amplifier gives no loopback.
 */
#define ICOM_BT_TICKS 300U   /* test length, ms */
#define ICOM_BT_SETTLE 50U   /* relay release and settling, ms */
#define ICOM_BT_NONE 0U
#define ICOM_BT_PASS 1U
#define ICOM_BT_FAIL 2U

typedef struct {
	bool running;
	uint32_t t;
	float phase;
	float acc;
	uint32_t n;
	uint8_t result;
} icom_bt_t;

void icom_bt_init(icom_bt_t *t);
void icom_bt_start(icom_bt_t *t);
void icom_bt_abort(icom_bt_t *t);
/* One frame while running: writes the test tone into out, accumulates the
 * loopback level from aux, and sets result at the end. */
void icom_bt_frame(icom_bt_t *t, const int16_t aux[ICOM_N], int16_t out[ICOM_N]);

/* ---- Central communication unit core (CCU-01..CCU-08) ------------------------ */

#define ICOM_MAX_BOX 8U
#define ICOM_PTT_ICOM 0x01U
#define ICOM_PTT_R1 0x02U
#define ICOM_PTT_R2 0x04U
#define ICOM_CCU_T_LOST_MS 20U   /* box declared lost after 20 ms without uplink */

typedef struct {
	int16_t rx[2][ICOM_N];       /* radio 1, radio 2 receive audio */
	uint8_t rx_on;               /* bit 0: radio 1 squelch open, bit 1: radio 2 */
	int16_t alarm[ICOM_N];       /* vehicle warning tone */
	bool alarm_on;
	uint16_t speed_ckmh;
} icom_ccu_in_t;

typedef struct {
	icom_down_t down[ICOM_MAX_BOX];
	int16_t tx[2][ICOM_N];       /* radio 1, radio 2 transmit audio */
	uint8_t tx_key;              /* bit 0: key radio 1, bit 1: key radio 2 */
	int8_t tx_owner[2];          /* box keying each radio, -1 none, -2 rebroadcast */
	uint32_t lost;               /* number of boxes declared lost */
} icom_ccu_out_t;

typedef struct {
	uint32_t n_box;
	uint8_t commander;
	uint8_t mode;
	uint8_t seq;
	bool have_up[ICOM_MAX_BOX];
	icom_up_t up[ICOM_MAX_BOX];
	uint32_t missed[ICOM_MAX_BOX];
	bool lost[ICOM_MAX_BOX];
	uint8_t box_state[ICOM_MAX_BOX];
	uint8_t cmd[ICOM_MAX_BOX];
	int16_t arg[ICOM_MAX_BOX];
} icom_ccu_t;

void icom_ccu_init(icom_ccu_t *c, uint32_t n_box, uint8_t commander);
/* Store an uplink received this tick (ignored if the box ID is out of range). */
void icom_ccu_uplink(icom_ccu_t *c, const icom_up_t *u);
/* Queue a one-shot command for a box (sent in its next downlink). */
void icom_ccu_command(icom_ccu_t *c, uint8_t box, uint8_t cmd, int16_t arg);
/* One 1 ms tick: supervision, mix-minus intercom, radio routing and rebroadcast. */
void icom_ccu_tick(icom_ccu_t *c, const icom_ccu_in_t *in, icom_ccu_out_t *out);

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
	uint8_t ptt;            /* push-to-talk inputs ICOM_PTT_* (set by the application) */
	icom_bt_t bt;           /* emergency-path self-test */
	bool bypass_fault;      /* last self-test failed */
	uint8_t bits;
	uint8_t tele_rr;
	uint16_t speed_cckmh;
	uint32_t max_cycles;
} icom_box_t;

void icom_box_init(icom_box_t *b, uint8_t box_id);
/* Feed received bus bytes (any number, any time). */
void icom_box_rx(icom_box_t *b, const uint8_t *p, uint32_t n);
/* One 1 ms tick: mic and err are this frame's codec inputs (int16), out the
 * earphone frame; aux is the loopback ADC input (emergency line). bits: local BIT
 * results. Returns the uplink frame length
 * written to upbuf: 0 in INIT; a telemetry-only frame (no microphone audio)
 * in BYPASS, so that the CCU still sees the box state. */
uint32_t icom_box_tick(icom_box_t *b, const int16_t mic[ICOM_N], const int16_t err[ICOM_N],
		       const int16_t aux[ICOM_N], uint8_t bits, int16_t out[ICOM_N], uint8_t *upbuf, uint32_t cap);
/* Relay drive for this tick: the mode machine's toggling drive, held inactive
 * while the emergency-path self-test runs. */
bool icom_box_relay_drive(icom_box_t *b);

#endif /* ICOM_ICOM_H */
