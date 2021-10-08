#pragma once

#include "bogaudio.hpp"

extern Model* modelUnison;

namespace bogaudio {

struct Unison : BGModule {
	enum ParamsIds {
		CHANNELS_PARAM,
		DETUNE_PARAM,
		NUM_PARAMS
	};

	enum InputsIds {
		DETUNE_INPUT,
		PITCH_INPUT,
		GATE_INPUT,
		NUM_INPUTS
	};

	enum OutputsIds {
		PITCH_OUTPUT,
		GATE_OUTPUT,
		NUM_OUTPUTS
	};

	const float maxDetuneCents = 50.0f;
	int _channels = 0;
	float _cents = 0.0f;

	Unison() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
		configParam(CHANNELS_PARAM, 1.0f, 16.0f, 1.0f, "Channels");
		paramQuantities[CHANNELS_PARAM]->snapEnabled = true;
		configParam(DETUNE_PARAM, 0.0f, maxDetuneCents, 0.0f, "Detune");

		configInput(DETUNE_INPUT, "DETUNE");
		configInput(PITCH_INPUT, "PITCH");
		configInput(GATE_INPUT, "GATE");

		configOutput(PITCH_OUTPUT, "PITCH");
		configOutput(GATE_OUTPUT, "GATE");
	}

	void modulate() override;
	void processAll(const ProcessArgs& args) override;
};

} // namespace bogaudio
