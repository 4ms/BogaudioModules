
#include "PolyMult.hpp"

void PolyMult::processAll(const ProcessArgs& args) {
	int cn = clamp(params[CHANNELS_PARAM].getValue(), 1.0f, 16.0f);
	if (inputs[CHANNELS_INPUT].isConnected()) {
		int channels = inputs[CHANNELS_INPUT].getChannels();
		if (channels == 1) {
			cn = clamp(roundf(inputs[CHANNELS_INPUT].getVoltage() / 10.0f * cn), 1.0f, 16.0f);
		}
		else {
			cn = channels;
		}
	}

	float out = inputs[IN_INPUT].getVoltage();
	outputs[OUT1_OUTPUT].setChannels(cn);
	outputs[OUT2_OUTPUT].setChannels(cn);
	outputs[OUT3_OUTPUT].setChannels(cn);
	outputs[OUT4_OUTPUT].setChannels(cn);
	for (int c = 0; c < cn; ++c) {
		outputs[OUT1_OUTPUT].setVoltage(out, c);
		outputs[OUT2_OUTPUT].setVoltage(out, c);
		outputs[OUT3_OUTPUT].setVoltage(out, c);
		outputs[OUT4_OUTPUT].setVoltage(out, c);
	}
}

struct PolyMultWidget : BGModuleWidget {
	static constexpr int hp = 3;

	PolyMultWidget(PolyMult* module) {
		setModule(module);
		box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);
		setPanel(box.size, "PolyMult");
		createScrews();

		// generated by svg_widgets.rb
		auto channelsParamPosition = Vec(14.5, 42.0);

		auto channelsInputPosition = Vec(10.5, 73.0);
		auto inInputPosition = Vec(10.5, 109.0);

		auto out1OutputPosition = Vec(10.5, 147.0);
		auto out2OutputPosition = Vec(10.5, 182.0);
		auto out3OutputPosition = Vec(10.5, 217.0);
		auto out4OutputPosition = Vec(10.5, 252.0);
		// end generated by svg_widgets.rb

		addParam(createParam<Knob16>(channelsParamPosition, module, PolyMult::CHANNELS_PARAM));

		addInput(createInput<Port24>(channelsInputPosition, module, PolyMult::CHANNELS_INPUT));
		addInput(createInput<Port24>(inInputPosition, module, PolyMult::IN_INPUT));

		addOutput(createOutput<Port24>(out1OutputPosition, module, PolyMult::OUT1_OUTPUT));
		addOutput(createOutput<Port24>(out2OutputPosition, module, PolyMult::OUT2_OUTPUT));
		addOutput(createOutput<Port24>(out3OutputPosition, module, PolyMult::OUT3_OUTPUT));
		addOutput(createOutput<Port24>(out4OutputPosition, module, PolyMult::OUT4_OUTPUT));
	}
};

Model* modelPolyMult = createModel<PolyMult, PolyMultWidget>("Bogaudio-PolyMult", "POLYMULT", "Monophonic-to-polyphonic multiple", "Polyphonic");
