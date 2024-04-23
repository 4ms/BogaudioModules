#include "skins.hpp"
#include <cstdio>

Skins globalSkins;

Skins &Skins::skins() {
	globalSkins.loadSkins();
	globalSkins.loadCssValues();
	globalSkins._loaded = true;
	return globalSkins;
}

bool Skins::validKey(const std::string &key) const {
	for (auto s : _available) {
		if (s.key == key) {
			return true;
		}
	}
	return false;
}

const char *Skins::skinCssValue(const std::string &skinKey, const std::string &valueKey) const {
	std::string sk = skinKey;
	if (sk == "default") {
		sk = defaultKey();
	}
	if (!validKey(sk)) {
		return NULL;
	}

	auto values = _skinCssValues.find(sk);
	if (values == _skinCssValues.end()) {
		return NULL;
	}
	auto value = values->second.find(valueKey);
	if (value == values->second.end()) {
		return NULL;
	}
	return value->second.c_str();
}

NVGcolor Skins::cssColorToNVGColor(const char *color, const NVGcolor &ifError) {
	auto h2i = [](char h) {
		switch (h) {
			case '0':
				return 0;
			case '1':
				return 1;
			case '2':
				return 2;
			case '3':
				return 3;
			case '4':
				return 4;
			case '5':
				return 5;
			case '6':
				return 6;
			case '7':
				return 7;
			case '8':
				return 8;
			case '9':
				return 9;
			case 'A':
				return 10;
			case 'B':
				return 11;
			case 'C':
				return 12;
			case 'D':
				return 13;
			case 'E':
				return 14;
			case 'F':
				return 15;
			case 'a':
				return 10;
			case 'b':
				return 11;
			case 'c':
				return 12;
			case 'd':
				return 13;
			case 'e':
				return 14;
			case 'f':
				return 15;
			default:
				return -1;
		}
	};
	if (color[0] == '#') {
		if (strlen(color) == 4) {
			int i1 = h2i(color[1]);
			int i2 = h2i(color[2]);
			int i3 = h2i(color[3]);
			if (i1 >= 0 && i2 >= 0 && i3 >= 0) {
				return nvgRGBA(16 * i1 + i1, 16 * i2 + i2, 16 * i3 + i3, 0xff);
			}
		} else if (strlen(color) == 7) {
			int i11 = h2i(color[1]);
			int i12 = h2i(color[1]);
			int i21 = h2i(color[3]);
			int i22 = h2i(color[4]);
			int i31 = h2i(color[5]);
			int i32 = h2i(color[6]);
			if (i11 >= 0 && i12 >= 0 && i21 >= 0 && i22 >= 0 && i31 >= 0 && i32 >= 0) {
				return nvgRGBA(16 * i11 + i12, 16 * i21 + i22, 16 * i31 + i32, 0xff);
			}
		}
	}
	return ifError;
}

void Skins::setDefaultSkin(std::string skinKey) {
	if (skinKey == "default") {
		skinKey = "light";
	}
	_default = skinKey;
	for (auto listener : _defaultSkinListeners) {
		listener->defaultSkinChanged(_default);
	}
}

void Skins::registerDefaultSkinChangeListener(DefaultSkinChangeListener *listener) {
	_defaultSkinListeners.insert(listener);
}

void Skins::deregisterDefaultSkinChangeListener(DefaultSkinChangeListener *listener) {
	_defaultSkinListeners.erase(listener);
}

void Skins::loadSkins() {
	_available.push_back(Skin("light", "Light"));
	_available.push_back(Skin("dark", "Dark"));
	_available.push_back(Skin("lowcontrast", "Dark (low-contrast)"));
	_default = "light";
}

void Skins::loadCssValues() {
	// "light": {
	//   "background-fill": "#ddd",
	//   "path-stroke": "#333",
	//   "knob-center": "#eee",
	//   "knob-rim": "#333",
	//   "knob-tick": "#fff"
	// },
	// "dark": {
	//   "background-fill": "#111",
	//   "path-stroke": "#ccc",
	//   "knob-center": "#888",
	//   "knob-rim": "#444",
	//   "knob-tick": "#fff"
	// },
	// "lowcontrast": {
	//   "background-fill": "#333",
	//   "path-stroke": "#b3b3b3",
	//   "knob-center": "#bbb",
	//   "knob-rim": "#555",
	//   "knob-tick": "#fff"
	// }

	// css_values_map lightvaluesMap;
	// lightvaluesMap.insert({"background-fill", "#ddd"});
	// _skinCssValues.insert(skin_css_values_map::value_type("light", valuesMap));
	///etc...
}

