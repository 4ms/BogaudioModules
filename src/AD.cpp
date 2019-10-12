
#include "AD.hpp"

#define INVERT "invert"

void AD::Engine::reset() {
	trigger.reset();
	eocPulseGen.process(10.0);
	envelope.reset();
	on = false;
}

void AD::Engine::sampleRateChange() {
	float sr = APP->engine->getSampleRate();
	envelope.setSampleRate(sr);
	attackSL.setParams(sr / (float)modulationSteps);
	decaySL.setParams(sr / (float)modulationSteps);
}

void AD::reset() {
	for (int c = 0; c < _channels; ++c) {
		_engines[c]->reset();
	}
}

void AD::sampleRateChange() {
	for (int c = 0; c < _channels; ++c) {
		_engines[c]->sampleRateChange();
	}
}

json_t* AD::dataToJson() {
	json_t* root = json_object();
	json_object_set_new(root, INVERT, json_real(_invert));
	return root;
}

void AD::dataFromJson(json_t* root) {
	json_t* i = json_object_get(root, INVERT);
	if (i) {
		_invert = json_real_value(i);
	}
}

bool AD::active() {
	return inputs[TRIGGER_INPUT].isConnected() || outputs[ENV_OUTPUT].isConnected() || outputs[EOC_OUTPUT].isConnected();
}

int AD::channels() {
	return inputs[TRIGGER_INPUT].getChannels();
}

void AD::addEngine(int c) {
	_engines[c] = new Engine(_modulationSteps);
	_engines[c]->reset();
	_engines[c]->sampleRateChange();
}

void AD::removeEngine(int c) {
	delete _engines[c];
	_engines[c] = NULL;
}

void AD::modulateChannel(int c) {
	Engine& e = *_engines[c];

	float attack = powf(params[ATTACK_PARAM].getValue(), 2.0f);
	if (inputs[ATTACK_INPUT].isConnected()) {
		attack *= clamp(inputs[ATTACK_INPUT].getPolyVoltage(c) / 10.0f, 0.0f, 1.0f);
	}
	e.envelope.setAttack(e.attackSL.next(attack * 10.f));

	float decay = powf(params[DECAY_PARAM].getValue(), 2.0f);
	if (inputs[DECAY_INPUT].isConnected()) {
		decay *= clamp(inputs[DECAY_INPUT].getPolyVoltage(c) / 10.0f, 0.0f, 1.0f);
	}
	e.envelope.setDecay(e.decaySL.next(decay * 10.f));

	e.envelope.setLinearShape(_linearMode);
}

void AD::always(const ProcessArgs& args) {
	lights[LOOP_LIGHT].value = _loopMode = params[LOOP_PARAM].getValue() > 0.5f;
	lights[LINEAR_LIGHT].value = _linearMode = params[LINEAR_PARAM].getValue() > 0.5f;
	_attackLightSum = _decayLightSum = 0;
}

void AD::processChannel(const ProcessArgs& args, int c) {
	Engine& e = *_engines[c];

	e.trigger.process(inputs[TRIGGER_INPUT].getVoltage(c));
	if (!e.on && (e.trigger.isHigh() || (_loopMode && e.envelope.isStage(ADSR::STOPPED_STAGE)))) {
		e.on = true;
	}
	e.envelope.setGate(e.on);
	outputs[ENV_OUTPUT].setChannels(_channels);
	outputs[ENV_OUTPUT].setVoltage(e.envelope.next() * 10.0f * _invert, c);
	if (e.on && e.envelope.isStage(ADSR::SUSTAIN_STAGE)) {
		e.envelope.reset();
		e.on = false;
		e.eocPulseGen.trigger(0.001f);
	}
	outputs[EOC_OUTPUT].setChannels(_channels);
	outputs[EOC_OUTPUT].setVoltage(e.eocPulseGen.process(APP->engine->getSampleTime()) ? 5.0f : 0.0f, c);

	_attackLightSum += e.envelope.isStage(ADSR::ATTACK_STAGE);
	_decayLightSum += e.envelope.isStage(ADSR::DECAY_STAGE);
}

void AD::postProcess(const ProcessArgs& args) {
	lights[ATTACK_LIGHT].value = _attackLightSum / (float)_channels;
	lights[DECAY_LIGHT].value = _decayLightSum / (float)_channels;
}

struct ADWidget : ModuleWidget {
	static constexpr int hp = 3;

	ADWidget(AD* module) {
		setModule(module);
		box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);

		{
			SvgPanel *panel = new SvgPanel();
			panel->box.size = box.size;
			panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/AD.svg")));
			addChild(panel);
		}

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		// generated by svg_widgets.rb
		auto attackParamPosition = Vec(8.0, 33.0);
		auto decayParamPosition = Vec(8.0, 90.0);
		auto loopParamPosition = Vec(33.5, 131.2);
		auto linearParamPosition = Vec(33.5, 145.7);

		auto triggerInputPosition = Vec(10.5, 163.5);
		auto attackInputPosition = Vec(10.5, 198.5);
		auto decayInputPosition = Vec(10.5, 233.5);

		auto envOutputPosition = Vec(10.5, 271.5);
		auto eocOutputPosition = Vec(10.5, 306.5);

		auto attackLightPosition = Vec(20.8, 65.0);
		auto decayLightPosition = Vec(20.8, 122.0);
		auto loopLightPosition = Vec(2.5, 132.5);
		auto linearLightPosition = Vec(2.5, 147.0);
		// end generated by svg_widgets.rb

		addParam(createParam<Knob29>(attackParamPosition, module, AD::ATTACK_PARAM));
		addParam(createParam<Knob29>(decayParamPosition, module, AD::DECAY_PARAM));
		addParam(createParam<StatefulButton9>(loopParamPosition, module, AD::LOOP_PARAM));
		addParam(createParam<StatefulButton9>(linearParamPosition, module, AD::LINEAR_PARAM));

		addInput(createInput<Port24>(triggerInputPosition, module, AD::TRIGGER_INPUT));
		addInput(createInput<Port24>(attackInputPosition, module, AD::ATTACK_INPUT));
		addInput(createInput<Port24>(decayInputPosition, module, AD::DECAY_INPUT));

		addOutput(createOutput<Port24>(envOutputPosition, module, AD::ENV_OUTPUT));
		addOutput(createOutput<Port24>(eocOutputPosition, module, AD::EOC_OUTPUT));

		addChild(createLight<TinyLight<GreenLight>>(attackLightPosition, module, AD::ATTACK_LIGHT));
		addChild(createLight<TinyLight<GreenLight>>(decayLightPosition, module, AD::DECAY_LIGHT));
		addChild(createLight<SmallLight<GreenLight>>(loopLightPosition, module, AD::LOOP_LIGHT));
		addChild(createLight<SmallLight<GreenLight>>(linearLightPosition, module, AD::LINEAR_LIGHT));
	}

	struct InvertMenuItem : MenuItem {
		AD* _module;

		InvertMenuItem(AD* module, const char* label, int offset)
		: _module(module)
		{
			this->text = label;
		}

		void onAction(const event::Action& e) override {
			if (_module->_invert < 0.0f) {
				_module->_invert = 1.0f;
			}
			else {
				_module->_invert = -1.0f;
			}
		}

		void step() override {
			MenuItem::step();
			rightText = _module->_invert == -1.0f ? "✔" : "";
		}
	};

	void appendContextMenu(Menu* menu) override {
		AD* m = dynamic_cast<AD*>(module);
		assert(m);
		menu->addChild(new MenuLabel());
		menu->addChild(new InvertMenuItem(m, "Invert output", -1));
	}
};

Model* modelAD = bogaudio::createModel<AD, ADWidget>("Bogaudio-AD", "AD", "Utility attack/decay envelope generator", "Envelope generator", "Polyphonic");
