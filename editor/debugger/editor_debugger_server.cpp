/*************************************************************************/
/*  editor_debugger_server.cpp                                           */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
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

#include "editor_debugger_server.h"

#include "core/io/marshalls.h"
#include "core/io/tcp_server.h"
#include "core/os/mutex.h"
#include "core/os/thread.h"
#include "editor/editor_log.h"
#include "editor/editor_node.h"
#include "editor/editor_settings.h"

class EditorDebuggerServerTCP : public EditorDebuggerServer {

private:
	Ref<TCP_Server> server;

public:
	virtual void poll() override {}
	virtual Error start() override;
	virtual void stop() override;
	virtual bool is_active() const override;
	virtual bool is_connection_available() const override;
	virtual Ref<RemoteDebuggerPeer> take_connection() override;

	EditorDebuggerServerTCP();
};

EditorDebuggerServerTCP::EditorDebuggerServerTCP() {
	server.instance();
}

Error EditorDebuggerServerTCP::start() {
	int remote_port = (int)EditorSettings::get_singleton()->get("network/debug/remote_port");
	const Error err = server->listen(remote_port);
	if (err != OK) {
		EditorNode::get_log()->add_message(String("Error listening on port ") + itos(remote_port), EditorLog::MSG_TYPE_ERROR);
		return err;
	}
	return err;
}

void EditorDebuggerServerTCP::stop() {
	server->stop();
}

bool EditorDebuggerServerTCP::is_active() const {
	return server->is_listening();
}

bool EditorDebuggerServerTCP::is_connection_available() const {
	return server->is_listening() && server->is_connection_available();
}

Ref<RemoteDebuggerPeer> EditorDebuggerServerTCP::take_connection() {
	ERR_FAIL_COND_V(!is_connection_available(), Ref<RemoteDebuggerPeer>());
	return memnew(RemoteDebuggerPeerTCP(server->take_connection()));
}

EditorDebuggerServer *EditorDebuggerServer::create_default() {
	return memnew(EditorDebuggerServerTCP);
}
