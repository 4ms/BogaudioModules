#pragma once

#include "bogaudio.hpp"
#include "dsp/signal.hpp"

using namespace bogaudio::dsp;

namespace bogaudio {

struct MixerChannel {
	static const float maxDecibels;
	static const float minDecibels;
	static const float levelSlewTimeMS;
	static const float panSlewTimeMS;

	Amplifier _amplifier;
	Panner _panner;
	bogaudio::dsp::SlewLimiter _levelSL;
	bogaudio::dsp::SlewLimiter _panSL;
	RootMeanSquare _rms;

	Param& _levelParam;
	Param& _panParam;
	Param& _muteParam;
	Input& _levelInput;
	Input& _panInput;
	Input* _muteInput;

	float out = 0.0f;
	float left = 0.0f;
	float right = 0.0f;
	float rms = 0.0f;

	MixerChannel(
		Param& level,
		Param& pan,
		Param& mute,
		Input& levelCv,
		Input& panCv,
		float sampleRate = 1000.0f,
		Input* muteCv = NULL
	)
	: _levelParam(level)
	, _panParam(pan)
	, _muteParam(mute)
	, _levelInput(levelCv)
	, _panInput(panCv)
	, _muteInput(muteCv)
	{
		setSampleRate(sampleRate);
		_rms.setSensitivity(0.05f);
	}

	void setSampleRate(float sampleRate);
	void next(float sample, bool stereo, bool solo, int c = 0); // outputs on out, left, right, rms.
};

struct MuteButton : ToggleButton {
	MuteButton() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/button_18px_0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/button_18px_1_orange.svg")));
	}

	void onButton(const event::Button& e) override;
};

struct SoloMuteButton : ParamWidget {
	std::vector<std::shared_ptr<Svg>> _frames;
	SvgWidget* _svgWidget; // deleted elsewhere.
	CircularShadow* shadow = NULL;

	SoloMuteButton();
	void onButton(const event::Button& e) override;
	void onChange(const event::Change& e) override;
};

} // namespace bogaudio
