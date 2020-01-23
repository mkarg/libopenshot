/**
 * @file
 * @brief Source file for LUT class
 * @author Markus KARG <markus@headcrashing.eu>
 *
 * @ref License
 */

/* LICENSE
 *
 * Copyright (c) 2020 Markus KARG
 * This file is part of
 * OpenShot Library (libopenshot), an open-source project dedicated to
 * delivering high quality video editing and animation solutions to the
 * world. For more information visit <http://www.openshot.org/>.
 *
 * OpenShot Library (libopenshot) is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * OpenShot Library (libopenshot) is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with OpenShot Library. If not, see <http://www.gnu.org/licenses/>.
 */

#include "../../include/effects/LUT.h"
#include <cmath>

using namespace openshot;

/// Blank constructor, useful when using Json to load the effect properties
LUT::LUT() : lut(1.0), x("Y") {
	// Init effect properties
	init_effect_details();
}

// Default constructor
LUT::LUT(Keyframe lut, std::string x) : lut(lut), x(x)
{
	// Init effect properties
	init_effect_details();
}

// Init effect settings
void LUT::init_effect_details()
{
	/// Initialize the values of the EffectInfo struct.
	InitEffectInfo();

	/// Set the effect info
	info.class_name = "LUT";
	info.name = "Color Lookup Table";
	info.description = "Adjust the color space using a color lookup table (LUT).";
	info.has_audio = false;
	info.has_video = true;
}

// This method is required for all derived classes of EffectBase, and returns a
// modified openshot::Frame object
std::shared_ptr<Frame> LUT::GetFrame(std::shared_ptr<Frame> frame, int64_t frame_number)
{
	// Get the frame's image
	std::shared_ptr<QImage> frame_image = frame->GetImage();

	if (!frame_image)
		return frame;

	const ILUT* lookUpTable = CUBEReader::read("lut.cube");

	// Get keyframe values for this frame
	float lut_value = lut.GetValue(frame_number);

	// Loop through pixels
	unsigned char *pixels = (unsigned char *) frame_image->bits();
	for (int pixel = 0, byte_index=0; pixel < frame_image->width() * frame_image->height(); pixel++, byte_index+=4)
	{
		// Get the RGB values from the pixel
		const RGB sourceColor = { pixels[byte_index], pixels[byte_index + 1], pixels[byte_index + 2] };

		// Translate source color into target color using Color Look Up Table
		const RGB targetColor = lookUpTable->lookup(sourceColor);

		// Set all pixels to new value
		pixels[byte_index + 0] = constrain(targetColor.R);
		pixels[byte_index + 1] = constrain(targetColor.G);
		pixels[byte_index + 2] = constrain(targetColor.B);
	}

	delete lookUpTable;

	// return the modified frame
	return frame;
}

// Generate JSON string of this object
std::string LUT::Json() {

	// Return formatted string
	return JsonValue().toStyledString();
}

// Generate Json::JsonValue for this object
Json::Value LUT::JsonValue() {

	// Create root json object
	Json::Value root = EffectBase::JsonValue(); // get parent properties
	root["type"] = info.class_name;
	root["lut"] = lut.JsonValue();
	root["x"] = "Y";

	// return JsonValue
	return root;
}

// Load JSON string into this object
void LUT::SetJson(std::string value) {

	// Parse JSON string into JSON objects
	Json::Value root;
	Json::CharReaderBuilder rbuilder;
	Json::CharReader* reader(rbuilder.newCharReader());

	std::string errors;
	bool success = reader->parse( value.c_str(),
                 value.c_str() + value.size(), &root, &errors );
	delete reader;

	if (!success)
		// Raise exception
		throw InvalidJSON("JSON could not be parsed (or is invalid)");

	try
	{
		// Set all values that match
		SetJsonValue(root);
	}
	catch (const std::exception& e)
	{
		// Error parsing JSON (or missing keys)
		throw InvalidJSON("JSON is invalid (missing keys or invalid data types)");
	}
}

// Load Json::JsonValue into this object
void LUT::SetJsonValue(Json::Value root) {

	// Set parent data
	EffectBase::SetJsonValue(root);

	// Set data from Json (if key is found)
	if (!root["lut"].isNull())
		lut.SetJsonValue(root["lut"]);
}

// Get all properties for a specific frame
std::string LUT::PropertiesJSON(int64_t requested_frame) {

	// Generate JSON properties list
	Json::Value root;
	root["id"] = add_property_json("ID", 0.0, "string", Id(), NULL, -1, -1, true, requested_frame);
	root["position"] = add_property_json("Position", Position(), "float", "", NULL, 0, 30 * 60 * 60 * 48, false, requested_frame);
	root["layer"] = add_property_json("Track", Layer(), "int", "", NULL, 0, 20, false, requested_frame);
	root["start"] = add_property_json("Start", Start(), "float", "", NULL, 0, 30 * 60 * 60 * 48, false, requested_frame);
	root["end"] = add_property_json("End", End(), "float", "", NULL, 0, 30 * 60 * 60 * 48, false, requested_frame);
	root["duration"] = add_property_json("Duration", Duration(), "float", "", NULL, 0, 30 * 60 * 60 * 48, true, requested_frame);

	// Keyframes
	root["lut"] = add_property_json("LUT", lut.GetValue(requested_frame), "float", "", &lut, 0.0, 4.0, false, requested_frame);
	root["x"] = add_property_string_json("X", "Y");

	// Return formatted string
	return root.toStyledString();
}

// TODO Replace by C++20's std::lerp
float lerp(const float a, const float b, const float t) {
	return fma(t, b - a, a);
}

RGB C1DLUT::lookup(const RGB& rgb) const {
	return { compute(rgb.R, N, MIN_R, IVW_R, R), compute(rgb.G, N, MIN_G, IVW_G, G), compute(rgb.B, N, MIN_B, IVW_B, B) };
}

float C1DLUT::compute(const int source, const int n, const float min, const float intervalWidth, const float target[]) {
	const float i = (source - min) / intervalWidth;
	const int i0 = floor(i);
	const int i1 = ceil(i);
return i0 == i1 ? target[i0] : lerp(target[i0], target[i1], i);
}

RGB C3DLUT::lookup(const RGB& rgb) const {
	return {0, 0, 0};
}

ILUT* CUBEReader::read(const std::string& file){
	return new C1DLUT;
}
