
#include "Mix4.hpp"

void Mix4::onSampleRateChange() {
	float sr = APP->engine->getSampleRate();
	_channel1.setSampleRate(sr);
	_channel2.setSampleRate(sr);
	_channel3.setSampleRate(sr);
	_channel4.setSampleRate(sr);
	_slewLimiter.setParams(sr, MixerChannel::levelSlewTimeMS, MixerChannel::maxDecibels - MixerChannel::minDecibels);
	_rms.setSampleRate(sr);
}

void Mix4::process(const ProcessArgs& args) {
	bool stereo = outputs[L_OUTPUT].active && outputs[R_OUTPUT].active;
	bool solo =
		params[MUTE1_PARAM].value > 1.5f ||
		params[MUTE2_PARAM].value > 1.5f ||
		params[MUTE3_PARAM].value > 1.5f ||
		params[MUTE4_PARAM].value > 1.5f;
	_channel1.next(stereo, solo);
	_channel2.next(stereo, solo);
	_channel3.next(stereo, solo);
	_channel4.next(stereo, solo);

	float level = Amplifier::minDecibels;
	if (params[MIX_MUTE_PARAM].value < 0.5f) {
		level = params[MIX_PARAM].value;
		if (inputs[MIX_CV_INPUT].active) {
			level *= clamp(inputs[MIX_CV_INPUT].value / 10.0f, 0.0f, 1.0f);
		}
		level *= MixerChannel::maxDecibels - MixerChannel::minDecibels;
		level += MixerChannel::minDecibels;
	}
	_amplifier.setLevel(_slewLimiter.next(level));

	float mono = 0.0f;
	mono += _channel1.out;
	mono += _channel2.out;
	mono += _channel3.out;
	mono += _channel4.out;
	mono = _amplifier.next(mono);
	mono = _saturator.next(mono);
	_rmsLevel = _rms.next(mono) / 5.0f;

	if (stereo) {
		float left = 0.0f;
		left += _channel1.left;
		left += _channel2.left;
		left += _channel3.left;
		left += _channel4.left;
		left = _amplifier.next(left);
		left = _saturator.next(left);
		outputs[L_OUTPUT].value = left;

		float right = 0.0f;
		right += _channel1.right;
		right += _channel2.right;
		right += _channel3.right;
		right += _channel4.right;
		right = _amplifier.next(right);
		right = _saturator.next(right);
		outputs[R_OUTPUT].value = right;
	}
	else {
		outputs[L_OUTPUT].value = outputs[R_OUTPUT].value = mono;
	}
}

struct Mix4Widget : ModuleWidget {
	static constexpr int hp = 15;

	Mix4Widget(Mix4* module) : ModuleWidget(module) {
		box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);

		{
			SVGPanel *panel = new SVGPanel();
			panel->box.size = box.size;
			panel->setBackground(SVG::load(asset::plugin(pluginInstance, "res/Mix4.svg")));
			addChild(panel);
		}

		addChild(createWidget<ScrewSilver>(Vec(15, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 0)));
		addChild(createWidget<ScrewSilver>(Vec(15, 365)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 365)));

		// generated by svg_widgets.rb
		auto level1ParamPosition = Vec(17.5, 32.0);
		auto mute1ParamPosition = Vec(17.2, 185.7);
		auto pan1ParamPosition = Vec(18.5, 223.0);
		auto level2ParamPosition = Vec(60.5, 32.0);
		auto mute2ParamPosition = Vec(60.2, 185.7);
		auto pan2ParamPosition = Vec(61.5, 223.0);
		auto level3ParamPosition = Vec(103.5, 32.0);
		auto mute3ParamPosition = Vec(103.2, 185.7);
		auto pan3ParamPosition = Vec(104.5, 223.0);
		auto level4ParamPosition = Vec(146.5, 32.0);
		auto mute4ParamPosition = Vec(146.2, 185.7);
		auto pan4ParamPosition = Vec(147.5, 223.0);
		auto mixParamPosition = Vec(189.5, 32.0);
		auto mixMuteParamPosition = Vec(189.2, 185.7);

		auto cv1InputPosition = Vec(14.5, 255.0);
		auto pan1InputPosition = Vec(14.5, 290.0);
		auto in1InputPosition = Vec(14.5, 325.0);
		auto cv2InputPosition = Vec(57.5, 255.0);
		auto pan2InputPosition = Vec(57.5, 290.0);
		auto in2InputPosition = Vec(57.5, 325.0);
		auto cv3InputPosition = Vec(100.5, 255.0);
		auto pan3InputPosition = Vec(100.5, 290.0);
		auto in3InputPosition = Vec(100.5, 325.0);
		auto cv4InputPosition = Vec(143.5, 255.0);
		auto pan4InputPosition = Vec(143.5, 290.0);
		auto in4InputPosition = Vec(143.5, 325.0);
		auto mixCvInputPosition = Vec(186.5, 252.0);

		auto lOutputPosition = Vec(186.5, 290.0);
		auto rOutputPosition = Vec(186.5, 325.0);
		// end generated by svg_widgets.rb

		addSlider(level1ParamPosition, module, Mix4::LEVEL1_PARAM, module ? &module->_channel1.rms : NULL);
		addParam(createParam<Knob16>(pan1ParamPosition, module, Mix4::PAN1_PARAM, -1.0, 1.0, 0.0));
		addParam(createParam<SoloMuteButton>(mute1ParamPosition, module, Mix4::MUTE1_PARAM, 0.0, 3.0, 0.0));
		addSlider(level2ParamPosition, module, Mix4::LEVEL2_PARAM, module ? &module->_channel2.rms : NULL);
		addParam(createParam<Knob16>(pan2ParamPosition, module, Mix4::PAN2_PARAM, -1.0, 1.0, 0.0));
		addParam(createParam<SoloMuteButton>(mute2ParamPosition, module, Mix4::MUTE2_PARAM, 0.0, 3.0, 0.0));
		addSlider(level3ParamPosition, module, Mix4::LEVEL3_PARAM, module ? &module->_channel3.rms : NULL);
		addParam(createParam<Knob16>(pan3ParamPosition, module, Mix4::PAN3_PARAM, -1.0, 1.0, 0.0));
		addParam(createParam<SoloMuteButton>(mute3ParamPosition, module, Mix4::MUTE3_PARAM, 0.0, 3.0, 0.0));
		addSlider(level4ParamPosition, module, Mix4::LEVEL4_PARAM, module ? &module->_channel4.rms : NULL);
		addParam(createParam<Knob16>(pan4ParamPosition, module, Mix4::PAN4_PARAM, -1.0, 1.0, 0.0));
		addParam(createParam<SoloMuteButton>(mute4ParamPosition, module, Mix4::MUTE4_PARAM, 0.0, 3.0, 0.0));
		addSlider(mixParamPosition, module, Mix4::MIX_PARAM, module ? &module->_rmsLevel : NULL);
		addParam(createParam<MuteButton>(mixMuteParamPosition, module, Mix4::MIX_MUTE_PARAM, 0.0, 1.0, 0.0));

		addInput(createPort<Port24>(cv1InputPosition, PortWidget::INPUT, module, Mix4::CV1_INPUT));
		addInput(createPort<Port24>(pan1InputPosition, PortWidget::INPUT, module, Mix4::PAN1_INPUT));
		addInput(createPort<Port24>(in1InputPosition, PortWidget::INPUT, module, Mix4::IN1_INPUT));
		addInput(createPort<Port24>(cv2InputPosition, PortWidget::INPUT, module, Mix4::CV2_INPUT));
		addInput(createPort<Port24>(pan2InputPosition, PortWidget::INPUT, module, Mix4::PAN2_INPUT));
		addInput(createPort<Port24>(in2InputPosition, PortWidget::INPUT, module, Mix4::IN2_INPUT));
		addInput(createPort<Port24>(cv3InputPosition, PortWidget::INPUT, module, Mix4::CV3_INPUT));
		addInput(createPort<Port24>(pan3InputPosition, PortWidget::INPUT, module, Mix4::PAN3_INPUT));
		addInput(createPort<Port24>(in3InputPosition, PortWidget::INPUT, module, Mix4::IN3_INPUT));
		addInput(createPort<Port24>(cv4InputPosition, PortWidget::INPUT, module, Mix4::CV4_INPUT));
		addInput(createPort<Port24>(pan4InputPosition, PortWidget::INPUT, module, Mix4::PAN4_INPUT));
		addInput(createPort<Port24>(in4InputPosition, PortWidget::INPUT, module, Mix4::IN4_INPUT));
		addInput(createPort<Port24>(mixCvInputPosition, PortWidget::INPUT, module, Mix4::MIX_CV_INPUT));

		addOutput(createPort<Port24>(lOutputPosition, PortWidget::OUTPUT, module, Mix4::L_OUTPUT));
		addOutput(createPort<Port24>(rOutputPosition, PortWidget::OUTPUT, module, Mix4::R_OUTPUT));
	}

	void addSlider(Vec position, Mix4* module, int id, float* rms) {
		auto slider = createParam<VUSlider151>(
			position,
			module,
			id,
			0.0,
			1.0,
			fabsf(MixerChannel::minDecibels) / (MixerChannel::maxDecibels - MixerChannel::minDecibels)
		);
		if (rms) {
			dynamic_cast<VUSlider*>(slider)->setVULevel(rms);
		}
		addParam(slider);
	}
};

Model* modelMix4 = bogaudio::createModel<Mix4, Mix4Widget>("Bogaudio-Mix4", "Mix4",  "4-channel mixer and panner", MIXER_TAG, PANNING_TAG);
