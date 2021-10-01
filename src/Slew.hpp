#pragma once

#include "slew_common.hpp"

extern Model* modelSlew;

namespace bogaudio {

struct Slew : BGModule {
	enum ParamsIds {
		RISE_PARAM,
		RISE_SHAPE_PARAM,
		FALL_PARAM,
		FALL_SHAPE_PARAM,
		SLOW_PARAM,
		NUM_PARAMS
	};

	enum InputsIds {
		RISE_INPUT,
		FALL_INPUT,
		IN_INPUT,
		NUM_INPUTS
	};

	enum OutputsIds {
		OUT_OUTPUT,
		NUM_OUTPUTS
	};

	RiseFallShapedSlewLimiter _slew[maxChannels];
	float _timeScale = 1.0f;

	Slew() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
		configParam<EnvelopeSegmentParamQuantity>(RISE_PARAM, 0.0f, 1.0f, 0.31623f, "Rise", " s");
		configParam(RISE_SHAPE_PARAM, -1.0f, 1.0f, 0.0f, "Rise shape");
		configParam<EnvelopeSegmentParamQuantity>(FALL_PARAM, 0.0f, 1.0f, 0.31623f, "Fall", " s");
		configParam(FALL_SHAPE_PARAM, -1.0f, 1.0f, 0.0f, "Fall shape");
		configParam(SLOW_PARAM, 0.0f, 1.0f, 0.0f, "Slow mode");
		configBypass(IN_INPUT, OUT_OUTPUT);
	}

	bool active() override;
	int channels() override;
	void modulate() override;
	void modulateChannel(int c) override;
	void processChannel(const ProcessArgs& args, int c) override;
};

} // namespace bogaudio
