
#include "LFO.hpp"
#include "dsp/pitch.hpp"

void LFO::onReset() {
	_resetTrigger.reset();
	_modulationStep = modulationSteps;
	_sampleStep = _phasor._sampleRate;
}

void LFO::onSampleRateChange() {
	_phasor.setSampleRate(engineGetSampleRate());
	_modulationStep = modulationSteps;
	_sampleStep = _phasor._sampleRate;
}

void LFO::step() {
	lights[SLOW_LIGHT].value = _slowMode = params[SLOW_PARAM].value > 0.5f;
	if (!(
		outputs[SINE_OUTPUT].active ||
		outputs[TRIANGLE_OUTPUT].active ||
		outputs[RAMP_UP_OUTPUT].active ||
		outputs[RAMP_DOWN_OUTPUT].active ||
		outputs[SQUARE_OUTPUT].active
	)) {
		return;
	}

	++_modulationStep;
	if (_modulationStep >= modulationSteps) {
		_modulationStep = 0;

		float frequency = params[FREQUENCY_PARAM].value;
		if (inputs[PITCH_INPUT].active) {
			frequency += inputs[PITCH_INPUT].value;
		}
		if (_slowMode) {
			frequency -= 8.0f;
		}
		else {
			frequency -= 4.0f;
		}
		frequency = cvToFrequency(frequency);
		if (frequency > 2000.0f) {
			frequency = 2000.0f;
		}
		_phasor.setFrequency(frequency);

		float pw = params[PW_PARAM].value;
		if (inputs[PW_INPUT].active) {
			pw *= clamp(inputs[PW_INPUT].value / 5.0f, -1.0f, 1.0f);
		}
		pw = (pw + 1.0f) / 2.0f;
		pw *= 1.0f - 2.0f * _square.minPulseWidth;
		_square.setPulseWidth(pw);

		float sample = params[SAMPLE_PARAM].value;
		if (inputs[SAMPLE_INPUT].active) {
			sample *= clamp(inputs[SAMPLE_INPUT].value / 10.0f, 0.0f, 1.0f);
		}
		float maxSampleSteps = (_phasor._sampleRate / _phasor._frequency) / 4.0f;
		_sampleSteps = clamp((int)(sample * maxSampleSteps), 1, (int)maxSampleSteps);

		_offset = params[OFFSET_PARAM].value;
		if (inputs[OFFSET_INPUT].active) {
			_offset *= clamp(inputs[OFFSET_INPUT].value / 5.0f, -1.0f, 1.0f);
		}
		_offset *= 5.0f;

		_scale = params[SCALE_PARAM].value;
		if (inputs[SCALE_INPUT].active) {
			_scale *= clamp(inputs[SCALE_INPUT].value / 10.0f, 0.0f, 1.0f);
		}

		if (_resetTrigger.process(inputs[RESET_INPUT].value)) {
			_phasor.setPhase(0.0f);
		}
	}

	_phasor.advancePhase();
	bool useSample = false;
	if (_sampleSteps > 1) {
		++_sampleStep;
		if (_sampleStep >= _sampleSteps) {
			_sampleStep = 0;
		}
		else {
			useSample = true;
		}
	}
	updateOutput(_sine, useSample, false, outputs[SINE_OUTPUT], _sineSample, _sineActive);
	updateOutput(_triangle, useSample, false, outputs[TRIANGLE_OUTPUT], _triangleSample, _triangleActive);
	updateOutput(_ramp, useSample, false, outputs[RAMP_UP_OUTPUT], _rampUpSample, _rampUpActive);
	updateOutput(_ramp, useSample, true, outputs[RAMP_DOWN_OUTPUT], _rampDownSample, _rampDownActive);
	updateOutput(_square, false, false, outputs[SQUARE_OUTPUT], _squareSample, _squareActive);
}

void LFO::updateOutput(Phasor& wave, bool useSample, bool invert, Output& output, float& sample, bool& active) {
	if (output.active) {
		if (useSample && active) {
			output.value = sample;
		}
		else {
			sample = wave.nextFromPhasor(_phasor) * amplitude * _scale + (invert ? -_offset : _offset);
			if (invert) {
				sample = -sample;
			}
			output.value = sample;
		}
		active = true;
	}
	else {
		active = false;
	}
}

struct LFOWidget : ModuleWidget {
	LFOWidget(LFO* module) : ModuleWidget(module) {
		box.size = Vec(RACK_GRID_WIDTH * 10, RACK_GRID_HEIGHT);

		{
			SVGPanel *panel = new SVGPanel();
			panel->box.size = box.size;
			panel->setBackground(SVG::load(assetPlugin(plugin, "res/LFO.svg")));
			addChild(panel);
		}

		addChild(Widget::create<ScrewSilver>(Vec(0, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(0, 365)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15, 365)));

		// generated by svg_widgets.rb
		auto frequencyParamPosition = Vec(41.0, 45.0);
		auto slowParamPosition = Vec(120.0, 249.0);
		auto sampleParamPosition = Vec(37.0, 150.0);
		auto pwParamPosition = Vec(102.0, 150.0);
		auto offsetParamPosition = Vec(42.0, 196.0);
		auto scaleParamPosition = Vec(107.0, 196.0);

		auto sampleInputPosition = Vec(15.0, 230.0);
		auto pwInputPosition = Vec(47.0, 230.0);
		auto offsetInputPosition = Vec(15.0, 274.0);
		auto scaleInputPosition = Vec(47.0, 274.0);
		auto pitchInputPosition = Vec(15.0, 318.0);
		auto resetInputPosition = Vec(47.0, 318.0);

		auto rampDownOutputPosition = Vec(79.0, 230.0);
		auto rampUpOutputPosition = Vec(79.0, 274.0);
		auto squareOutputPosition = Vec(111.0, 274.0);
		auto triangleOutputPosition = Vec(79.0, 318.0);
		auto sineOutputPosition = Vec(111.0, 318.0);

		auto slowLightPosition = Vec(111.0, 240.0);
		// end generated by svg_widgets.rb

		addParam(ParamWidget::create<Knob68>(frequencyParamPosition, module, LFO::FREQUENCY_PARAM, -8.0, 5.0, 0.0));
		addParam(ParamWidget::create<StatefulButton9>(slowParamPosition, module, LFO::SLOW_PARAM, 0.0, 1.0, 0.0));
		addParam(ParamWidget::create<Knob26>(sampleParamPosition, module, LFO::SAMPLE_PARAM, 0.0, 1.0, 0.0));
		addParam(ParamWidget::create<Knob26>(pwParamPosition, module, LFO::PW_PARAM, -1.0, 1.0, 0.0));
		addParam(ParamWidget::create<Knob16>(offsetParamPosition, module, LFO::OFFSET_PARAM, -1.0, 1.0, 0.0));
		addParam(ParamWidget::create<Knob16>(scaleParamPosition, module, LFO::SCALE_PARAM, 0.0, 1.0, 1.0));

		addInput(Port::create<Port24>(sampleInputPosition, Port::INPUT, module, LFO::SAMPLE_INPUT));
		addInput(Port::create<Port24>(pwInputPosition, Port::INPUT, module, LFO::PW_INPUT));
		addInput(Port::create<Port24>(offsetInputPosition, Port::INPUT, module, LFO::OFFSET_INPUT));
		addInput(Port::create<Port24>(scaleInputPosition, Port::INPUT, module, LFO::SCALE_INPUT));
		addInput(Port::create<Port24>(pitchInputPosition, Port::INPUT, module, LFO::PITCH_INPUT));
		addInput(Port::create<Port24>(resetInputPosition, Port::INPUT, module, LFO::RESET_INPUT));

		addOutput(Port::create<Port24>(rampUpOutputPosition, Port::OUTPUT, module, LFO::RAMP_UP_OUTPUT));
		addOutput(Port::create<Port24>(rampDownOutputPosition, Port::OUTPUT, module, LFO::RAMP_DOWN_OUTPUT));
		addOutput(Port::create<Port24>(squareOutputPosition, Port::OUTPUT, module, LFO::SQUARE_OUTPUT));
		addOutput(Port::create<Port24>(triangleOutputPosition, Port::OUTPUT, module, LFO::TRIANGLE_OUTPUT));
		addOutput(Port::create<Port24>(sineOutputPosition, Port::OUTPUT, module, LFO::SINE_OUTPUT));

		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(slowLightPosition, module, LFO::SLOW_LIGHT));
	}
};

Model* modelLFO = Model::create<LFO, LFOWidget>("Bogaudio", "Bogaudio-LFO", "LFO");
