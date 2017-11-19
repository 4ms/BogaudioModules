
#include "dsp/digital.hpp"
#include "BogaudioModules.hpp"

struct Manual : Module {
	enum ParamsIds {
		TRIGGER_PARAM,
		NUM_PARAMS
	};

	enum InputsIds {
		NUM_INPUTS
	};

	enum OutputsIds {
		OUT1_OUTPUT,
		OUT2_OUTPUT,
		OUT3_OUTPUT,
		OUT4_OUTPUT,
		OUT5_OUTPUT,
		OUT6_OUTPUT,
		OUT7_OUTPUT,
		OUT8_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightsIds {
		NUM_LIGHTS
	};

	SchmittTrigger _trigger;

	Manual() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		reset();
	}

	virtual void reset() override;
	virtual void step() override;
};

void Manual::reset() {
	_trigger.reset();
}

void Manual::step() {
	bool high = _trigger.process(params[TRIGGER_PARAM].value) || _trigger.isHigh();
	float out = high ? 5.0 : 0.0;
	outputs[OUT1_OUTPUT].value = out;
	outputs[OUT2_OUTPUT].value = out;
	outputs[OUT3_OUTPUT].value = out;
	outputs[OUT4_OUTPUT].value = out;
	outputs[OUT5_OUTPUT].value = out;
	outputs[OUT6_OUTPUT].value = out;
	outputs[OUT7_OUTPUT].value = out;
	outputs[OUT8_OUTPUT].value = out;
}


ManualWidget::ManualWidget() {
	Manual *module = new Manual();
	setModule(module);
	box.size = Vec(RACK_GRID_WIDTH * 3, RACK_GRID_HEIGHT);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/Manual.svg")));
		addChild(panel);
	}

	addChild(createScrew<ScrewSilver>(Vec(0, 0)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 15, 365)));

	// generated by svg_widgets.rb
	auto triggerParamPosition = Vec(13.5, 28.0);

	auto out1OutputPosition = Vec(10.5, 64.0);
	auto out2OutputPosition = Vec(10.5, 94.0);
	auto out3OutputPosition = Vec(10.5, 124.0);
	auto out4OutputPosition = Vec(10.5, 154.0);
	auto out5OutputPosition = Vec(10.5, 184.0);
	auto out6OutputPosition = Vec(10.5, 214.0);
	auto out7OutputPosition = Vec(10.5, 244.0);
	auto out8OutputPosition = Vec(10.5, 274.0);
	// end generated by svg_widgets.rb

	addParam(createParam<Button18>(triggerParamPosition, module, Manual::TRIGGER_PARAM, 0.0, 1.0, 0.0));

	addOutput(createOutput<PJ301MPort>(out1OutputPosition, module, Manual::OUT1_OUTPUT));
	addOutput(createOutput<PJ301MPort>(out2OutputPosition, module, Manual::OUT2_OUTPUT));
	addOutput(createOutput<PJ301MPort>(out3OutputPosition, module, Manual::OUT3_OUTPUT));
	addOutput(createOutput<PJ301MPort>(out4OutputPosition, module, Manual::OUT4_OUTPUT));
	addOutput(createOutput<PJ301MPort>(out5OutputPosition, module, Manual::OUT5_OUTPUT));
	addOutput(createOutput<PJ301MPort>(out6OutputPosition, module, Manual::OUT6_OUTPUT));
	addOutput(createOutput<PJ301MPort>(out7OutputPosition, module, Manual::OUT7_OUTPUT));
	addOutput(createOutput<PJ301MPort>(out8OutputPosition, module, Manual::OUT8_OUTPUT));
}
