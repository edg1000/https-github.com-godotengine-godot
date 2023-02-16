// Copyright 2011-2020, Molecular Matters GmbH <office@molecular-matters.com>
// See LICENSE.txt for licensing details (2-clause BSD License: https://opensource.org/licenses/BSD-2-Clause)

#pragma once


PSD_NAMESPACE_BEGIN

/// \ingroup Types
/// \namespace imageResource
/// \brief A namespace holding some of the image resources known by Photoshop.
/// \details The type of image resource is indicated by a 4-byte integer stored in the Image Resources section.
namespace imageResource
{
	enum Enum
	{
		IPTC_NAA = 1028,
		CAPTION_DIGEST = 1061,
		XMP_METADATA = 1060,
		PRINT_INFORMATION = 1082,
		PRINT_STYLE = 1083,
		PRINT_SCALE = 1062,
		PRINT_FLAGS = 1011,
		PRINT_FLAGS_INFO = 10000,
		PRINT_INFO = 1071,
		RESOLUTION_INFO = 1005,
		DISPLAY_INFO = 1077,
		GLOBAL_ANGLE = 1037,
		GLOBAL_ALTITUDE = 1049,
		COLOR_HALFTONING_INFO = 1013,
		COLOR_TRANSFER_FUNCTIONS = 1016,
		MULTICHANNEL_HALFTONING_INFO = 1012,
		MULTICHANNEL_TRANSFER_FUNCTIONS = 1015,
		LAYER_STATE_INFORMATION = 1024,
		LAYER_GROUP_INFORMATION = 1026,
		LAYER_GROUP_ENABLED_ID = 1072,
		LAYER_SELECTION_ID = 1069,
		GRID_GUIDES_INFO = 1032,
		URL_LIST = 1054,
		SLICES = 1050,
		PIXEL_ASPECT_RATIO = 1064,
		ICC_PROFILE = 1039,
		ICC_UNTAGGED_PROFILE = 1041,
		ID_SEED_NUMBER = 1044,
		THUMBNAIL_RESOURCE = 1036,
		VERSION_INFO = 1057,
		EXIF_DATA = 1058,
		BACKGROUND_COLOR = 1010,
		ALPHA_CHANNEL_ASCII_NAMES = 1006,
		ALPHA_CHANNEL_UNICODE_NAMES = 1045,
		ALPHA_IDENTIFIERS = 1053,
		COPYRIGHT_FLAG = 1034,
		PATH_SELECTION_STATE = 1088,
		ONION_SKINS = 1078,
		TIMELINE_INFO = 1075,
		SHEET_DISCLOSURE = 1076,
		WORKING_PATH = 1025,
		MAC_PRINT_MANAGER_INFO = 1001,
		WINDOWS_DEVMODE = 1085
	};
}

PSD_NAMESPACE_END
