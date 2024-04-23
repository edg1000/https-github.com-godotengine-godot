/**************************************************************************/
/*  crash_handler_macos.mm                                                */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "crash_handler_macos.h"

#include "core/config/project_settings.h"
#include "core/os/os.h"
#include "core/string/print_string.h"
#include "core/version.h"
#include "main/main.h"
#include "servers/display_server.h"

#include <string.h>
#include <unistd.h>

#if defined(DEBUG_ENABLED)
#define CRASH_HANDLER_ENABLED 1
#endif

#ifdef CRASH_HANDLER_ENABLED
#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>

#include <mach-o/dyld.h>
#include <mach-o/getsect.h>

static uint64_t load_address() {
	const struct segment_command_64 *cmd = getsegbyname("__TEXT");
	char full_path[1024];
	uint32_t size = sizeof(full_path);

	if (cmd && !_NSGetExecutablePath(full_path, &size)) {
		uint32_t dyld_count = _dyld_image_count();
		for (uint32_t i = 0; i < dyld_count; i++) {
			const char *image_name = _dyld_get_image_name(i);
			if (image_name && strncmp(image_name, full_path, 1024) == 0) {
				return cmd->vmaddr + _dyld_get_image_vmaddr_slide(i);
			}
		}
	}

	return 0;
}

static void handle_crash(int sig) {
	signal(SIGSEGV, SIG_DFL);
	signal(SIGFPE, SIG_DFL);
	signal(SIGILL, SIG_DFL);
	signal(SIGTRAP, SIG_DFL);

	if (OS::get_singleton() == nullptr) {
		abort();
	}

	void *bt_buffer[256];
	size_t size = backtrace(bt_buffer, 256);
	String _execpath = OS::get_singleton()->get_executable_path();

	String msg;
	String log_path;
	const ProjectSettings *proj_settings = ProjectSettings::get_singleton();
	if (proj_settings) {
		msg = proj_settings->get_setting_with_override("debug/settings/crash_handler/message");
		if (proj_settings->get_setting_with_override("debug/file_logging/enable_file_logging")) {
			log_path = proj_settings->globalize_path(proj_settings->get_setting_with_override("debug/file_logging/log_path"));
		}
	}

	// Tell MainLoop about the crash. This can be handled by users too in Node.
	if (OS::get_singleton()->get_main_loop()) {
		OS::get_singleton()->get_main_loop()->notification(MainLoop::NOTIFICATION_CRASH);
	}

	// Set window title while dumping the backtrace, as this can take 10+ seconds to finish.
	DisplayServer::get_singleton()->window_set_title("ERROR: Program crashed, dumping backtrace to log...");

	// Dump the backtrace to stderr with a message to the user
	print_error("\n================================================================");
	print_error(vformat("%s: Program crashed with signal %d.", __FUNCTION__, sig));

	// Print the engine version just before, so that people are reminded to include the version in backtrace reports.
	if (String(VERSION_HASH).is_empty()) {
		print_error(vformat("Engine version: %s", VERSION_FULL_NAME));
	} else {
		print_error(vformat("Engine version: %s (%s)", VERSION_FULL_NAME, VERSION_HASH));
	}
	print_error(vformat("Dumping the backtrace. %s", msg));
	char **strings = backtrace_symbols(bt_buffer, size);
	if (strings) {
		void *load_addr = (void *)load_address();

		for (size_t i = 1; i < size; i++) {
			char fname[1024];
			Dl_info info;

			snprintf(fname, 1024, "%s", strings[i]);

			// Try to demangle the function name to provide a more readable one
			if (dladdr(bt_buffer[i], &info) && info.dli_sname) {
				if (info.dli_sname[0] == '_') {
					int status;
					char *demangled = abi::__cxa_demangle(info.dli_sname, nullptr, 0, &status);

					if (status == 0 && demangled) {
						snprintf(fname, 1024, "%s", demangled);
					}

					if (demangled) {
						free(demangled);
					}
				}
			}

			String output = fname;

			// Try to get the file/line number using atos
			if (bt_buffer[i] > (void *)0x0 && OS::get_singleton()) {
				List<String> args;
				char str[1024];

				args.push_back("-o");
				args.push_back(_execpath);
#if defined(__x86_64) || defined(__x86_64__) || defined(__amd64__)
				args.push_back("-arch");
				args.push_back("x86_64");
#elif defined(__aarch64__)
				args.push_back("-arch");
				args.push_back("arm64");
#endif
				args.push_back("-l");
				snprintf(str, 1024, "%p", load_addr);
				args.push_back(str);
				snprintf(str, 1024, "%p", bt_buffer[i]);
				args.push_back(str);

				int ret;
				String out = "";
				Error err = OS::get_singleton()->execute(String("atos"), args, &out, &ret);
				if (err == OK && out.substr(0, 2) != "0x") {
					out = out.substr(0, out.length() - 1);
					output = out;
				}
			}

			print_error(vformat("[%d] %s", (int64_t)i, output));
		}

		free(strings);
	}
	print_error("-- END OF BACKTRACE --");
	print_error("================================================================");

	// Notify the user that backtrace dumping is finished.
	DisplayServer::get_singleton()->window_set_title("ERROR: Program crashed, dumped backtrace to log");

	// Show alert so the user is aware of the crash, even if the engine wasn't started with visible stdout/stderr.
	// This must be done after printing the backtrace to prevent the process from being blocked too early (`OS::alert()` is blocking).
	if (!log_path.is_empty()) {
		// `fprintf()` is used instead of `print_error()`, as this printed line must not be present
		// in the log file (it references the log file itself).
		fprintf(stderr, "\nFind the log file for this session at:\n%s\n\n", log_path.utf8().get_data());

		if (DisplayServer::get_singleton()->get_name() != "headless") {
			// Use AppleScript in `OS::execute()` instead of `OS::alert()` so that it works even when the OS event loop is blocked by the crash handler.
			// (this is only required on macOS).
			List<String> args;
			args.push_back("-e");
			args.push_back(vformat("display alert \"Crash\" message \"%s: Program crashed with signal %d.\\n\\nFind the log file for this session at:\\n%s\\n\\nClicking OK will open this log file. Please include the this log file's contents in bug reports.\"", __FUNCTION__, sig, log_path.c_escape()));
			OS::get_singleton()->execute("osascript", args);
			args.clear();
			args.push_back(log_path);
			// Use `OS::execute()` instead of `OS::shell_open()` so that it works even when the OS event loop is blocked by the crash handler.
			OS::get_singleton()->execute("open", args);
		}
	} else if (DisplayServer::get_singleton()->get_name() != "headless") {
		// Use AppleScript in `OS::execute()` instead of `OS::alert()` so that it works even when the OS event loop is blocked by the crash handler.
		List<String> args;
		args.push_back("-e");
		args.push_back(vformat("display alert \"Crash\" message \"%s: Program crashed with signal %d.\"", __FUNCTION__, sig));
		OS::get_singleton()->execute("osascript", args);
	}

	// Abort to pass the error to the OS
	abort();
}
#endif

CrashHandler::CrashHandler() {
	disabled = false;
}

CrashHandler::~CrashHandler() {
	disable();
}

void CrashHandler::disable() {
	if (disabled) {
		return;
	}

#ifdef CRASH_HANDLER_ENABLED
	signal(SIGSEGV, SIG_DFL);
	signal(SIGFPE, SIG_DFL);
	signal(SIGILL, SIG_DFL);
	signal(SIGTRAP, SIG_DFL);
#endif

	disabled = true;
}

void CrashHandler::initialize() {
#ifdef CRASH_HANDLER_ENABLED
	signal(SIGSEGV, handle_crash);
	signal(SIGFPE, handle_crash);
	signal(SIGILL, handle_crash);
	signal(SIGTRAP, handle_crash);
#endif
}
