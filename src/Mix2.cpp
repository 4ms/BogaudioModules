
#include "Mix2.hpp"

void Mix2::sampleRateChange() {
	float sr = APP->engine->getSampleRate();
	for (int c = 0; c < _channels; ++c) {
		_engines[c]->left.setSampleRate(sr);
		_engines[c]->right.setSampleRate(sr);
	}
}

bool Mix2::active() {
	return outputs[L_OUTPUT].isConnected() || outputs[R_OUTPUT].isConnected();
}

int Mix2::channels() {
	return inputs[L_INPUT].getChannels();
}

void Mix2::addChannel(int c) {
	_engines[c] = new Engine(
		params[LEVEL_PARAM],
		params[MUTE_PARAM],
		inputs[LEVEL_INPUT],
		inputs[MUTE_INPUT]
	);
}

void Mix2::removeChannel(int c) {
	delete _engines[c];
	_engines[c] = NULL;
}

void Mix2::processAlways(const ProcessArgs& args) {
	_leftRmsSum = 0.0f;
	_rightRmsSum = 0.0f;
}

void Mix2::processChannel(const ProcessArgs& args, int c) {
	Engine& e = *_engines[c];

	float left = inputs[L_INPUT].getVoltage(c);
	e.left.next(left, false, c, _linearCV);
	_leftRmsSum += e.left.rms;
	outputs[L_OUTPUT].setChannels(_channels);
	outputs[L_OUTPUT].setVoltage(e.left.out, c);

	float right = left;
	if (inputs[R_INPUT].isConnected()) {
		right = inputs[R_INPUT].getVoltage(c);
	}
	e.right.next(right, false, c, _linearCV);
	_rightRmsSum += e.right.rms;
	outputs[R_OUTPUT].setChannels(_channels);
	outputs[R_OUTPUT].setVoltage(e.right.out, c);
}

void Mix2::postProcessAlways(const ProcessArgs& args) {
	_leftRms = _leftRmsSum * _inverseChannels;
	_rightRms = _rightRmsSum * _inverseChannels;
}

void Mix2::processBypass(const ProcessArgs& args) {
	outputs[L_OUTPUT].setChannels(inputs[L_INPUT].getChannels());
	outputs[L_OUTPUT].writeVoltages(inputs[L_INPUT].getVoltages());
	if (inputs[R_INPUT].isConnected()) {
		outputs[R_OUTPUT].setChannels(inputs[R_INPUT].getChannels());
		outputs[R_OUTPUT].writeVoltages(inputs[R_INPUT].getVoltages());
	}
	else {
		outputs[R_OUTPUT].setChannels(inputs[L_INPUT].getChannels());
		outputs[R_OUTPUT].writeVoltages(inputs[L_INPUT].getVoltages());
	}
}

struct Mix2Widget : LinearCVMixerWidget {
	static constexpr int hp = 5;

	Mix2Widget(Mix2* module) {
		setModule(module);
		box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);
		setPanel(box.size, "Mix2");
		createScrews();

		// generated by svg_widgets.rb
		auto levelParamPosition = Vec(28.5, 32.0);
		auto muteParamPosition = Vec(28.5, 197.0);

		auto levelInputPosition = Vec(10.5, 244.0);
		auto muteInputPosition = Vec(40.5, 244.0);
		auto lInputPosition = Vec(10.5, 280.0);
		auto rInputPosition = Vec(40.5, 280.0);

		auto lOutputPosition = Vec(10.5, 320.0);
		auto rOutputPosition = Vec(40.5, 320.0);
		// end generated by svg_widgets.rb

		{
			auto slider = createParam<VUSlider151>(levelParamPosition, module, Mix2::LEVEL_PARAM);
			if (module) {
				dynamic_cast<VUSlider*>(slider)->setVULevel(&module->_leftRms);
				dynamic_cast<VUSlider*>(slider)->setStereoVULevel(&module->_rightRms);
			}
			addParam(slider);
		}
		addParam(createParam<MuteButton>(muteParamPosition, module, Mix2::MUTE_PARAM));

		addInput(createInput<Port24>(levelInputPosition, module, Mix2::LEVEL_INPUT));
		addInput(createInput<Port24>(muteInputPosition, module, Mix2::MUTE_INPUT));
		addInput(createInput<Port24>(lInputPosition, module, Mix2::L_INPUT));
		addInput(createInput<Port24>(rInputPosition, module, Mix2::R_INPUT));

		addOutput(createOutput<Port24>(lOutputPosition, module, Mix2::L_OUTPUT));
		addOutput(createOutput<Port24>(rOutputPosition, module, Mix2::R_OUTPUT));	}
};

Model* modelMix2 = createModel<Mix2, Mix2Widget>("Bogaudio-Mix2", "MIX2", "Stereo fader/amplifier with CV-controllable mute", "VCA", "Polyphonic");
