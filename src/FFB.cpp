
#include "FFB.hpp"

void FFB::Engine::sampleRateChange() {
	float sr = APP->engine->getSampleRate();
	for (int i = 0; i < 14; i++) {
		_slews[i].setParams(sr, 1.0f, 1.0f);
	}

	auto bp = [this, sr](int i, float cutoff) {
		_filters[i].setParams(
			sr,
			MultimodeFilter::BUTTERWORTH_TYPE,
			4.0f,
			MultimodeFilter::BANDPASS_MODE,
			cutoff,
			0.22f / MultimodeFilter::maxBWPitch,
			MultimodeFilter::PITCH_BANDWIDTH_MODE
		);
	};
	_filters[ 0].setParams(sr, MultimodeFilter::BUTTERWORTH_TYPE, 12.0f, MultimodeFilter::LOWPASS_MODE, 95.0f, 0.0f);
	bp(1, 125.0f);
	bp(2, 175.0f);
	bp(3, 250.0f);
	bp(4, 350.0f);
	bp(5, 500.0f);
	bp(6, 700.0f);
	bp(7, 1000.0f);
	bp(8, 1400.0f);
	bp(9, 2000.0f);
	bp(10, 2800.0f);
	bp(11, 4000.0f);
	bp(12, 5600.0f);
	_filters[13].setParams(sr, MultimodeFilter::BUTTERWORTH_TYPE, 12.0f, MultimodeFilter::HIGHPASS_MODE, 6900.0f, 0.0f);
}

void FFB::sampleRateChange() {
	for (int c = 0; c < _channels; ++c) {
		_engines[c]->sampleRateChange();
	}
}

bool FFB::active() {
	return outputs[ALL_OUTPUT].isConnected() || outputs[ODD_OUTPUT].isConnected() || outputs[EVEN_OUTPUT].isConnected();
}

int FFB::channels() {
	return inputs[IN_INPUT].getChannels();
}

void FFB::addChannel(int c) {
	_engines[c] = new Engine();
}

void FFB::removeChannel(int c) {
	delete _engines[c];
	_engines[c] = NULL;
}

void FFB::modulate() {
	for (int i = 0; i < 14; ++i) {
		_levels[i] = clamp(params[LOWPASS_PARAM + i].getValue(), 0.0f, 1.0f);
	}
}

void FFB::modulateChannel(int c) {
	Engine& e = *_engines[c];

	float cv = 1.0f;
	if (inputs[CV_INPUT].isConnected()) {
		cv = clamp(inputs[CV_INPUT].getPolyVoltage(c) / 10.0f, 0.0f, 1.0f);
		cv *= clamp(params[CV_PARAM].getValue(), 0.0f, 1.0f);
	}

	for (int i = 0; i < 14; ++i) {
		float level = e._slews[i].next(_levels[i] * cv);
		level = 1.0f - level;
		level *= Amplifier::minDecibels;
		e._amplifiers[i].setLevel(level);
	}
}

void FFB::processChannel(const ProcessArgs& args, int c) {
	Engine& e = *_engines[c];

	float in = inputs[IN_INPUT].getVoltage(c);
	float outAll = 0.0f;
	float outOdd = 0.0f;
	float outEven = 0.0f;
	for (int i = 0; i < 14; ++i) {
		float out = e._amplifiers[i].next(e._filters[i].next(in));
		outAll += out;
		outOdd += (i == 0 || i == 13 || i % 2 == 1) * out;
		outEven += (i == 0 || i == 13 || i % 2 == 0) * out;
	}

	outputs[ALL_OUTPUT].setChannels(_channels);
	outputs[ALL_OUTPUT].setVoltage(outAll, c);
	outputs[ODD_OUTPUT].setChannels(_channels);
	outputs[ODD_OUTPUT].setVoltage(outOdd, c);
	outputs[EVEN_OUTPUT].setChannels(_channels);
	outputs[EVEN_OUTPUT].setVoltage(outEven, c);
}

struct FFBWidget : ModuleWidget {
	static constexpr int hp = 8;

	FFBWidget(FFB* module) {
		setModule(module);
		box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);

		{
			SvgPanel *panel = new SvgPanel();
			panel->box.size = box.size;
			panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/FFB.svg")));
			addChild(panel);
		}

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		// generated by svg_widgets.rb
		auto band1ParamPosition = Vec(7.5, 37.5);
		auto band5ParamPosition = Vec(47.0, 37.5);
		auto band9ParamPosition = Vec(86.5, 37.5);
		auto band2ParamPosition = Vec(7.5, 88.5);
		auto band6ParamPosition = Vec(47.0, 88.5);
		auto band10ParamPosition = Vec(86.5, 88.5);
		auto band3ParamPosition = Vec(7.5, 139.5);
		auto band7ParamPosition = Vec(47.0, 139.5);
		auto band11ParamPosition = Vec(86.5, 139.5);
		auto band4ParamPosition = Vec(7.5, 190.5);
		auto band8ParamPosition = Vec(47.0, 190.5);
		auto band12ParamPosition = Vec(86.5, 190.5);
		auto lowpassParamPosition = Vec(7.5, 241.5);
		auto cvParamPosition = Vec(52.0, 246.5);
		auto highpassParamPosition = Vec(86.5, 241.5);

		auto inInputPosition = Vec(32.5, 282.0);
		auto cvInputPosition = Vec(63.5, 282.0);

		auto allOutputPosition = Vec(17.0, 324.0);
		auto oddOutputPosition = Vec(48.0, 324.0);
		auto evenOutputPosition = Vec(79.0, 324.0);
		// end generated by svg_widgets.rb

		addParam(createParam<Knob26>(band1ParamPosition, module, FFB::BAND_1_PARAM));
		addParam(createParam<Knob26>(band5ParamPosition, module, FFB::BAND_5_PARAM));
		addParam(createParam<Knob26>(band9ParamPosition, module, FFB::BAND_9_PARAM));
		addParam(createParam<Knob26>(band2ParamPosition, module, FFB::BAND_2_PARAM));
		addParam(createParam<Knob26>(band6ParamPosition, module, FFB::BAND_6_PARAM));
		addParam(createParam<Knob26>(band10ParamPosition, module, FFB::BAND_10_PARAM));
		addParam(createParam<Knob26>(band3ParamPosition, module, FFB::BAND_3_PARAM));
		addParam(createParam<Knob26>(band7ParamPosition, module, FFB::BAND_7_PARAM));
		addParam(createParam<Knob26>(band11ParamPosition, module, FFB::BAND_11_PARAM));
		addParam(createParam<Knob26>(band4ParamPosition, module, FFB::BAND_4_PARAM));
		addParam(createParam<Knob26>(band8ParamPosition, module, FFB::BAND_8_PARAM));
		addParam(createParam<Knob26>(band12ParamPosition, module, FFB::BAND_12_PARAM));
		addParam(createParam<Knob26>(lowpassParamPosition, module, FFB::LOWPASS_PARAM));
		addParam(createParam<Knob16>(cvParamPosition, module, FFB::CV_PARAM));
		addParam(createParam<Knob26>(highpassParamPosition, module, FFB::HIGHPASS_PARAM));

		addInput(createInput<Port24>(inInputPosition, module, FFB::IN_INPUT));
		addInput(createInput<Port24>(cvInputPosition, module, FFB::CV_INPUT));

		addOutput(createOutput<Port24>(allOutputPosition, module, FFB::ALL_OUTPUT));
		addOutput(createOutput<Port24>(oddOutputPosition, module, FFB::ODD_OUTPUT));
		addOutput(createOutput<Port24>(evenOutputPosition, module, FFB::EVEN_OUTPUT));
	}
};

Model* modelFFB = createModel<FFB, FFBWidget>("Bogaudio-FFB", "FFB", "Fixed filter bank", "Filter", "Polyphonic");
