
#include "Additator.hpp"

const int modulationSteps = 100;
const float maxWidth = 2.0f;
const float maxSkew = 0.99f;
const float minAmplitudeNormalization = 1.0f;
const float maxAmplitudeNormalization = 5.0f;
const float minDecay = -1.0f;
const float maxDecay = 3.0f;
const float minFilter = 0.1;
const float maxFilter = 1.9;
const float slewLimitTime = 1.0f;

void Additator::onReset() {
	_syncTrigger.reset();
	_steps = modulationSteps;
	_phase = PHASE_RESET;
}

void Additator::onSampleRateChange() {
	float sampleRate = APP->engine->getSampleRate();
	_oscillator.setSampleRate(sampleRate);
	_maxFrequency = 0.475f * sampleRate;
	_steps = modulationSteps;
	_phase = PHASE_RESET;
	_widthSL.setParams(sampleRate, slewLimitTime, maxWidth);
	_oddSkewSL.setParams(sampleRate, slewLimitTime, 2.0f * maxSkew);
	_evenSkewSL.setParams(sampleRate, slewLimitTime, 2.0f * maxSkew);
	_amplitudeNormalizationSL.setParams(sampleRate, slewLimitTime, maxAmplitudeNormalization - minAmplitudeNormalization);
	_decaySL.setParams(sampleRate, slewLimitTime, maxDecay - minDecay);
	_balanceSL.setParams(sampleRate, slewLimitTime, 2.0f);
	_filterSL.setParams(sampleRate, slewLimitTime, maxFilter - minFilter);
}

float Additator::cvValue(Input& cv, bool dc) {
	if (!cv.active) {
		return dc ? 1.0f : 0.0f;
	}
	if (dc) {
		return clamp(cv.value / 10.0f, 0.0f, 1.0f);
	}
	return clamp(cv.value / 5.0f, -1.0f, 1.0f);
}

void Additator::process(const ProcessArgs& args) {
	if (!outputs[AUDIO_OUTPUT].isConnected()) {
		Phase phase = params[PHASE_PARAM].getValue() > 1.5f ? PHASE_COSINE : PHASE_SINE;
		lights[SINE_LIGHT].value = phase == PHASE_SINE;
		lights[COSINE_LIGHT].value = phase == PHASE_COSINE;
		return;
	}
	lights[SINE_LIGHT].value = _phase == PHASE_SINE;
	lights[COSINE_LIGHT].value = _phase == PHASE_COSINE;

	++_steps;
	if (_steps >= modulationSteps) {
		_steps = 0;

		float width = _widthSL.next(clamp(params[WIDTH_PARAM].getValue() + (maxWidth / 2.0f) * cvValue(inputs[WIDTH_INPUT]), 0.0f, maxWidth));
		float oddSkew = _oddSkewSL.next(clamp(params[ODD_SKEW_PARAM].getValue() + cvValue(inputs[ODD_SKEW_INPUT]), -maxSkew, maxSkew));
		float evenSkew = _evenSkewSL.next(clamp(params[EVEN_SKEW_PARAM].getValue() + cvValue(inputs[EVEN_SKEW_INPUT]), -maxSkew, maxSkew));
		if (
			_width != width ||
			_oddSkew != oddSkew ||
			_evenSkew != evenSkew
		) {
			_width = width;
			_oddSkew = oddSkew;
			_evenSkew = evenSkew;

			float multiple = 1.0f;
			_oscillator.setPartialFrequencyRatio(1, multiple);
			_activePartials = 1;
			for (int i = 2, n = _oscillator.partialCount(); i <= n; ++i) {
				float ii = i;
				if (i % 2 == 0) {
					ii += _evenSkew;
				}
				else {
					ii += _oddSkew;
				}
				if (_oscillator.setPartialFrequencyRatio(i, powf(ii, _width))) {
					_activePartials = i;
				}
			}
		}

		int partials = clamp((int)roundf(params[PARTIALS_PARAM].getValue() * cvValue(inputs[PARTIALS_INPUT], true)), 0, maxPartials);
		float amplitudeNormalization = _amplitudeNormalizationSL.next(clamp(params[GAIN_PARAM].getValue() + ((maxAmplitudeNormalization - minAmplitudeNormalization) / 2.0f) * cvValue(inputs[GAIN_INPUT]), minAmplitudeNormalization, maxAmplitudeNormalization));
		float decay = _decaySL.next(clamp(params[DECAY_PARAM].getValue() + ((maxDecay - minDecay) / 2.0f) * cvValue(inputs[DECAY_INPUT]), minDecay, maxDecay));
		float balance = _balanceSL.next(clamp(params[BALANCE_PARAM].getValue() + cvValue(inputs[BALANCE_INPUT]), -1.0f, 1.0f));
		float filter = _filterSL.next(clamp(params[FILTER_PARAM].getValue() + cvValue(inputs[FILTER_INPUT]), minFilter, maxFilter));
		if (
			_partials != partials ||
			_amplitudeNormalization != amplitudeNormalization ||
			_decay != decay ||
			_balance != balance ||
			_filter != filter
		) {
			int envelopes = _partials != partials ? std::max(_partials, partials) : 0;
			_partials = partials;
			_amplitudeNormalization = amplitudeNormalization;
			_decay = decay;
			_balance = balance;
			_filter = filter;

			float as[maxPartials + 1];
			float total = as[1] = 1.0f;
			filter = log10f(_filter) + 1.0f;
			int np = std::min(_partials, _activePartials);
			for (int i = 2, n = _oscillator.partialCount(); i <= n; ++i) {
				as[i] = 0.0f;
				if (i <= np) {
					as[i] = powf(i, -_decay) * powf(_filter, i);
					if (i % 2 == 0) {
						if (_balance > 0.0f) {
							as[i] *= 1.0f - _balance;
						}
					}
					else {
						if (_balance < 0.0f) {
							as[i] *= 1.0f + _balance;
						}
					}
					total += as[i];
				}
			}
			float norm = std::max(np / (float)_oscillator.partialCount(), 0.1f);
			norm = 1.0f + (_amplitudeNormalization - 1.0f) * norm;
			norm = std::max(total / norm, 0.7f);
			for (int i = 1, n = _oscillator.partialCount(); i <= n; ++i) {
				as[i] /= norm;
				_oscillator.setPartialAmplitude(i, as[i], i <= envelopes);
			}
		}

		float frequency = params[FREQUENCY_PARAM].getValue();
		frequency += params[FINE_PARAM].getValue() / 12.0f;;
		if (inputs[PITCH_INPUT].isConnected()) {
			frequency += clamp(inputs[PITCH_INPUT].getVoltage(), -5.0f, 5.0f);
		}
		frequency = clamp(cvToFrequency(frequency), 20.0f, _maxFrequency);
		_oscillator.setFrequency(frequency);

		Phase phase = params[PHASE_PARAM].getValue() > 1.5f ? PHASE_COSINE : PHASE_SINE;
		if (_phase != phase) {
			_phase = phase;
			_oscillator.syncToPhase(_phase == PHASE_SINE ? 0.0f : M_PI / 2.0f);
		}
	}

	if (_syncTrigger.next(inputs[SYNC_INPUT].getVoltage())) {
		_oscillator.syncToPhase(_phase == PHASE_SINE ? 0.0f : M_PI / 2.0f);
	}
	outputs[AUDIO_OUTPUT].setVoltage(_oscillator.next() * 5.0);
}

struct AdditatorWidget : ModuleWidget {
	static constexpr int hp = 15;

	AdditatorWidget(Additator* module) {
		setModule(module);
		box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);

		{
			SVGPanel *panel = new SVGPanel();
			panel->box.size = box.size;
			panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Additator.svg")));
			addChild(panel);
		}

		addChild(createWidget<ScrewSilver>(Vec(15, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 0)));
		addChild(createWidget<ScrewSilver>(Vec(15, 365)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 365)));

		// generated by svg_widgets.rb
		auto frequencyParamPosition = Vec(40.0, 45.0);
		auto partialsParamPosition = Vec(165.0, 60.0);
		auto fineParamPosition = Vec(30.0, 160.0);
		auto widthParamPosition = Vec(79.0, 155.0);
		auto oddSkewParamPosition = Vec(132.0, 155.0);
		auto evenSkewParamPosition = Vec(184.0, 155.0);
		auto gainParamPosition = Vec(25.0, 218.0);
		auto decayParamPosition = Vec(79.0, 218.0);
		auto balanceParamPosition = Vec(132.0, 218.0);
		auto filterParamPosition = Vec(184.0, 218.0);
		auto phaseParamPosition = Vec(194.0, 299.0);

		auto syncInputPosition = Vec(16.0, 274.0);
		auto partialsInputPosition = Vec(50.0, 274.0);
		auto widthInputPosition = Vec(84.0, 274.0);
		auto oddSkewInputPosition = Vec(118.0, 274.0);
		auto evenSkewInputPosition = Vec(152.0, 274.0);
		auto pitchInputPosition = Vec(16.0, 318.0);
		auto gainInputPosition = Vec(50.0, 318.0);
		auto decayInputPosition = Vec(84.0, 318.0);
		auto balanceInputPosition = Vec(118.0, 318.0);
		auto filterInputPosition = Vec(152.0, 318.0);

		auto audioOutputPosition = Vec(186.0, 318.0);

		auto sineLightPosition = Vec(185.0, 272.0);
		auto cosineLightPosition = Vec(185.0, 287.0);
		// end generated by svg_widgets.rb

		addParam(createParam<Knob68>(frequencyParamPosition, module, Additator::FREQUENCY_PARAM));
		{
			auto w = createParam<Knob38>(partialsParamPosition, module, Additator::PARTIALS_PARAM);
			dynamic_cast<Knob*>(w)->snap = true;
			addParam(w);
		}
		addParam(createParam<Knob16>(fineParamPosition, module, Additator::FINE_PARAM));
		addParam(createParam<Knob26>(widthParamPosition, module, Additator::WIDTH_PARAM));
		addParam(createParam<Knob26>(oddSkewParamPosition, module, Additator::ODD_SKEW_PARAM));
		addParam(createParam<Knob26>(evenSkewParamPosition, module, Additator::EVEN_SKEW_PARAM));
		addParam(createParam<Knob26>gainParamPosition, module, Additator::GAIN_PARAM));
		addParam(createParam<Knob26>decayParamPosition, module, Additator::DECAY_PARAM));
		addParam(createParam<Knob26>(balanceParamPosition, module, Additator::BALANCE_PARAM));
		addParam(createParam<Knob26>filterParamPosition, module, Additator::FILTER_PARAM));
		addParam(createParam<StatefulButton9>(phaseParamPosition, module, Additator::PHASE_PARAM));

		addInput(createInput<Port24>(partialsInputPosition, module, Additator::PARTIALS_INPUT));
		addInput(createInput<Port24>(widthInputPosition, module, Additator::WIDTH_INPUT));
		addInput(createInput<Port24>(oddSkewInputPosition, module, Additator::ODD_SKEW_INPUT));
		addInput(createInput<Port24>(evenSkewInputPosition, module, Additator::EVEN_SKEW_INPUT));
		addInput(createInput<Port24>(gainInputPosition, module, Additator::GAIN_INPUT));
		addInput(createInput<Port24>(decayInputPosition, module, Additator::DECAY_INPUT));
		addInput(createInput<Port24>(balanceInputPosition, module, Additator::BALANCE_INPUT));
		addInput(createInput<Port24>(filterInputPosition, module, Additator::FILTER_INPUT));
		addInput(createInput<Port24>(pitchInputPosition, module, Additator::PITCH_INPUT));
		addInput(createInput<Port24>(syncInputPosition, module, Additator::SYNC_INPUT));

		addOutput(createOutput<Port24>(audioOutputPosition, module, Additator::AUDIO_OUTPUT));

		addChild(createLight<SmallLight<GreenLight>>(sineLightPosition, module, Additator::SINE_LIGHT));
		addChild(createLight<SmallLight<GreenLight>>(cosineLightPosition, module, Additator::COSINE_LIGHT));
	}
};

Model* modelAdditator = bogaudio::createModel<Additator, AdditatorWidget>("Bogaudio-Additator", "Additator",  "additive oscillator", OSCILLATOR_TAG);
