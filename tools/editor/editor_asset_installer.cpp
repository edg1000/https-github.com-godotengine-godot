#include "editor_asset_installer.h"
#include "io/zip_io.h"
#include "os/dir_access.h"
#include "os/file_access.h"
#include "editor_node.h"
void EditorAssetInstaller::_update_subitems(TreeItem* p_item,bool p_check,bool p_first) {


	if (p_check) {
		if (p_item->get_custom_color(0)==Color())	 {
			p_item->set_checked(0,true);
		}
	} else {
		p_item->set_checked(0,false);

	}

	if (p_item->get_children()) {
		_update_subitems(p_item->get_children(),p_check);
	}

	if (!p_first && p_item->get_next()) {
		_update_subitems(p_item->get_next(),p_check);
	}

}

void EditorAssetInstaller::_item_edited() {


	if (updating)
		return;

	TreeItem *item = tree->get_edited();
	if (!item)
		return;


	String path=item->get_metadata(0);

	updating=true;
	if (path==String()) { //a dir
		_update_subitems(item,item->is_checked(0),true);
	}

	if (item->is_checked(0)) {
		while(item) {
			item->set_checked(0,true);
			item=item->get_parent();
		}

	}
	updating=false;

}

void EditorAssetInstaller::open(const String& p_path,int p_depth) {


	package_path=p_path;
	Set<String> files_sorted;

	FileAccess *src_f=NULL;
	zlib_filefunc_def io = zipio_create_io_from_file(&src_f);

	unzFile pkg = unzOpen2(p_path.utf8().get_data(), &io);
	if (!pkg) {

		error->set_text("Error opening package file, not in zip format.");
		return;
	}

	int ret = unzGoToFirstFile(pkg);


	while(ret==UNZ_OK) {

		//get filename
		unz_file_info info;
		char fname[16384];
		ret = unzGetCurrentFileInfo(pkg,&info,fname,16384,NULL,0,NULL,0);

		String name=fname;
		files_sorted.insert(name);

		ret = unzGoToNextFile(pkg);
	}


	Map<String,Ref<Texture> > extension_guess;
	{
		extension_guess["png"]=get_icon("Texture","EditorIcons");
		extension_guess["jpg"]=get_icon("Texture","EditorIcons");
		extension_guess["tex"]=get_icon("Texture","EditorIcons");
		extension_guess["atex"]=get_icon("Texture","EditorIcons");
		extension_guess["dds"]=get_icon("Texture","EditorIcons");
		extension_guess["scn"]=get_icon("PackedScene","EditorIcons");
		extension_guess["tscn"]=get_icon("PackedScene","EditorIcons");
		extension_guess["xml"]=get_icon("PackedScene","EditorIcons");
		extension_guess["xscn"]=get_icon("PackedScene","EditorIcons");
		extension_guess["mtl"]=get_icon("Material","EditorIcons");
		extension_guess["shd"]=get_icon("Shader","EditorIcons");
		extension_guess["gd"]=get_icon("GDScript","EditorIcons");
	}

	Ref<Texture> generic_extension = get_icon("Object","EditorIcons");


	unzClose(pkg);

	updating=true;
	tree->clear();
	TreeItem *root=tree->create_item();
	root->set_cell_mode(0,TreeItem::CELL_MODE_CHECK);
	root->set_checked(0,true);
	root->set_icon(0,get_icon("folder","FileDialog"));
	root->set_text(0,"res://");
	root->set_editable(0,true);
	Map<String,TreeItem*> dir_map;

	for(Set<String>::Element *E=files_sorted.front();E;E=E->next()) {

		String path = E->get();
		int depth=p_depth;
		bool skip=false;
		while(depth>0) {
			int pp = path.find("/");
			if (pp==-1) {
				skip=true;
				break;
			}
			path=path.substr(pp+1,path.length());
			depth--;
		}

		if (skip || path==String())
			continue;

		bool isdir=false;

		if (path.ends_with("/")) {
			//a directory
			path=path.substr(0,path.length()-1);
			isdir=true;
		}

		int pp = path.find_last("/");


		TreeItem *parent;
		if (pp==-1) {
			parent=root;
		} else {
			String ppath=path.substr(0,pp);
			print_line("PPATH IS: "+ppath);
			ERR_CONTINUE(!dir_map.has(ppath));
			parent=dir_map[ppath];

		}

		TreeItem *ti = tree->create_item(parent);
		ti->set_cell_mode(0,TreeItem::CELL_MODE_CHECK);
		ti->set_checked(0,true);
		ti->set_editable(0,true);
		if (isdir) {
			dir_map[path]=ti;
			ti->set_text(0,path.get_file()+"/");
			ti->set_icon(0,get_icon("folder","FileDialog"));
		} else {
			String file = path.get_file();
			String extension = file.extension().to_lower();
			if (extension_guess.has(extension)) {
				ti->set_icon(0,extension_guess[extension]);
			} else {
				ti->set_icon(0,generic_extension);
			}
			ti->set_text(0,file);


			String res_path = "res://"+path;
			if (FileAccess::exists(res_path)) {
				ti->set_custom_color(0,Color(1,0.3,0.2));
				ti->set_tooltip(0,res_path+" (Already Exists)");
				ti->set_checked(0,false);
			} else {
				ti->set_tooltip(0,res_path);

			}

			ti->set_metadata(0,res_path);

		}

		status_map[E->get()]=ti;



	}
	popup_centered_ratio();
	updating=false;

}

void EditorAssetInstaller::ok_pressed() {

	FileAccess *src_f=NULL;
	zlib_filefunc_def io = zipio_create_io_from_file(&src_f);

	unzFile pkg = unzOpen2(package_path.utf8().get_data(), &io);
	if (!pkg) {

		error->set_text("Error opening package file, not in zip format.");
		return;
	}

	int ret = unzGoToFirstFile(pkg);

	Vector<String> failed_files;

	ProgressDialog::get_singleton()->add_task("uncompress","Uncompressing Assets",status_map.size());

	int idx=0;
	while(ret==UNZ_OK) {

		//get filename
		unz_file_info info;
		char fname[16384];
		ret = unzGetCurrentFileInfo(pkg,&info,fname,16384,NULL,0,NULL,0);

		String name=fname;

		if (status_map.has(name) && status_map[name]->is_checked(0)) {

			String path = status_map[name]->get_metadata(0);
			if (path==String()) { // a dir

				String dirpath;
				TreeItem *t = status_map[name];
				while(t) {
					dirpath=t->get_text(0)+dirpath;
					t=t->get_parent();
				}

				if (dirpath.ends_with("/")) {
					dirpath=dirpath.substr(0,dirpath.length()-1);
				}

				DirAccess *da = DirAccess::create(DirAccess::ACCESS_RESOURCES);
				da->make_dir(dirpath);
				memdelete(da);

			} else {

				Vector<uint8_t> data;
				data.resize(info.uncompressed_size);

				//read
				unzOpenCurrentFile(pkg);
				unzReadCurrentFile(pkg,data.ptr(),data.size());
				unzCloseCurrentFile(pkg);

				FileAccess *f=FileAccess::open(path,FileAccess::WRITE);
				if (f) {
					f->store_buffer(data.ptr(),data.size());
					memdelete(f);
				} else {
					failed_files.push_back(path);
				}

				ProgressDialog::get_singleton()->task_step("uncompress",path,idx);
			}

		}

		idx++;
		ret = unzGoToNextFile(pkg);
	}

	ProgressDialog::get_singleton()->end_task("uncompress");
	unzClose(pkg);

	if (failed_files.size()) {
		String msg="The following files failed extraction from package:\n\n";
		for(int i=0;i<failed_files.size();i++) {

			if (i>15) {
				msg+="\nAnd "+itos(failed_files.size()-i)+" more files.";
				break;
			}
			msg+=failed_files[i];
		}
		EditorNode::get_singleton()->show_warning(msg);
	} else {
		EditorNode::get_singleton()->show_warning("Package Installed Successfully!","Success!");
	}




}

void EditorAssetInstaller::_bind_methods() {

	ObjectTypeDB::bind_method("_item_edited",&EditorAssetInstaller::_item_edited);

}

EditorAssetInstaller::EditorAssetInstaller() {

	VBoxContainer *vb = memnew( VBoxContainer );
	add_child(vb);
	set_child_rect(vb);

	tree = memnew( Tree );
	vb->add_margin_child("Package Contents:",tree,true);
	tree->connect("item_edited",this,"_item_edited");

	error = memnew( AcceptDialog) ;
	add_child(error);
	get_ok()->set_text("Install");
	set_title("Package Installer");

	updating=false;

	set_hide_on_ok(true);
}

