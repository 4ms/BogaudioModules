
#include <assert.h>
#include <math.h>
#include <algorithm>

#include "filters/multimode.hpp"

using namespace bogaudio::dsp;

void MultimodeFilter::setParams(
	float sampleRate,
	Type type,
	int poles,
	Mode mode,
	float frequency,
	float qbw,
	BandwidthMode bwm
) {
	assert(poles >= minPoles && poles <= maxPoles);
	assert(poles % modPoles == 0);
	assert(frequency >= minFrequency && frequency <= maxFrequency);
	assert(qbw >= minQbw && qbw <= maxQbw);

	bool repole = _type != type || _mode != mode || _nPoles != poles || (type == CHEBYSHEV_TYPE && (mode == LOWPASS_MODE || mode == HIGHPASS_MODE) && _qbw != qbw);
	bool redesign = repole || _frequency != frequency || _qbw != qbw || _sampleRate != sampleRate || _bandwidthMode != bwm;
	_sampleRate = sampleRate;
	_half2PiST = M_PI * (1.0f / sampleRate);
	_type = type;
	_nPoles = poles;
	_mode = mode;
	_frequency = frequency;
	_qbw = qbw;
	_bandwidthMode = bwm;

	if (repole) {
		switch (_type) {
			case BUTTERWORTH_TYPE: {
				int np = _nPoles / 2 + (_nPoles % 2 == 1);
				for (int k = 1, j = np - 1; k <= np; ++k, --j) {
					T a = (T)(2 * k + _nPoles - 1) * M_PI / (T)(2 * _nPoles);
					T re = std::cos(a);
					T im = std::sin(a);
					_poles[j] = Pole(-re, im, re + re, re * re + im * im);
				}

				_outGain = 1.0f;
				break;
			}

			case CHEBYSHEV_TYPE: {
				T ripple = 3.0;
				if (mode == LOWPASS_MODE || mode == HIGHPASS_MODE) {
					ripple += std::max(0.0f, 6.0f * qbw);
				}
				T e = ripple / (T)10.0;
				e = std::pow((T)10.0, e);
				e -= (T)1.0;
				e = std::sqrt(e);
				T ef = std::asinh((T)1.0 / e) / (float)_nPoles;
				T efr = -std::sinh(ef);
				T efi = std::cosh(ef);

				int np = _nPoles / 2 + (_nPoles % 2 == 1);
				for (int k = 1, j = np - 1; k <= np; ++k, --j) {
					T a = (T)(2 * k - 1) * M_PI / (T)(2 * _nPoles);
					T re = efr * std::sin(a);
					T im = efi * std::cos(a);
					_poles[j] = Pole(-re, im, re + re, re * re + im * im);
				}

				_outGain = 1.0 / (e * std::pow(2.0, (T)(_nPoles - 1)));
				// _outGain = 1.0f / std::pow(2.0f, (T)(_nPoles - 1));
				break;
			}

			default: {
				assert(false);
			}
		}
	}

	if (redesign) {
		switch (_mode) {
			case LOWPASS_MODE:
			case HIGHPASS_MODE: {
				int nf = _nPoles / 2 + _nPoles % 2;
				for (int i = _nFilters; i < nf; ++i) {
					_filters[i].reset();
				}
				_nFilters = nf;

				// T iq = (1.0 / std::sqrt(2.0)) - 0.65 * _qbw;
				T iq = (T)0.8 - (T)0.6 * _qbw;
				T wa = std::tan(_frequency * _half2PiST);
				T wa2 = wa * wa;

				if (_mode == LOWPASS_MODE) {
					int ni = 0;
					int nf = _nFilters;
					if (_nPoles % 2 == 1) {
						++ni;
						--nf;
						T wap = wa * std::real(_poles[0].p);
						_filters[0].setParams(wa, wa, 0.0, wap + (T)1.0, wap - (T)1.0, (T)0.0);
					}
					T a0 = wa2;
					T a1 = wa2 + wa2;
					T a2 = wa2;
					for (int i = 0; i < nf; ++i) {
						Pole& pole = _poles[ni + i];
						T ywa2 = pole.y * wa2;
						T ywa21 = ywa2 + (T)1.0;
						T x = (((T)(i == nf / 2) * (iq - (T)1.0)) + (T)1.0) * pole.x;
						T xwa = x * wa;
						T b0 = ywa21 - xwa;
						T b1 = (T)-2.0 + (ywa2 + ywa2);
						T b2 = ywa21 + xwa;
						_filters[ni + i].setParams(a0, a1, a2, b0, b1, b2);
					}
				}
				else {
					int ni = 0;
					int nf = _nFilters;
					if (_nPoles % 2 == 1) {
						++ni;
						--nf;
						T rp = std::real(_poles[0].p);
						_filters[0].setParams(1.0, -1.0, 0.0, wa + rp, wa - rp, 0.0);
					}
					T a0 = 1.0;
					T a1 = -2.0f;
					T a2 = 1.0;
					for (int i = 0; i < nf; ++i) {
						Pole& pole = _poles[ni + i];
						T wa2y = wa2 + pole.y;
						T x = (((T)(i == nf / 2) * (iq - (T)1.0)) + (T)1.0) * pole.x;
						T xwa = x * wa;
						T b0 = wa2y - xwa;
						T b1 = (wa2 + wa2) - (pole.y + pole.y);
						T b2 = wa2y + xwa;
						_filters[ni + i].setParams(a0, a1, a2, b0, b1, b2);
					}
				}
				break;
			}

			case BANDPASS_MODE:
			case BANDREJECT_MODE: {
				int nf = ((_nPoles / 2) * 2) + (_nPoles % 2);
				for (int i = _nFilters; i < nf; ++i) {
					_filters[i].reset();
				}
				_nFilters = nf;

				T wdl = 0.0;
				T wdh = 0.0;
				switch (_bandwidthMode) {
					case LINEAR_BANDWIDTH_MODE: {
						float bandwidth = std::max(minBWLinear, maxBWLinear * _qbw);
						wdl = std::max(minFrequency, _frequency - 0.5f * bandwidth);
						wdh = std::min(maxFrequency, std::max((float)wdl + 10.0f, _frequency + 0.5f * bandwidth));
						break;
					}
					case PITCH_BANDWIDTH_MODE: {
						float bandwidth = std::max(minBWPitch, maxBWPitch * _qbw);
						wdl = std::max(minFrequency, powf(2.0f, -bandwidth) * _frequency);
						wdh = std::min(maxFrequency, std::max((float)wdl + 10.0f, powf(2.0f, bandwidth) * _frequency));
						break;
					}
					default: {
						assert(false);
					}
				}
				T wal = std::tan(wdl * _half2PiST);
				T wah = std::tan(wdh * _half2PiST);
				T w = wah - wal;
				T w2 = w * w;
				T w02 = wah * wal;

				if (_mode == BANDPASS_MODE) {
					T a0 = w;
					T a1 = 0.0;
					T a2 = -w;

					int ni = 0;
					int nf = _nFilters;
					if (_nPoles % 2 == 1) {
						++ni;
						--nf;
						T wp = w * std::real(_poles[0].p);
						_filters[0].setParams(
							a0,
							a1,
							a2,
							(T)1.0 + wp + w02,
							(T)-2.0 + (w02 + w02),
							(T)1.0 - wp + w02
						);
					}
					for (int i = 0; i < nf; i += 2) {
						Pole& pole = _poles[ni + i / 2];
						TC x = pole.p2;
						x *= w2;
						x -= (T)4.0 * w02;
						x = std::sqrt(x);
						TC xc = std::conj(x);
						TC wp = w * pole.p;
						TC wpc = w * pole.pc;
						TC y1 = (x - wp) * (T)0.5;
						TC y1c = (xc - wpc) * (T)0.5;
						TC y2 = (-x - wp) * (T)0.5;
						TC y2c = (-xc - wpc) * (T)0.5;
						TC cf1a = -(y1 + y1c);
						TC cf2a = y1 * y1c;
						TC cf1b = -(y2 + y2c);
						TC cf2b = y2 * y2c;
						T f1a = std::real(cf1a);
						T f1b = std::real(cf1b);
						T f2a = std::real(cf2a);
						T f2b = std::real(cf2b);

						{
							T b0 = (T)1.0 + f1a + f2a;
							T b1 = (T)-2.0 + (f2a + f2a);
							T b2 = (T)1.0 - f1a + f2a;
							_filters[ni + i].setParams(a0, a1, a2, b0, b1, b2);
						}
						{
							T b0 = (T)1.0 + f1b + f2b;
							T b1 = (T)-2.0 + (f2b + f2b);
							T b2 = (T)1.0 - f1b + f2b;
							_filters[ni + i + 1].setParams(a0, a1, a2, b0, b1, b2);
						}
					}
				}
				else {
					T a0 = (T)1.0 + w02;
					T a1 = (T)-2.0 + (w02 + w02);
					T a2 = a0;

					int ni = 0;
					int nf = _nFilters;
					if (_nPoles % 2 == 1) {
						++ni;
						--nf;
						T rp = std::real(_poles[0].p);
						T rpw02 = rp * w02;
						_filters[0].setParams(
							a0,
							a1,
							a2,
							rp + w + rpw02,
							(T)-2.0 * rp + (rpw02 + rpw02),
							rp - w + rpw02
						);
					}
					for (int i = 0; i < nf; i += 2) {
						Pole& pole = _poles[ni + i / 2];
						TC x = pole.p2;
						x *= (T)-4.0 * w02;
						x += w2;
						x = std::sqrt(x);
						TC xc = std::conj(x);
						TC y1 = (x - w) * pole.i2p;
						TC y1c = (xc - w) * pole.i2pc;
						TC y2 = (-x - w) * pole.i2p;
						TC y2c = (-xc - w) * pole.i2pc;
						TC cf1a = -pole.r * (y1 + y1c);
						TC cf2a = pole.r * y1 * y1c;
						TC cf1b = -pole.r * (y2 + y2c);
						TC cf2b = pole.r * y2 * y2c;
						T f1a = std::real(cf1a);
						T f1b = std::real(cf1b);
						T f2a = std::real(cf2a);
						T f2b = std::real(cf2b);

						{
							T b0 = pole.r + f1a + f2a;
							T b1 = (T)-2.0 * pole.r + (f2a + f2a);
							T b2 = pole.r - f1a + f2a;
							_filters[ni + i].setParams(a0, a1, a2, b0, b1, b2);
						}
						{
							T b0 = pole.r + f1b + f2b;
							T b1 = (T)-2.0 * pole.r + (f2b + f2b);
							T b2 = pole.r - f1b + f2b;
							_filters[ni + i + 1].setParams(a0, a1, a2, b0, b1, b2);
						}
					}
				}
				break;
			}

			default: {
				assert(false);
			}
		}
	}
}

float MultimodeFilter::next(float sample) {
	for (int i = 0; i < _nFilters; ++i) {
		sample = _filters[i].next(sample);
	}
	return _outGain * sample;
}

void MultimodeFilter::reset() {
	for (int i = 0; i < _nFilters; ++i) {
		_filters[i].reset();
	}
}