#pragma once

#include "bogaudio.hpp"
#include "lfo_base.hpp"

using namespace bogaudio::dsp;

extern Model* modelEightFO;

namespace bogaudio {

struct EightFO : LFOBase {
	enum ParamsIds {
		FREQUENCY_PARAM,
		WAVE_PARAM,
		SAMPLE_PWM_PARAM,
		PHASE7_PARAM,
		PHASE6_PARAM,
		PHASE5_PARAM,
		PHASE4_PARAM,
		PHASE3_PARAM,
		PHASE2_PARAM,
		PHASE1_PARAM,
		PHASE0_PARAM,
		SLOW_PARAM,
		OFFSET_PARAM,
		SCALE_PARAM,
		NUM_PARAMS
	};

	enum InputsIds {
		SAMPLE_PWM_INPUT,
		PHASE7_INPUT,
		PHASE6_INPUT,
		PHASE5_INPUT,
		PHASE4_INPUT,
		PHASE3_INPUT,
		PHASE2_INPUT,
		PHASE1_INPUT,
		PHASE0_INPUT,
		PITCH_INPUT,
		RESET_INPUT,
		OFFSET_INPUT,
		SCALE_INPUT,
		NUM_INPUTS
	};

	enum OutputsIds {
		PHASE7_OUTPUT,
		PHASE6_OUTPUT,
		PHASE5_OUTPUT,
		PHASE4_OUTPUT,
		PHASE3_OUTPUT,
		PHASE2_OUTPUT,
		PHASE1_OUTPUT,
		PHASE0_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightsIds {
		SLOW_LIGHT,
		NUM_LIGHTS
	};

	enum Wave {
		NO_WAVE,
		RAMP_UP_WAVE,
		RAMP_DOWN_WAVE,
		SINE_WAVE,
		TRIANGLE_WAVE,
		SQUARE_WAVE
	};

	const int modulationSteps = 100;
	const float amplitude = 5.0f;

	int _modulationStep = 0;
	Wave _wave = NO_WAVE;
	bool _slowMode = false;
	int _sampleSteps = 1;
	int _sampleStep = 0;
	float _offset = 0.0f;
	float _scale = 0.0f;
	PositiveZeroCrossing _resetTrigger;

	Phasor _phasor;
	SineTableOscillator _sine;
	TriangleOscillator _triangle;
	SawOscillator _ramp;
	SquareOscillator _square;

	Phasor::phase_delta_t _phase7Offset = 0.0f;
	Phasor::phase_delta_t _phase6Offset = 0.0f;
	Phasor::phase_delta_t _phase5Offset = 0.0f;
	Phasor::phase_delta_t _phase4Offset = 0.0f;
	Phasor::phase_delta_t _phase3Offset = 0.0f;
	Phasor::phase_delta_t _phase2Offset = 0.0f;
	Phasor::phase_delta_t _phase1Offset = 0.0f;
	Phasor::phase_delta_t _phase0Offset = 0.0f;

	float _phase7Sample = 0.0f;
	float _phase6Sample = 0.0f;
	float _phase5Sample = 0.0f;
	float _phase4Sample = 0.0f;
	float _phase3Sample = 0.0f;
	float _phase2Sample = 0.0f;
	float _phase1Sample = 0.0f;
	float _phase0Sample = 0.0f;

	bool _phase7Active = false;
	bool _phase6Active = false;
	bool _phase5Active = false;
	bool _phase4Active = false;
	bool _phase3Active = false;
	bool _phase2Active = false;
	bool _phase1Active = false;
	bool _phase0Active = false;

	EightFO() : LFOBase(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		configParam(FREQUENCY_PARAM, -8.0, 5.0, 0.0, "frequency");
		configParam(WAVE_PARAM, 1.0, 5.0, 3.0, "wave");
		configParam(SLOW_PARAM, 0.0, 1.0, 0.0, "slow");
		configParam(SAMPLE_PWM_PARAM, -1.0, 1.0, 0.0, "sample/pwm");
		configParam(OFFSET_PARAM, -1.0, 1.0, 0.0, "offset");
		configParam(SCALE_PARAM, 0.0, 1.0, 1.0, "scale");
		configParam(PHASE7_PARAM, -1.0, 1.0, 0.0, "phase");
		configParam(PHASE6_PARAM, -1.0, 1.0, 0.0, "phase");
		configParam(PHASE5_PARAM, -1.0, 1.0, 0.0, "phase");
		configParam(PHASE4_PARAM, -1.0, 1.0, 0.0, "phase");
		configParam(PHASE3_PARAM, -1.0, 1.0, 0.0, "phase");
		configParam(PHASE2_PARAM, -1.0, 1.0, 0.0, "phase");
		configParam(PHASE1_PARAM, -1.0, 1.0, 0.0, "phase");
		configParam(PHASE0_PARAM, -1.0, 1.0, 0.0f, "phase");

		onReset();
		onSampleRateChange();
	}

	void onReset() override;
	void onSampleRateChange() override;
	void process(const ProcessArgs& args) override;
	Phasor::phase_delta_t phaseOffset(Param& p, Input& i, Phasor::phase_delta_t baseOffset);
	void updateOutput(bool useSample, Output& output, Phasor::phase_delta_t& offset, float& sample, bool& active);
};

} // namespace bogaudio
