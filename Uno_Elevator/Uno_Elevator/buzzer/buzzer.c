/*
 * buzzer.c
 *
 * Created: 4/11/2026 1:48:34 AM
 *  Author: Hamidur
 */

#include "buzzer.h"

#include <avr/io.h>

#include "pins.h"
#include "delay.h"
#include "gpio.h"

#define BUZZER_TIMER0_PRESCALER  256UL
#define BUZZER_MELODY_GAP_MS     40U

/* Fur Elise — verse 1 + verse 2 */
/* Verse 1: E5  Eb5  E5  Eb5  E5   B4   D5   C5   A4  */
/* Verse 2: A3  C4   E4  A4   B4   E3   G#3  B3   E4  */
static const uint16_t g_melody_hz_moving[] = {
	659U, 622U, 659U, 622U, 659U, 494U, 587U, 523U, 440U,
	220U, 262U, 330U, 440U, 494U, 165U, 208U, 247U, 330U
};
static const uint16_t g_melody_dur_moving[] = {
	250U, 250U, 250U, 250U, 250U, 250U, 250U, 250U, 500U,
	250U, 250U, 250U, 500U, 500U, 250U, 250U, 250U, 500U
};
#define MELODY_MOVING_COUNT  18U

/* Never Gonna Give You Up — chorus (measures 18-21 from Robson Couto score)  */
/* tempo=114: 16th=132ms 8th=263ms d.8th=395ms quarter=526ms d.quarter=789ms */
/* half=1053ms  REST=0Hz                                                      */
static const uint16_t g_melody_hz_obstacle[] = {
	/* m18a: FS5,-8 FS5,-8 E5,-4 | A4,16 B4,16 D5,16 B4,16  (never gonna give you up) */
	740U, 740U, 659U, 440U, 494U, 587U, 494U,
	/* m18b: E5,-8 E5,-8 D5,-8 CS5,16 B4,-8 | A4,16 B4,16 D5,16 B4,16  (never gonna let you down) */
	659U, 659U, 587U, 554U, 494U, 440U, 494U, 587U, 494U,
	/* m19a: D5,4 E5,8 CS5,-8 B4,16 A4,8 A4,8 A4,8  (never gonna make you cry) */
	587U, 659U, 554U, 494U, 440U, 440U, 440U,
	/* m19b: E5,4 D5,2 | A4,16 B4,16 D5,16 B4,16  (never gonna say goodbye) */
	659U, 587U, 440U, 494U, 587U, 494U,
	/* m20a: FS5,-8 FS5,-8 E5,-4 | A4,16 B4,16 D5,16 B4,16  (never gonna tell a lie and) */
	740U, 740U, 659U, 440U, 494U, 587U, 494U,
	/* m20b: A5,4 CS5,8 D5,-8 CS5,16 B4,8 | A4,16 B4,16 D5,16 B4,16  (hurt you) */
	880U, 554U, 587U, 554U, 494U, 440U, 494U, 587U, 494U,
	/* m21a: D5,4 E5,8 CS5,-8 B4,16 A4,4 A4,8 */
	587U, 659U, 554U, 494U, 440U, 440U, 440U,
	/* m21b: E5,4 D5,2 REST,4 */
	659U, 587U, 0U
};
static const uint16_t g_melody_dur_obstacle[] = {
	/* m18a */
	395U, 395U, 789U, 132U, 132U, 132U, 132U,
	/* m18b */
	395U, 395U, 395U, 132U, 395U, 132U, 132U, 132U, 132U,
	/* m19a */
	526U, 263U, 395U, 132U, 263U, 263U, 263U,
	/* m19b */
	526U, 1053U, 132U, 132U, 132U, 132U,
	/* m20a */
	395U, 395U, 789U, 132U, 132U, 132U, 132U,
	/* m20b */
	526U, 263U, 395U, 132U, 263U, 132U, 132U, 132U, 132U,
	/* m21a */
	526U, 263U, 395U, 132U, 526U, 263U, 263U,
	/* m21b */
	526U, 1053U, 526U
};
#define MELODY_OBSTACLE_COUNT  52U

/* Nokia ringtone */
/* E5   D5   F#4  G#4  C#5  B4   D4   E4   B4   A4   */
static const uint16_t g_melody_hz_door[] = {
	659U, 587U, 370U, 415U, 554U, 494U, 294U, 330U, 494U, 440U
};
static const uint16_t g_melody_dur_door[] = {
	175U, 175U, 350U, 350U, 175U, 175U, 350U, 350U, 175U, 725U
};
#define MELODY_DOOR_COUNT  10U

static const uint16_t *g_current_melody_hz    = g_melody_hz_moving;
static const uint16_t *g_current_melody_dur   = g_melody_dur_moving;
static uint8_t         g_current_melody_count = MELODY_MOVING_COUNT;

static uint8_t  g_melody_loop_enabled      = 0U;
static uint8_t  g_melody_phase_is_gap      = 0U;
static uint8_t  g_melody_note_index        = 0U;
static uint16_t g_melody_phase_remaining_ms = 0U;

static void buzzer_hw_tone_stop(void)
{
	TCCR0A = 0U;
	TCCR0B = 0U;
	TCNT0 = 0U;
	OCR0A = 0U;
	pin_mode(BUZZER, OUTPUT);
	digital_write(BUZZER, LOW);
}

static void buzzer_hw_tone_start(uint16_t hz)
{
	uint32_t compare_value;

	if (hz == 0U)
	{
		buzzer_hw_tone_stop();
		return;
	}

	compare_value = (F_CPU / (2UL * BUZZER_TIMER0_PRESCALER * (uint32_t)hz));

	if (compare_value == 0UL)
	{
		compare_value = 1UL;
	}

	if (compare_value > 255UL)
	{
		compare_value = 255UL;
	}

	OCR0A = (uint8_t)(compare_value - 1UL);
	TCNT0 = 0U;

	/* Timer0 CTC mode, toggle OC0A (D6) on compare match. */
	TCCR0A = (1U << COM0A0) | (1U << WGM01);
	TCCR0B = (1U << CS02);
}

static void buzzer_melody_start_note(uint8_t note_index)
{
	buzzer_hw_tone_start(g_current_melody_hz[note_index]);
	g_melody_phase_is_gap = 0U;
	g_melody_phase_remaining_ms = g_current_melody_dur[note_index];
}

static void buzzer_melody_start_gap(void)
{
	buzzer_hw_tone_stop();
	g_melody_phase_is_gap = 1U;
	g_melody_phase_remaining_ms = BUZZER_MELODY_GAP_MS;
}

static void buzzer_delay_us(uint16_t duration_us)
{
	while (duration_us > 0U)
	{
		DELAY_us(1);
		duration_us--;
	}
}

static void buzzer_play_tone_for_ms(uint16_t hz, uint16_t duration_ms)
{
	uint16_t half_period_us;
	uint32_t total_cycles;
	uint32_t cycle;

	if (hz == 0U)
	{
		buzzer_stop();
		return;
	}

	half_period_us = (uint16_t)(500000UL / hz);
	if (half_period_us == 0U)
	{
		half_period_us = 1U;
	}

	total_cycles = ((uint32_t)duration_ms * 1000UL) / (uint32_t)(2U * half_period_us);

	for (cycle = 0U; cycle < total_cycles; cycle++)
	{
		digital_write(BUZZER, HIGH);
		buzzer_delay_us(half_period_us);
		digital_write(BUZZER, LOW);
		buzzer_delay_us(half_period_us);
	}
}

void buzzer_init(void)
{
	buzzer_hw_tone_stop();
	g_melody_loop_enabled = 0U;
	g_melody_phase_is_gap = 0U;
	g_melody_note_index = 0U;
	g_melody_phase_remaining_ms = 0U;

	pin_mode(BUZZER, OUTPUT);
	digital_write(BUZZER, LOW);
}

void buzzer_tone(uint16_t hz)
{
	buzzer_set_melody_loop(0U);
	buzzer_init();
	buzzer_play_tone_for_ms(hz, 250U);
}

void buzzer_start_melody(void)
{
	uint8_t index;

	buzzer_set_melody_loop(0U);
	buzzer_init();

	for (index = 0U; index < g_current_melody_count; index++)
	{
		buzzer_play_tone_for_ms(g_current_melody_hz[index], g_current_melody_dur[index]);
		DELAY_ms(BUZZER_MELODY_GAP_MS);
	}

	buzzer_stop();
}

void buzzer_stop(void)
{
	buzzer_set_melody_loop(0U);
}

void buzzer_select_melody(uint8_t melody_id)
{
	buzzer_hw_tone_stop();
	g_melody_loop_enabled       = 0U;
	g_melody_phase_is_gap       = 0U;
	g_melody_note_index         = 0U;
	g_melody_phase_remaining_ms = 0U;

	if (melody_id == BUZZER_MELODY_DOOR)
	{
		g_current_melody_hz    = g_melody_hz_door;
		g_current_melody_dur   = g_melody_dur_door;
		g_current_melody_count = MELODY_DOOR_COUNT;
	}
	else if (melody_id == BUZZER_MELODY_OBSTACLE)
	{
		g_current_melody_hz    = g_melody_hz_obstacle;
		g_current_melody_dur   = g_melody_dur_obstacle;
		g_current_melody_count = MELODY_OBSTACLE_COUNT;
	}
	else
	{
		g_current_melody_hz    = g_melody_hz_moving;
		g_current_melody_dur   = g_melody_dur_moving;
		g_current_melody_count = MELODY_MOVING_COUNT;
	}
}

void buzzer_set_melody_loop(uint8_t enabled)
{
	uint8_t loop_enable = (enabled != 0U) ? 1U : 0U;

	if (loop_enable == g_melody_loop_enabled)
	{
		return;
	}

	g_melody_loop_enabled = loop_enable;

	if (g_melody_loop_enabled == 0U)
	{
		g_melody_phase_is_gap = 0U;
		g_melody_note_index = 0U;
		g_melody_phase_remaining_ms = 0U;
		buzzer_hw_tone_stop();
		return;
	}

	g_melody_phase_is_gap = 0U;
	g_melody_note_index = 0U;
	buzzer_melody_start_note(g_melody_note_index);
}

void buzzer_task(void)
{
	if ((g_melody_loop_enabled == 0U) || (g_melody_phase_remaining_ms == 0U))
	{
		return;
	}

	g_melody_phase_remaining_ms--;

	if (g_melody_phase_remaining_ms != 0U)
	{
		return;
	}

	if (g_melody_phase_is_gap != 0U)
	{
		g_melody_note_index++;

		if (g_melody_note_index >= g_current_melody_count)
		{
			g_melody_note_index = 0U;
		}

		buzzer_melody_start_note(g_melody_note_index);
	}
	else
	{
		buzzer_melody_start_gap();
	}
}
