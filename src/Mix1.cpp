
#include "Mix1.hpp"

void Mix1::onSampleRateChange() {
	_channel.setSampleRate(APP->engine->getSampleRate());
}

void Mix1::process(const ProcessArgs& args) {
	_channel.next(false, false);
	outputs[OUT_OUTPUT].value = _channel.out;
}

struct Mix1Widget : ModuleWidget {
	static constexpr int hp = 3;

	Mix1Widget(Mix1* module) : ModuleWidget(module) {
		box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);

		{
			SVGPanel *panel = new SVGPanel();
			panel->box.size = box.size;
			panel->setBackground(SVG::load(asset::plugin(pluginInstance, "res/Mix1.svg")));
			addChild(panel);
		}

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		// generated by svg_widgets.rb
		auto levelParamPosition = Vec(13.5, 18.0);
		auto muteParamPosition = Vec(13.2, 175.7);

		auto muteInputPosition = Vec(10.5, 198.0);
		auto levelInputPosition = Vec(10.5, 233.0);
		auto inInputPosition = Vec(10.5, 268.0);

		auto outOutputPosition = Vec(10.5, 306.0);
		// end generated by svg_widgets.rb

		{
			auto slider = createParam<VUSlider151>(
				levelParamPosition,
				module,
				Mix1::LEVEL_PARAM,
				0.0,
				1.0,
				fabsf(MixerChannel::minDecibels) / (MixerChannel::maxDecibels - MixerChannel::minDecibels)
			);
			if (module) {
				dynamic_cast<VUSlider*>(slider)->setVULevel(&module->_channel.rms);
			}
			addParam(slider);
		}
		addParam(createParam<MuteButton>(muteParamPosition, module, Mix1::MUTE_PARAM, 0.0, 1.0, 0.0));

		addInput(createPort<Port24>(muteInputPosition, PortWidget::INPUT, module, Mix1::MUTE_INPUT));
		addInput(createPort<Port24>(levelInputPosition, PortWidget::INPUT, module, Mix1::LEVEL_INPUT));
		addInput(createPort<Port24>(inInputPosition, PortWidget::INPUT, module, Mix1::IN_INPUT));

		addOutput(createPort<Port24>(outOutputPosition, PortWidget::OUTPUT, module, Mix1::OUT_OUTPUT));
	}
};

Model* modelMix1 = bogaudio::createModel<Mix1, Mix1Widget>("Bogaudio-Mix1", "MIX1", "fader/amplifier with mute", AMPLIFIER_TAG);
