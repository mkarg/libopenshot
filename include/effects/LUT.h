/**
 * @file
 * @brief Header file for LUT class
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

#ifndef OPENSHOT_LUT_EFFECT_H
#define OPENSHOT_LUT_EFFECT_H

#include "../EffectBase.h"

#include <cmath>
#include <ctime>
#include <iostream>
#include <omp.h>
#include <stdio.h>
#include <memory>
#include "../Color.h"
#include "../Exceptions.h"
#include "../Json.h"
#include "../KeyFrame.h"
#include "../ReaderBase.h"
#include "../FFmpegReader.h"
#include "../QtImageReader.h"
#include "../ChunkReader.h"

namespace openshot
{

	/**
	 * @brief This class adjusts the color space on a frame's image using a color lookup table (LUT).
	 *
	 * This can be animated by passing in a Keyframe. Animating the color space can create
	 * some very cool effects.
	 */
	class LUT : public EffectBase
	{
	private:
		/// Init effect settings
		void init_effect_details();

	public:
		Keyframe lut;	///< LUT: The color lookup table
		std::string x;

		/// Blank constructor, useful when using Json to load the effect properties
		LUT();

		/// Default constructor, which takes a curve, to adjust the color space over time.
		///
		/// @param lut The color lookup table
		LUT(Keyframe lut, std::string x);

		/// @brief This method is required for all derived classes of EffectBase, and returns a
		/// modified openshot::Frame object
		///
		/// The frame object is passed into this method, and a frame_number is passed in which
		/// tells the effect which settings to use from its keyframes (starting at 1).
		///
		/// @returns The modified openshot::Frame object
		/// @param frame The frame object that needs the effect applied to it
		/// @param frame_number The frame number (starting at 1) of the effect on the timeline.
		std::shared_ptr<Frame> GetFrame(std::shared_ptr<Frame> frame, int64_t frame_number);

		/// Get and Set JSON methods
		std::string Json(); ///< Generate JSON string of this object
		void SetJson(std::string value); ///< Load JSON string into this object
		Json::Value JsonValue(); ///< Generate Json::JsonValue for this object
		void SetJsonValue(Json::Value root); ///< Load Json::JsonValue into this object

		/// Get all properties for a specific frame (perfect for a UI to display the current state
		/// of all properties at any time)
		std::string PropertiesJSON(int64_t requested_frame);
	};

	struct RGB {
		int R, G, B;
	};

	class ILUT {
	public:
		virtual RGB lookup(const RGB& rgb) const = 0;
	};

	class C1DLUT: public ILUT {
	private:
		static inline float intervalWidth(const float a, const float b, const int n) {
			return (b - a) / n;
		}
		static inline float compute(const int source, const int n, const float min, const float intervalWidth, const float target[]);
		static const int N = 1;
		const float MIN_R = 0, MIN_G = 0, MIN_B = 0;
		const float MAX_R = 255, MAX_G = 255, MAX_B = 255;
		const float IVW_R = intervalWidth(MIN_R, MAX_R, N), IVW_G = intervalWidth(MIN_G, MAX_G, N), IVW_B = intervalWidth(MIN_B, MAX_B, N);
		const float R[N + 1] = { 0, 255 }, G[N + 1] = { 0, 255 }, B[N + 1] = { 0, 255 };
	public:
		RGB lookup(const RGB& rgb) const;
	};

	class C3DLUT: public ILUT {
		RGB lookup(const RGB& rgb) const;
	};

	class CUBEReader {
	public:
		static ILUT* read(const std::string& file);
	};

}

#endif
