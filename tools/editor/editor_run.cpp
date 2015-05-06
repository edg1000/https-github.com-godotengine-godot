/*************************************************************************/
/*  editor_run.cpp                                                       */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                    http://www.godotengine.org                         */
/*************************************************************************/
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                 */
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
#include "editor_run.h"
#include "globals.h"


EditorRun::Status EditorRun::get_status() const {

	return status;
}
Error EditorRun::run(const String& p_scene,const String p_custom_args,const List<String>& p_breakpoints,const String& p_edited_scene) {

	List<String> args;


	String resource_path = Globals::get_singleton()->get_resource_path();

	if (resource_path!="") {
		args.push_back("-path");
		args.push_back(resource_path.replace(" ","%20"));

	}

	if (true) {
		args.push_back("-rdebug");
		args.push_back("localhost:"+String::num(GLOBAL_DEF("debug/debug_port", 6007)));
	}

	if (p_custom_args!="") {

		Vector<String> cargs=p_custom_args.split(" ",false);
		for(int i=0;i<cargs.size();i++) {

			args.push_back(cargs[i].replace("%20"," ").replace("$scene",p_edited_scene.replace(" ","%20")));
		}
	}

	if (p_breakpoints.size()) {

		args.push_back("-bp");
		String bpoints;
		for(const List<String>::Element *E=p_breakpoints.front();E;E=E->next()) {

			bpoints+=E->get().replace(" ","%20");
			if (E->next())
				bpoints+=",";
		}

		args.push_back(bpoints);
	}

	args.push_back(p_scene);

	String exec = OS::get_singleton()->get_executable_path();

	printf("running: %ls", exec.c_str());
	for (List<String>::Element* E = args.front(); E ; E = E->next()) {

		printf(" %ls", E->get().c_str());
	};
	printf("\n");

	pid=0;
	Error err = OS::get_singleton()->execute(exec,args,false,&pid);
	ERR_FAIL_COND_V(err,err);

	status = STATUS_PLAY;

	return OK;
}

void EditorRun::stop() {

	if (status!=STATUS_STOP && pid!=0) {

		OS::get_singleton()->kill(pid);
	}

	status=STATUS_STOP;
}

EditorRun::EditorRun() {

	status=STATUS_STOP;
}
