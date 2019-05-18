
#include "Walk.hpp"

void Walk::onReset() {
	_jumpTrigger.reset();
	_modulationStep = modulationSteps;
}

void Walk::onSampleRateChange() {
	_modulationStep = modulationSteps;
	_slew.setParams(APP->engine->getSampleRate(), 100.0f, 10.0f);
}

void Walk::process(const ProcessArgs& args) {
	++_modulationStep;
	if (_modulationStep >= modulationSteps) {
		_modulationStep = 0;

		float rate = params[RATE_PARAM].value;
		if (inputs[RATE_INPUT].active) {
			rate *= clamp(inputs[RATE_INPUT].value / 10.0f, 0.0f, 1.0f);
		}
		rate = 0.2f * powf(rate, 5.0f);
		_walk.setParams(APP->engine->getSampleRate(), rate);

		_offset = params[OFFSET_PARAM].value;
		if (inputs[OFFSET_INPUT].active) {
			_offset *= clamp(inputs[OFFSET_INPUT].value / 5.0f, -1.0f, 1.0f);
		}
		_offset *= 5.0f;

		_scale = params[SCALE_PARAM].value;
		if (inputs[SCALE_INPUT].active) {
			_scale *= clamp(inputs[SCALE_INPUT].value / 10.0f, 0.0f, 1.0f);
		}
	}

	if (_jumpTrigger.process(inputs[JUMP_INPUT].value)) {
		_walk.jump();
	}

	float out = _slew.next(_walk.next());
	out *= _scale;
	out += _offset;
	outputs[OUT_OUTPUT].value = out;
}

struct WalkWidget : ModuleWidget {
	static constexpr int hp = 3;

	WalkWidget(Walk* module) : ModuleWidget(module) {
		box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);

		{
			SVGPanel *panel = new SVGPanel();
			panel->box.size = box.size;
			panel->setBackground(SVG::load(asset::plugin(pluginInstance, "res/Walk.svg")));
			addChild(panel);
		}

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		// generated by svg_widgets.rb
		auto rateParamPosition = Vec(8.0, 36.0);
		auto offsetParamPosition = Vec(14.5, 91.0);
		auto scaleParamPosition = Vec(14.5, 130.0);

		auto rateInputPosition = Vec(10.5, 160.0);
		auto offsetInputPosition = Vec(10.5, 195.0);
		auto scaleInputPosition = Vec(10.5, 230.0);
		auto jumpInputPosition = Vec(10.5, 265.0);

		auto outOutputPosition = Vec(10.5, 303.0);
		// end generated by svg_widgets.rb

		addParam(createParam<Knob29>(rateParamPosition, module, Walk::RATE_PARAM, 0.0, 1.0, 0.1));
		addParam(createParam<Knob16>(offsetParamPosition, module, Walk::OFFSET_PARAM, -1.0, 1.0, 0.0));
		addParam(createParam<Knob16>(scaleParamPosition, module, Walk::SCALE_PARAM, 0.0, 1.0, 1.0));

		addInput(createPort<Port24>(rateInputPosition, PortWidget::INPUT, module, Walk::RATE_INPUT));
		addInput(createPort<Port24>(offsetInputPosition, PortWidget::INPUT, module, Walk::OFFSET_INPUT));
		addInput(createPort<Port24>(scaleInputPosition, PortWidget::INPUT, module, Walk::SCALE_INPUT));
		addInput(createPort<Port24>(jumpInputPosition, PortWidget::INPUT, module, Walk::JUMP_INPUT));

		addOutput(createPort<Port24>(outOutputPosition, PortWidget::OUTPUT, module, Walk::OUT_OUTPUT));
	}
};

Model* modelWalk = bogaudio::createModel<Walk, WalkWidget>("Bogaudio-Walk", "Walk", "random-walk CV source", RANDOM_TAG);
