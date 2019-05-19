#pragma once

#include "bogaudio.hpp"
#include "dsp/signal.hpp"

using namespace bogaudio::dsp;

extern Model* modelVU;

namespace bogaudio {

struct VU : Module {
	enum ParamsIds {
		NUM_PARAMS
	};

	enum InputsIds {
		L_INPUT,
		R_INPUT,
		NUM_INPUTS
	};

	enum OutputsIds {
		L_OUTPUT,
		R_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightsIds {
		NUM_LIGHTS
	};

	RootMeanSquare _lRms;
	RootMeanSquare _rRms;
	float _lLevel = 0.0f;
	float _rLevel = 0.0f;

	VU() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		onSampleRateChange();
		_lRms.setSensitivity(0.05f);
		_rRms.setSensitivity(0.05f);
	}

	void onSampleRateChange() override;
	void process(const ProcessArgs& args) override;
};

} // namespace bogaudio
