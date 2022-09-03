/*************************************************************************/
/*  webrtc_peer_connection_extension.h                                   */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef WEBRTC_PEER_CONNECTION_EXTENSION_H
#define WEBRTC_PEER_CONNECTION_EXTENSION_H

#include "webrtc_peer_connection.h"

#include "core/extension/ext_wrappers.gen.inc"
#include "core/object/gdvirtual.gen.inc"
#include "core/object/script_language.h"
#include "core/variant/native_ptr.h"

class WebRTCPeerConnectionExtension : public WebRTCPeerConnection {
	GDCLASS(WebRTCPeerConnectionExtension, WebRTCPeerConnection);

protected:
	static void _bind_methods();

public:
	// FIXME Can't be directly exposed due to issues in exchanging Ref(s) between godot and extensions.
	// See godot-cpp GH-652 .
	virtual Ref<WebRTCDataChannel> create_data_channel(String p_label, Dictionary p_options = Dictionary()) override;
	GDVIRTUAL2R(Object *, _create_data_channel, String, Dictionary);
	// EXBIND2R(Ref<WebRTCDataChannel>, create_data_channel, String, Dictionary);

	/** GDExtension **/
	EXBIND0RC(ConnectionState, get_connection_state);
	EXBIND1R(Error, initialize, Dictionary);
	EXBIND0R(Error, create_offer);
	EXBIND2R(Error, set_remote_description, String, String);
	EXBIND2R(Error, set_local_description, String, String);
	EXBIND3R(Error, add_ice_candidate, String, int, String);
	EXBIND0R(Error, poll);
	EXBIND0(close);

	WebRTCPeerConnectionExtension() {}
};

#endif // WEBRTC_PEER_CONNECTION_EXTENSION_H
