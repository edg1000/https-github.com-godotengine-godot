#include "skeleton_2d.h"

void Bone2D::_notification(int p_what) {

	if (p_what == NOTIFICATION_ENTER_TREE) {
		Node *parent = get_parent();
		parent_bone = Object::cast_to<Bone2D>(parent);
		skeleton = NULL;
		while (parent) {
			skeleton = Object::cast_to<Skeleton2D>(parent);
			if (skeleton)
				break;
			if (!Object::cast_to<Bone2D>(parent))
				break; //skeletons must be chained to Bone2Ds.

			parent = parent->get_parent();
		}

		if (skeleton) {
			Skeleton2D::Bone bone;
			bone.bone = this;
			skeleton->bones.push_back(bone);
			skeleton->_make_bone_setup_dirty();
		}
	}
	if (p_what == NOTIFICATION_LOCAL_TRANSFORM_CHANGED) {
		if (skeleton) {
			skeleton->_make_transform_dirty();
		}
	}
	if (p_what == NOTIFICATION_MOVED_IN_PARENT) {
		if (skeleton) {
			skeleton->_make_bone_setup_dirty();
		}
	}

	if (p_what == NOTIFICATION_EXIT_TREE) {
		if (skeleton) {
			for (int i = 0; i < skeleton->bones.size(); i++) {
				if (skeleton->bones[i].bone == this) {
					skeleton->bones.remove(i);
					break;
				}
			}
			skeleton->_make_bone_setup_dirty();
			skeleton = NULL;
		}
		parent_bone = NULL;
	}
}
void Bone2D::_bind_methods() {

	ClassDB::bind_method(D_METHOD("set_rest", "rest"), &Bone2D::set_rest);
	ClassDB::bind_method(D_METHOD("get_rest"), &Bone2D::get_rest);
	ClassDB::bind_method(D_METHOD("apply_rest"), &Bone2D::apply_rest);
	ClassDB::bind_method(D_METHOD("get_skeleton_rest"), &Bone2D::get_skeleton_rest);
	ClassDB::bind_method(D_METHOD("get_index_in_skeleton"), &Bone2D::get_index_in_skeleton);

	ADD_PROPERTY(PropertyInfo(Variant::TRANSFORM2D,"rest"),"set_rest","get_rest");
}

void Bone2D::set_rest(const Transform2D &p_rest) {
	rest = p_rest;
	if (skeleton)
		skeleton->_make_bone_setup_dirty();

	update_configuration_warning();
}

Transform2D Bone2D::get_rest() const {
	return rest;
}

Transform2D Bone2D::get_skeleton_rest() const {

	if (parent_bone) {
		return parent_bone->get_skeleton_rest() * rest;
	} else {
		return rest;
	}
}

void Bone2D::apply_rest() {
	set_transform(rest);
}

int Bone2D::get_index_in_skeleton() const {
	ERR_FAIL_COND_V(!skeleton,-1);
	skeleton->_update_bone_setup();
	return skeleton_index;
}
String Bone2D::get_configuration_warning() const {

	String warning = Node2D::get_configuration_warning();
	if (!skeleton) {
		if (warning!=String()) {
			warning+="\n";
		}
		if (parent_bone) {
			warning+=TTR("This Bone2D chain should end at a Skeleton2D node.");
		} else {
			warning+=TTR("A Bone2D only works with a Skeleton2D or another Bone2D as parent node.");
		}
	}

	if (rest==Transform2D(0,0,0,0,0,0)) {
		if (warning!=String()) {
			warning+="\n";
		}
		warning+=TTR("This bone lacks a proper REST pose. Go to the Skeleton2D node and set one.");

	}

	return warning;
}

Bone2D::Bone2D() {
	skeleton = NULL;
	parent_bone = NULL;
	skeleton_index=-1;
	set_notify_local_transform(true);
	//this is a clever hack so the bone knows no rest has been set yet, allowing to show an error.
	for(int i=0;i<3;i++) {
		rest[i]=Vector2(0,0);
	}
}

//////////////////////////////////////

void Skeleton2D::_make_bone_setup_dirty() {

	if (bone_setup_dirty)
		return;
	bone_setup_dirty = true;
	if (is_inside_tree()) {
		call_deferred("_update_bone_setup");
	}
}

void Skeleton2D::_update_bone_setup() {

	if (!bone_setup_dirty)
		return;

	bone_setup_dirty = false;
	VS::get_singleton()->skeleton_allocate(skeleton, bones.size(), true);

	bones.sort(); //sorty so they are always in the same order/index

	for (int i = 0; i < bones.size(); i++) {
		bones[i].rest_inverse = bones[i].bone->get_skeleton_rest().affine_inverse(); //bind pose
		bones[i].bone->skeleton_index=i;
		Bone2D *parent_bone = Object::cast_to<Bone2D>(bones[i].bone->get_parent());
		if (parent_bone) {
			bones[i].parent_index=parent_bone->skeleton_index;
		} else {
			bones[i].parent_index=-1;
		}
	}

	transform_dirty = true;
	_update_transform();
}

void Skeleton2D::_make_transform_dirty() {

	if (transform_dirty)
		return;
	transform_dirty = true;
	if (is_inside_tree()) {
		call_deferred("_update_transform");
	}
}

void Skeleton2D::_update_transform() {

	if (bone_setup_dirty) {
		_update_bone_setup();
		return; //above will update transform anyway
	}
	if (!transform_dirty)
		return;

	transform_dirty = false;

	for (int i = 0; i < bones.size(); i++) {

		ERR_CONTINUE(bones[i].parent_index>=i);
		if (bones[i].parent_index>=0) {
			bones[i].accum_transform = bones[bones[i].parent_index].accum_transform * bones[i].bone->get_transform();
		} else {
			bones[i].accum_transform = bones[i].bone->get_transform();
		}
	}

	for (int i = 0; i < bones.size(); i++) {

		Transform2D final_xform = bones[i].accum_transform * bones[i].rest_inverse;
		VS::get_singleton()->skeleton_bone_set_transform_2d(skeleton, i, final_xform);
	}
}

int Skeleton2D::get_bone_count() const {

	ERR_FAIL_COND_V(!is_inside_tree(), 0);

	if (bone_setup_dirty) {
		const_cast<Skeleton2D *>(this)->_update_bone_setup();
	}

	return bones.size();
}

Bone2D *Skeleton2D::get_bone(int p_idx) {

	ERR_FAIL_COND_V(!is_inside_tree(), NULL);
	ERR_FAIL_INDEX_V(p_idx, bones.size(), NULL);

	return bones[p_idx].bone;
}

void Skeleton2D::_notification(int p_what) {

	if (p_what == NOTIFICATION_READY) {

		if (bone_setup_dirty)
			_update_bone_setup();
		if (transform_dirty)
			_update_transform();

		request_ready();
	}

	if (p_what == NOTIFICATION_TRANSFORM_CHANGED) {
		VS::get_singleton()->skeleton_set_base_transform_2d(skeleton,get_global_transform());
	}
}

RID Skeleton2D::get_skeleton() const {
	return skeleton;
}
void Skeleton2D::_bind_methods() {

	ClassDB::bind_method(D_METHOD("_update_bone_setup"), &Skeleton2D::_update_bone_setup);
	ClassDB::bind_method(D_METHOD("_update_transform"), &Skeleton2D::_update_transform);

	ClassDB::bind_method(D_METHOD("get_bone_count"), &Skeleton2D::get_bone_count);
	ClassDB::bind_method(D_METHOD("get_bone"), &Skeleton2D::get_bone);

	ClassDB::bind_method(D_METHOD("get_skeleton"), &Skeleton2D::get_skeleton);
}

Skeleton2D::Skeleton2D() {
	bone_setup_dirty = true;
	transform_dirty = true;

	skeleton = VS::get_singleton()->skeleton_create();
}

Skeleton2D::~Skeleton2D() {

	VS::get_singleton()->free(skeleton);
}
