
#include "Switch.hpp"

void bogaudio::Switch::reset() {
	for (int i = 0; i < _channels; ++i) {
		_trigger[i].reset();
	}
}

bool bogaudio::Switch::active() {
	return outputs[OUT1_OUTPUT].isConnected() || outputs[OUT2_OUTPUT].isConnected();
}

int bogaudio::Switch::channels() {
	if (inputs[GATE_INPUT].isConnected()) {
		return inputs[GATE_INPUT].getChannels();
	}
	return 1;
}

void bogaudio::Switch::channelsChanged(int before, int after) {
	while (before < after) {
		_trigger[before].reset();
		++before;
	}
}

void bogaudio::Switch::modulate() {
	_latch = params[LATCH_PARAM].getValue() > 0.5f;
}

void bogaudio::Switch::always(const ProcessArgs& args) {
	lights[LATCH_LIGHT].value = _latch;
}

void bogaudio::Switch::processChannel(const ProcessArgs& args, int c) {
	bool triggered = _trigger[c].process(params[GATE_PARAM].getValue() + inputs[GATE_INPUT].getVoltage(c));
	if (_latch) {
		if (triggered) {
			_latchedHigh[c] = !_latchedHigh[c];
		}
	}
	else {
		_latchedHigh[c] = false;
	}

	if (_latchedHigh[c] || _trigger[c].isHigh()) {
		if (_channels == 1) {
			outputs[OUT1_OUTPUT].setChannels(inputs[HIGH1_INPUT].getChannels());
			outputs[OUT1_OUTPUT].writeVoltages(inputs[HIGH1_INPUT].getVoltages());

			outputs[OUT2_OUTPUT].setChannels(inputs[HIGH2_INPUT].getChannels());
			outputs[OUT2_OUTPUT].writeVoltages(inputs[HIGH2_INPUT].getVoltages());
		}
		else {
			outputs[OUT1_OUTPUT].setChannels(std::max(inputs[LOW1_INPUT].getChannels(), inputs[HIGH1_INPUT].getChannels()));
			outputs[OUT1_OUTPUT].setVoltage(inputs[HIGH1_INPUT].getVoltage(c), c);

			outputs[OUT2_OUTPUT].setChannels(std::max(inputs[LOW2_INPUT].getChannels(), inputs[HIGH2_INPUT].getChannels()));
			outputs[OUT2_OUTPUT].setVoltage(inputs[HIGH2_INPUT].getVoltage(c), c);
		}
	}
	else {
		if (_channels == 1) {
			outputs[OUT1_OUTPUT].setChannels(std::max(inputs[LOW1_INPUT].getChannels(), inputs[HIGH1_INPUT].getChannels()));
			outputs[OUT1_OUTPUT].writeVoltages(inputs[LOW1_INPUT].getVoltages());

			outputs[OUT2_OUTPUT].setChannels(std::max(inputs[LOW2_INPUT].getChannels(), inputs[HIGH2_INPUT].getChannels()));
			outputs[OUT2_OUTPUT].writeVoltages(inputs[LOW2_INPUT].getVoltages());
		}
		else {
			outputs[OUT1_OUTPUT].setChannels(_channels);
			outputs[OUT1_OUTPUT].setVoltage(inputs[LOW1_INPUT].getVoltage(c), c);

			outputs[OUT2_OUTPUT].setChannels(_channels);
			outputs[OUT2_OUTPUT].setVoltage(inputs[LOW2_INPUT].getVoltage(c), c);
		}
	}
}

struct SwitchWidget : ModuleWidget {
	static constexpr int hp = 3;

	SwitchWidget(bogaudio::Switch* module) {
		setModule(module);
		box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);

		{
			SvgPanel *panel = new SvgPanel();
			panel->box.size = box.size;
			panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Switch.svg")));
			addChild(panel);
		}

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		// generated by svg_widgets.rb
		auto gateParamPosition = Vec(13.5, 22.0);
		auto latchParamPosition = Vec(32.9, 82.9);

		auto gateInputPosition = Vec(10.5, 44.0);
		auto high1InputPosition = Vec(10.5, 100.0);
		auto low1InputPosition = Vec(10.5, 136.0);
		auto high2InputPosition = Vec(10.5, 217.0);
		auto low2InputPosition = Vec(10.5, 253.0);

		auto out1OutputPosition = Vec(10.5, 174.0);
		auto out2OutputPosition = Vec(10.5, 291.0);

		auto latchLightPosition = Vec(4.0, 84.5);
		// end generated by svg_widgets.rb

		addParam(createParam<Button18>(gateParamPosition, module, bogaudio::Switch::GATE_PARAM));
		addParam(createParam<StatefulButton9>(latchParamPosition, module, bogaudio::Switch::LATCH_PARAM));

		addInput(createInput<Port24>(gateInputPosition, module, bogaudio::Switch::GATE_INPUT));
		addInput(createInput<Port24>(high1InputPosition, module, bogaudio::Switch::HIGH1_INPUT));
		addInput(createInput<Port24>(low1InputPosition, module, bogaudio::Switch::LOW1_INPUT));
		addInput(createInput<Port24>(high2InputPosition, module, bogaudio::Switch::HIGH2_INPUT));
		addInput(createInput<Port24>(low2InputPosition, module, bogaudio::Switch::LOW2_INPUT));

		addOutput(createOutput<Port24>(out1OutputPosition, module, bogaudio::Switch::OUT1_OUTPUT));
		addOutput(createOutput<Port24>(out2OutputPosition, module, bogaudio::Switch::OUT2_OUTPUT));

		addChild(createLight<SmallLight<GreenLight>>(latchLightPosition, module, bogaudio::Switch::LATCH_LIGHT));
	}
};

Model* modelSwitch = bogaudio::createModel<bogaudio::Switch, SwitchWidget>("Bogaudio-Switch", "SWITCH", "2-way signal router", "Switch", "Polyphonic");
