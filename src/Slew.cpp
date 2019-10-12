
#include "Slew.hpp"

bool Slew::active() {
	return inputs[IN_INPUT].isConnected() && outputs[OUT_OUTPUT].isConnected();
}

int Slew::channels() {
	return inputs[IN_INPUT].getChannels();
}

void Slew::modulate() {
	_riseShape = shape(params[RISE_SHAPE_PARAM]);
	_fallShape = shape(params[FALL_SHAPE_PARAM]);
}

void Slew::modulateChannel(int c) {
	float riseTime = time(params[RISE_PARAM], inputs[RISE_INPUT], c);
	float fallTime = time(params[FALL_PARAM], inputs[FALL_INPUT], c);
	_rise[c].setParams(APP->engine->getSampleRate(), riseTime, _riseShape);
	_fall[c].setParams(APP->engine->getSampleRate(), fallTime, _fallShape);
}

void Slew::processChannel(const ProcessArgs& args, int c) {
	float sample = inputs[IN_INPUT].getPolyVoltage(c);

	outputs[OUT_OUTPUT].setChannels(_channels);
	if (sample > _last[c]) {
		if (!_rising[c]) {
			_rising[c] = true;
			_rise[c]._last = _last[c];
		}
		outputs[OUT_OUTPUT].setVoltage(_last[c] = _rise[c].next(sample), c);
	}
	else {
		if (_rising[c]) {
			_rising[c] = false;
			_fall[c]._last = _last[c];
		}
		outputs[OUT_OUTPUT].setVoltage(_last[c] = _fall[c].next(sample), c);
	}
}

float Slew::time(Param& param, Input& input, int c) {
	float time = param.getValue();
	if (input.isConnected()) {
		time *= clamp(input.getPolyVoltage(c) / 10.0f, 0.0f, 1.0f);
	}
	return time * time * 10000.0f;
}

float Slew::shape(Param& param) {
	float shape = param.getValue();
	if (shape < 0.0) {
		shape = 1.0f + shape;
		shape = _rise[0].minShape + shape * (1.0f - _rise[0].minShape);
	}
	else {
		shape += 1.0f;
	}
	return shape;
}

struct SlewWidget : ModuleWidget {
	static constexpr int hp = 3;

	SlewWidget(Slew* module) {
		setModule(module);
		box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);

		{
			SvgPanel *panel = new SvgPanel();
			panel->box.size = box.size;
			panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Slew.svg")));
			addChild(panel);
		}

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		// generated by svg_widgets.rb
		auto riseParamPosition = Vec(9.5, 34.0);
		auto riseShapeParamPosition = Vec(14.5, 76.0);
		auto fallParamPosition = Vec(9.5, 155.0);
		auto fallShapeParamPosition = Vec(14.5, 197.0);

		auto riseInputPosition = Vec(10.5, 105.0);
		auto fallInputPosition = Vec(10.5, 226.0);
		auto inInputPosition = Vec(10.5, 263.0);

		auto outOutputPosition = Vec(10.5, 301.0);
		// end generated by svg_widgets.rb

		addParam(createParam<Knob26>(riseParamPosition, module, Slew::RISE_PARAM));
		addParam(createParam<Knob16>(riseShapeParamPosition, module, Slew::RISE_SHAPE_PARAM));
		addParam(createParam<Knob26>(fallParamPosition, module, Slew::FALL_PARAM));
		addParam(createParam<Knob16>(fallShapeParamPosition, module, Slew::FALL_SHAPE_PARAM));

		addInput(createInput<Port24>(riseInputPosition, module, Slew::RISE_INPUT));
		addInput(createInput<Port24>(fallInputPosition, module, Slew::FALL_INPUT));
		addInput(createInput<Port24>(inInputPosition, module, Slew::IN_INPUT));

		addOutput(createOutput<Port24>(outOutputPosition, module, Slew::OUT_OUTPUT));
	}
};

Model* modelSlew = bogaudio::createModel<Slew, SlewWidget>("Bogaudio-Slew", "SLEW", "Slew limiter / lag generator / glide utility", "Slew limiter", "Polyphonic");
