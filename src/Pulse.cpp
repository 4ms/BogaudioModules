
#include "Pulse.hpp"

#define LINEAR_MODE "linear_mode"

json_t* Pulse::dataToJson() {
	json_t* root = VCOBase::dataToJson();
	json_object_set_new(root, LINEAR_MODE, json_boolean(_linearMode));
	return root;
}

void Pulse::dataFromJson(json_t* root) {
	VCOBase::dataFromJson(root);
	json_t* l = json_object_get(root, LINEAR_MODE);
	if (l) {
		_linearMode = json_is_true(l);
	}
}

bool Pulse::active() {
	return outputs[OUT_OUTPUT].isConnected();
}

void Pulse::addChannel(int c) {
	VCOBase::addChannel(c);
	_engines[c]->squareActive = true;
}

void Pulse::modulate() {
	_slowMode = params[SLOW_PARAM].getValue() > 0.5f;
}

void Pulse::modulateChannel(int c) {
	VCOBase::modulateChannel(c);
	Engine& e = *_engines[c];

	float pw = params[PW_PARAM].getValue();
	if (inputs[PWM_INPUT].isConnected()) {
		float pwm = clamp(inputs[PWM_INPUT].getPolyVoltage(c) / 5.0f, -1.0f, 1.0f);
		pwm *= clamp(params[PWM_PARAM].getValue(), -1.0f, 1.0f);
		pw = clamp(pw + pwm, -1.0f, 1.0f);
	}
	pw *= 1.0f - 2.0f * e.square.minPulseWidth;
	pw *= 0.5f;
	pw += 0.5f;
	e.square.setPulseWidth(e.squarePulseWidthSL.next(pw));
}

void Pulse::processChannel(const ProcessArgs& args, int c) {
	VCOBase::processChannel(args, c);

	outputs[OUT_OUTPUT].setChannels(_channels);
	outputs[OUT_OUTPUT].setVoltage(_engines[c]->squareOut, c);
}

struct PulseWidget : ModuleWidget {
	static constexpr int hp = 3;

	PulseWidget(Pulse* module) {
		setModule(module);
		box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);

		{
			SvgPanel *panel = new SvgPanel();
			panel->box.size = box.size;
			panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Pulse.svg")));
			addChild(panel);
		}

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		// generated by svg_widgets.rb
		auto frequencyParamPosition = Vec(9.5, 27.0);
		auto slowParamPosition = Vec(31.0, 62.0);
		auto pwParamPosition = Vec(9.5, 98.5);
		auto pwmParamPosition = Vec(14.5, 154.5);

		auto pitchInputPosition = Vec(10.5, 185.0);
		auto pwmInputPosition = Vec(10.5, 220.0);
		auto syncInputPosition = Vec(10.5, 255.0);

		auto outOutputPosition = Vec(10.5, 293.0);
		// end generated by svg_widgets.rb

		addParam(createParam<Knob26>(frequencyParamPosition, module, Pulse::FREQUENCY_PARAM));
		addParam(createParam<IndicatorButtonGreen9>(slowParamPosition, module, Pulse::SLOW_PARAM));
		addParam(createParam<Knob26>(pwParamPosition, module, Pulse::PW_PARAM));
		addParam(createParam<Knob16>(pwmParamPosition, module, Pulse::PWM_PARAM));

		addInput(createInput<Port24>(pitchInputPosition, module, Pulse::PITCH_INPUT));
		addInput(createInput<Port24>(pwmInputPosition, module, Pulse::PWM_INPUT));
		addInput(createInput<Port24>(syncInputPosition, module, Pulse::SYNC_INPUT));

		addOutput(createOutput<Port24>(outOutputPosition, module, Pulse::OUT_OUTPUT));
	}

	void appendContextMenu(Menu* menu) override {
		auto m = dynamic_cast<Pulse*>(module);
		assert(m);
		menu->addChild(new MenuLabel());
		menu->addChild(new BoolOptionMenuItem("Lineary frequency mode", [m]() { return &m->_linearMode; }));
	}
};

Model* modelPulse = createModel<Pulse, PulseWidget>("Bogaudio-Pulse", "PULSE", "Compact square/pulse oscillator with PWM", "Oscillator", "Polyphonic");
