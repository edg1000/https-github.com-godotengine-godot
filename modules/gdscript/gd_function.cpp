#include "gd_function.h"
#include "gd_script.h"
#include "os/os.h"
#include "gd_functions.h"

Variant *GDFunction::_get_variant(int p_address,GDInstance *p_instance,GDScript *p_script,Variant &self, Variant *p_stack,String& r_error) const{

	int address = p_address&ADDR_MASK;

	//sequential table (jump table generated by compiler)
	switch((p_address&ADDR_TYPE_MASK)>>ADDR_BITS) {

		case ADDR_TYPE_SELF: {

			if (!p_instance) {
				r_error="Cannot access self without instance.";
				return NULL;
			}
			return &self;
		} break;
		case ADDR_TYPE_CLASS: {

			return &p_script->_static_ref;
		} break;
		case ADDR_TYPE_MEMBER: {
			//member indexing is O(1)
			if (!p_instance) {
				r_error="Cannot access member without instance.";
				return NULL;
			}
			return &p_instance->members[address];
		} break;
		case ADDR_TYPE_CLASS_CONSTANT: {

			//todo change to index!
			GDScript *o=p_script;
			ERR_FAIL_INDEX_V(address,_global_names_count,NULL);
			const StringName *sn = &_global_names_ptr[address];

			while(o) {
				GDScript *s=o;
				while(s) {

					Map<StringName,Variant>::Element *E=s->constants.find(*sn);
					if (E) {
						return &E->get();
					}
					s=s->_base;
				}
				o=o->_owner;
			}


			ERR_EXPLAIN("GDCompiler bug..");
			ERR_FAIL_V(NULL);
		} break;
		case ADDR_TYPE_LOCAL_CONSTANT: {
			ERR_FAIL_INDEX_V(address,_constant_count,NULL);
			return &_constants_ptr[address];
		} break;
		case ADDR_TYPE_STACK:
		case ADDR_TYPE_STACK_VARIABLE: {
			ERR_FAIL_INDEX_V(address,_stack_size,NULL);
			return &p_stack[address];
		} break;
		case ADDR_TYPE_GLOBAL: {


			ERR_FAIL_INDEX_V(address,GDScriptLanguage::get_singleton()->get_global_array_size(),NULL);


			return &GDScriptLanguage::get_singleton()->get_global_array()[address];
		} break;
		case ADDR_TYPE_NIL: {
			return &nil;
		} break;
	}

	ERR_EXPLAIN("Bad Code! (Addressing Mode)");
	ERR_FAIL_V(NULL);
	return NULL;
}


String GDFunction::_get_call_error(const Variant::CallError& p_err, const String& p_where,const Variant**argptrs) const {



	String err_text;

	if (p_err.error==Variant::CallError::CALL_ERROR_INVALID_ARGUMENT) {
		int errorarg=p_err.argument;
		err_text="Invalid type in "+p_where+". Cannot convert argument "+itos(errorarg+1)+" from "+Variant::get_type_name(argptrs[errorarg]->get_type())+" to "+Variant::get_type_name(p_err.expected)+".";
	} else if (p_err.error==Variant::CallError::CALL_ERROR_TOO_MANY_ARGUMENTS) {
		err_text="Invalid call to "+p_where+". Expected "+itos(p_err.argument)+" arguments.";
	} else if (p_err.error==Variant::CallError::CALL_ERROR_TOO_FEW_ARGUMENTS) {
		err_text="Invalid call to "+p_where+". Expected "+itos(p_err.argument)+" arguments.";
	} else if (p_err.error==Variant::CallError::CALL_ERROR_INVALID_METHOD) {
		err_text="Invalid call. Nonexistent "+p_where+".";
	} else if (p_err.error==Variant::CallError::CALL_ERROR_INSTANCE_IS_NULL) {
		err_text="Attempt to call "+p_where+" on a null instance.";
	} else {
		err_text="Bug, call error: #"+itos(p_err.error);
	}

	return err_text;

}

static String _get_var_type(const Variant* p_type) {

	String basestr;

	if (p_type->get_type()==Variant::OBJECT) {
		Object *bobj = *p_type;
		if (!bobj) {
			basestr = "null instance";
		} else {
#ifdef DEBUG_ENABLED
			if (ObjectDB::instance_validate(bobj)) {
				if (bobj->get_script_instance())
					basestr= bobj->get_class()+" ("+bobj->get_script_instance()->get_script()->get_path().get_file()+")";
				else
					basestr = bobj->get_class();
			} else {
				basestr="previously freed instance";
			}

#else
			basestr="Object";
#endif
		}

	} else {
		basestr = Variant::get_type_name(p_type->get_type());
	}

	return basestr;

}

Variant GDFunction::call(GDInstance *p_instance, const Variant **p_args, int p_argcount, Variant::CallError& r_err, CallState *p_state) {


	if (!_code_ptr) {

		return Variant();
	}

	r_err.error=Variant::CallError::CALL_OK;

	Variant self;
	Variant retvalue;
	Variant *stack = NULL;
	Variant **call_args;
	int defarg=0;

#ifdef DEBUG_ENABLED

	//GDScriptLanguage::get_singleton()->calls++;

#endif

	uint32_t alloca_size=0;
	GDScript *_class;
	int ip=0;
	int line=_initial_line;



	if (p_state) {
		//use existing (supplied) state (yielded)
		stack=(Variant*)p_state->stack.ptr();
		call_args=(Variant**)&p_state->stack[sizeof(Variant)*p_state->stack_size];
		line=p_state->line;
		ip=p_state->ip;
		alloca_size=p_state->stack.size();
		_class=p_state->_class;
		p_instance=p_state->instance;
		defarg=p_state->defarg;
		self=p_state->self;
		//stack[p_state->result_pos]=p_state->result; //assign stack with result

	} else {

		if (p_argcount!=_argument_count) {

			if (p_argcount>_argument_count) {

				r_err.error=Variant::CallError::CALL_ERROR_TOO_MANY_ARGUMENTS;
				r_err.argument=_argument_count;


				return Variant();
			} else if (p_argcount < _argument_count - _default_arg_count) {

				r_err.error=Variant::CallError::CALL_ERROR_TOO_FEW_ARGUMENTS;
				r_err.argument=_argument_count - _default_arg_count;
				return Variant();
			} else {

				defarg=_argument_count-p_argcount;
			}
		}

		alloca_size = sizeof(Variant*)*_call_size + sizeof(Variant)*_stack_size;

		if (alloca_size) {

			uint8_t *aptr = (uint8_t*)alloca(alloca_size);

			if (_stack_size) {

				stack=(Variant*)aptr;
				for(int i=0;i<p_argcount;i++)
					memnew_placement(&stack[i],Variant(*p_args[i]));
				for(int i=p_argcount;i<_stack_size;i++)
					memnew_placement(&stack[i],Variant);
			} else {
				stack=NULL;
			}

			if (_call_size) {

				call_args = (Variant**)&aptr[sizeof(Variant)*_stack_size];
			} else {

				call_args=NULL;
			}


		} else {
			stack=NULL;
			call_args=NULL;
		}

		if (p_instance) {
			if (p_instance->base_ref && static_cast<Reference*>(p_instance->owner)->is_referenced()) {

				self=REF(static_cast<Reference*>(p_instance->owner));
			} else {
				self=p_instance->owner;
			}
			_class=p_instance->script.ptr();
		} else {
			_class=_script;
		}
	}

	String err_text;

#ifdef DEBUG_ENABLED

	if (ScriptDebugger::get_singleton())
		GDScriptLanguage::get_singleton()->enter_function(p_instance,this,stack,&ip,&line);

#define CHECK_SPACE(m_space)\
	ERR_BREAK((ip+m_space)>_code_size)

#define GET_VARIANT_PTR(m_v,m_code_ofs) \
	Variant *m_v; \
	m_v = _get_variant(_code_ptr[ip+m_code_ofs],p_instance,_class,self,stack,err_text);\
	if (!m_v)\
	break;


#else
#define CHECK_SPACE(m_space)
#define GET_VARIANT_PTR(m_v,m_code_ofs) \
	Variant *m_v; \
	m_v = _get_variant(_code_ptr[ip+m_code_ofs],p_instance,_class,self,stack,err_text);

#endif


#ifdef DEBUG_ENABLED

	uint64_t function_start_time;
	uint64_t function_call_time;

	if (GDScriptLanguage::get_singleton()->profiling) {
		function_start_time=OS::get_singleton()->get_ticks_usec();
		function_call_time=0;
		profile.call_count++;
		profile.frame_call_count++;
	}
#endif
	bool exit_ok=false;

	while(ip<_code_size) {


		int last_opcode=_code_ptr[ip];
		switch(_code_ptr[ip]) {

			case OPCODE_OPERATOR: {

				CHECK_SPACE(5);

				bool valid;
				Variant::Operator op = (Variant::Operator)_code_ptr[ip+1];
				ERR_BREAK(op>=Variant::OP_MAX);

				GET_VARIANT_PTR(a,2);
				GET_VARIANT_PTR(b,3);
				GET_VARIANT_PTR(dst,4);

#ifdef DEBUG_ENABLED
				Variant ret;
				Variant::evaluate(op,*a,*b,ret,valid);
#else
				Variant::evaluate(op,*a,*b,*dst,valid);
#endif

				if (!valid) {
#ifdef DEBUG_ENABLED

					if (ret.get_type()==Variant::STRING) {
						//return a string when invalid with the error
						err_text=ret;
						err_text += " in operator '"+Variant::get_operator_name(op)+"'.";
					} else {
						err_text="Invalid operands '"+Variant::get_type_name(a->get_type())+"' and '"+Variant::get_type_name(b->get_type())+"' in operator '"+Variant::get_operator_name(op)+"'.";
					}
#endif
					break;

				}
#ifdef DEBUG_ENABLED
				*dst=ret;
#endif

				ip+=5;
				continue;
			}
			case OPCODE_EXTENDS_TEST: {

				CHECK_SPACE(4);

				GET_VARIANT_PTR(a,1);
				GET_VARIANT_PTR(b,2);
				GET_VARIANT_PTR(dst,3);

#ifdef DEBUG_ENABLED

				if (a->get_type()!=Variant::OBJECT || a->operator Object*()==NULL) {

					err_text="Left operand of 'extends' is not an instance of anything.";
					break;

				}
				if (b->get_type()!=Variant::OBJECT || b->operator Object*()==NULL) {

					err_text="Right operand of 'extends' is not a class.";
					break;

				}
#endif


				Object *obj_A = *a;
				Object *obj_B = *b;


				GDScript *scr_B = obj_B->cast_to<GDScript>();

				bool extends_ok=false;

				if (scr_B) {
					//if B is a script, the only valid condition is that A has an instance which inherits from the script
					//in other situation, this shoul return false.

					if (obj_A->get_script_instance() && obj_A->get_script_instance()->get_language()==GDScriptLanguage::get_singleton()) {

						GDScript *cmp = static_cast<GDScript*>(obj_A->get_script_instance()->get_script().ptr());
						//bool found=false;
						while(cmp) {

							if (cmp==scr_B) {
								//inherits from script, all ok
								extends_ok=true;
								break;

							}

							cmp=cmp->_base;
						}

					}


				} else {

					GDNativeClass *nc= obj_B->cast_to<GDNativeClass>();

					if (!nc) {

						err_text="Right operand of 'extends' is not a class (type: '"+obj_B->get_class()+"').";
						break;
					}

					extends_ok=ClassDB::is_parent_class(obj_A->get_class_name(),nc->get_name());
				}

				*dst=extends_ok;
				ip+=4;
				continue;
			}
			case OPCODE_SET: {

				CHECK_SPACE(3);

				GET_VARIANT_PTR(dst,1);
				GET_VARIANT_PTR(index,2);
				GET_VARIANT_PTR(value,3);

				bool valid;
				dst->set(*index,*value,&valid);

				if (!valid) {
					String v = index->operator String();
					if (v!="") {
						v="'"+v+"'";
					} else {
						v="of type '"+_get_var_type(index)+"'";
					}
					err_text="Invalid set index "+v+" (on base: '"+_get_var_type(dst)+"').";
					break;
				}

				ip+=4;
				continue;
			}
			case OPCODE_GET: {

				CHECK_SPACE(3);

				GET_VARIANT_PTR(src,1);
				GET_VARIANT_PTR(index,2);
				GET_VARIANT_PTR(dst,3);

				bool valid;
#ifdef DEBUG_ENABLED
				//allow better error message in cases where src and dst are the same stack position
				Variant ret = src->get(*index,&valid);
#else
				*dst = src->get(*index,&valid);

#endif
				if (!valid) {
					String v = index->operator String();
					if (v!="") {
						v="'"+v+"'";
					} else {
						v="of type '"+_get_var_type(index)+"'";
					}
					err_text="Invalid get index "+v+" (on base: '"+_get_var_type(src)+"').";
					break;
				}
#ifdef DEBUG_ENABLED
				*dst=ret;
#endif
				ip+=4;
				continue;
			}
			case OPCODE_SET_NAMED: {

				CHECK_SPACE(3);

				GET_VARIANT_PTR(dst,1);
				GET_VARIANT_PTR(value,3);

				int indexname = _code_ptr[ip+2];

				ERR_BREAK(indexname<0 || indexname>=_global_names_count);
				const StringName *index = &_global_names_ptr[indexname];

				bool valid;
				dst->set_named(*index,*value,&valid);

				if (!valid) {
					String err_type;
					err_text="Invalid set index '"+String(*index)+"' (on base: '"+_get_var_type(dst)+"').";
					break;
				}

				ip+=4;
				continue;
			}
			case OPCODE_GET_NAMED: {


				CHECK_SPACE(4);

				GET_VARIANT_PTR(src,1);
				GET_VARIANT_PTR(dst,3);

				int indexname = _code_ptr[ip+2];

				ERR_BREAK(indexname<0 || indexname>=_global_names_count);
				const StringName *index = &_global_names_ptr[indexname];

				bool valid;
#ifdef DEBUG_ENABLED
				//allow better error message in cases where src and dst are the same stack position
				Variant ret = src->get_named(*index,&valid);

#else
				*dst = src->get_named(*index,&valid);
#endif

				if (!valid) {
					if (src->has_method(*index)) {
						err_text="Invalid get index '"+index->operator String()+"' (on base: '"+_get_var_type(src)+"'). Did you mean '."+index->operator String()+"()' ?";
					} else {
						err_text="Invalid get index '"+index->operator String()+"' (on base: '"+_get_var_type(src)+"').";
					}
					break;
				}
#ifdef DEBUG_ENABLED
				*dst=ret;
#endif
				ip+=4;
				continue;
			}
			case OPCODE_SET_MEMBER: {

				CHECK_SPACE(3);
				int indexname = _code_ptr[ip+1];
				ERR_BREAK(indexname<0 || indexname>=_global_names_count);
				const StringName *index = &_global_names_ptr[indexname];
				GET_VARIANT_PTR(src,2);

				bool valid;
				bool ok = ClassDB::set_property(p_instance->owner,*index,*src,&valid);
#ifdef DEBUG_ENABLED
				if (!ok) {
					err_text="Internal error setting property: "+String(*index);
					break;
				} else if (!valid) {
					err_text="Error setting property '"+String(*index)+"' with value of type "+Variant::get_type_name(src->get_type())+".";
					break;

				}
#endif
				ip+=3;
				continue;
			}
			case OPCODE_GET_MEMBER: {

				CHECK_SPACE(3);
				int indexname = _code_ptr[ip+1];
				ERR_BREAK(indexname<0 || indexname>=_global_names_count);
				const StringName *index = &_global_names_ptr[indexname];
				GET_VARIANT_PTR(dst,2);
				bool ok = ClassDB::get_property(p_instance->owner,*index,*dst);

#ifdef DEBUG_ENABLED
				if (!ok) {
					err_text="Internal error getting property: "+String(*index);
					break;
				}
#endif
				ip+=3;
				continue;
			}
			case OPCODE_ASSIGN: {

				CHECK_SPACE(3);
				GET_VARIANT_PTR(dst,1);
				GET_VARIANT_PTR(src,2);

				*dst = *src;

				ip+=3;
				continue;
			}
			case OPCODE_ASSIGN_TRUE: {

				CHECK_SPACE(2);
				GET_VARIANT_PTR(dst,1);

				*dst = true;

				ip+=2;
				continue;
			}
			case OPCODE_ASSIGN_FALSE: {

				CHECK_SPACE(2);
				GET_VARIANT_PTR(dst,1);

				*dst = false;

				ip+=2;
				continue;
			}
			case OPCODE_CONSTRUCT: {

				CHECK_SPACE(2);
				Variant::Type t=Variant::Type(_code_ptr[ip+1]);
				int argc=_code_ptr[ip+2];
				CHECK_SPACE(argc+2);
				Variant **argptrs = call_args;
				for(int i=0;i<argc;i++) {
					GET_VARIANT_PTR(v,3+i);
					argptrs[i]=v;
				}

				GET_VARIANT_PTR(dst,3+argc);
				Variant::CallError err;
				*dst = Variant::construct(t,(const Variant**)argptrs,argc,err);

				if (err.error!=Variant::CallError::CALL_OK) {

					err_text=_get_call_error(err,"'"+Variant::get_type_name(t)+"' constructor",(const Variant**)argptrs);
					break;
				}

				ip+=4+argc;
				//construct a basic type
				continue;
			}
			case OPCODE_CONSTRUCT_ARRAY: {

				CHECK_SPACE(1);
				int argc=_code_ptr[ip+1];
				Array array; //arrays are always shared
				array.resize(argc);
				CHECK_SPACE(argc+2);

				for(int i=0;i<argc;i++) {
					GET_VARIANT_PTR(v,2+i);
					array[i]=*v;

				}

				GET_VARIANT_PTR(dst,2+argc);

				*dst=array;

				ip+=3+argc;
				continue;
			}
			case OPCODE_CONSTRUCT_DICTIONARY: {

				CHECK_SPACE(1);
				int argc=_code_ptr[ip+1];
				Dictionary dict; //arrays are always shared

				CHECK_SPACE(argc*2+2);

				for(int i=0;i<argc;i++) {

					GET_VARIANT_PTR(k,2+i*2+0);
					GET_VARIANT_PTR(v,2+i*2+1);
					dict[*k]=*v;

				}

				GET_VARIANT_PTR(dst,2+argc*2);

				*dst=dict;

				ip+=3+argc*2;
				continue;
			}
			case OPCODE_CALL_RETURN:
			case OPCODE_CALL: {


				CHECK_SPACE(4);
				bool call_ret = _code_ptr[ip]==OPCODE_CALL_RETURN;

				int argc=_code_ptr[ip+1];
				GET_VARIANT_PTR(base,2);
				int nameg=_code_ptr[ip+3];

				ERR_BREAK(nameg<0 || nameg>=_global_names_count);
				const StringName *methodname = &_global_names_ptr[nameg];

				ERR_BREAK(argc<0);
				ip+=4;
				CHECK_SPACE(argc+1);
				Variant **argptrs = call_args;

				for(int i=0;i<argc;i++) {
					GET_VARIANT_PTR(v,i);
					argptrs[i]=v;
				}

#ifdef DEBUG_ENABLED
				uint64_t call_time;

				if (GDScriptLanguage::get_singleton()->profiling) {
					call_time=OS::get_singleton()->get_ticks_usec();
				}

#endif
				Variant::CallError err;
				if (call_ret) {

					GET_VARIANT_PTR(ret,argc);
					base->call_ptr(*methodname,(const Variant**)argptrs,argc,ret,err);
				} else {

					base->call_ptr(*methodname,(const Variant**)argptrs,argc,NULL,err);
				}
#ifdef DEBUG_ENABLED
				if (GDScriptLanguage::get_singleton()->profiling) {
					function_call_time+=OS::get_singleton()->get_ticks_usec() - call_time;
				}
#endif

				if (err.error!=Variant::CallError::CALL_OK) {


					String methodstr = *methodname;
					String basestr = _get_var_type(base);

					if (methodstr=="call") {
						if (argc>=1) {
							methodstr=String(*argptrs[0])+" (via call)";
							if (err.error==Variant::CallError::CALL_ERROR_INVALID_ARGUMENT) {
								err.argument-=1;
							}
						}
					} if (methodstr=="free") {

						if (err.error==Variant::CallError::CALL_ERROR_INVALID_METHOD) {

							if (base->is_ref()) {
								err_text="Attempted to free a reference.";
								break;
							} else if (base->get_type()==Variant::OBJECT) {

								err_text="Attempted to free a locked object (calling or emitting).";
								break;
							}
						}
					}
					err_text=_get_call_error(err,"function '"+methodstr+"' in base '"+basestr+"'",(const Variant**)argptrs);
					break;
				}

				//_call_func(NULL,base,*methodname,ip,argc,p_instance,stack);
				ip+=argc+1;
				continue;
			}
			case OPCODE_CALL_BUILT_IN: {

				CHECK_SPACE(4);

				GDFunctions::Function func = GDFunctions::Function(_code_ptr[ip+1]);
				int argc=_code_ptr[ip+2];
				ERR_BREAK(argc<0);

				ip+=3;
				CHECK_SPACE(argc+1);
				Variant **argptrs = call_args;

				for(int i=0;i<argc;i++) {
					GET_VARIANT_PTR(v,i);
					argptrs[i]=v;
				}

				GET_VARIANT_PTR(dst,argc);

				Variant::CallError err;

				GDFunctions::call(func,(const Variant**)argptrs,argc,*dst,err);

				if (err.error!=Variant::CallError::CALL_OK) {


					String methodstr = GDFunctions::get_func_name(func);
					if (dst->get_type()==Variant::STRING) {
						//call provided error string
						err_text="Error calling built-in function '"+methodstr+"': "+String(*dst);
					} else {
						err_text=_get_call_error(err,"built-in function '"+methodstr+"'",(const Variant**)argptrs);
					}
					break;
				}
				ip+=argc+1;
				continue;
			}
			case OPCODE_CALL_SELF: {

				break;
			}
			case OPCODE_CALL_SELF_BASE: {

				CHECK_SPACE(2);
				int self_fun = _code_ptr[ip+1];
#ifdef DEBUG_ENABLED

				if (self_fun<0 || self_fun>=_global_names_count) {

					err_text="compiler bug, function name not found";
					break;
				}
#endif
				const StringName *methodname = &_global_names_ptr[self_fun];

				int argc=_code_ptr[ip+2];

				CHECK_SPACE(2+argc+1);

				Variant **argptrs = call_args;

				for(int i=0;i<argc;i++) {
					GET_VARIANT_PTR(v,i+3);
					argptrs[i]=v;
				}

				GET_VARIANT_PTR(dst,argc+3);

				const GDScript *gds = _script;


				const Map<StringName,GDFunction*>::Element *E=NULL;
				while (gds->base.ptr()) {
					gds=gds->base.ptr();
					E=gds->member_functions.find(*methodname);
					if (E)
						break;
				}

				Variant::CallError err;

				if (E) {

					*dst=E->get()->call(p_instance,(const Variant**)argptrs,argc,err);
				} else if (gds->native.ptr()) {

					if (*methodname!=GDScriptLanguage::get_singleton()->strings._init) {

						MethodBind *mb = ClassDB::get_method(gds->native->get_name(),*methodname);
						if (!mb) {
							err.error=Variant::CallError::CALL_ERROR_INVALID_METHOD;
						} else {
							*dst=mb->call(p_instance->owner,(const Variant**)argptrs,argc,err);
						}
					} else {
						err.error=Variant::CallError::CALL_OK;
					}
				} else {

					if (*methodname!=GDScriptLanguage::get_singleton()->strings._init) {
						err.error=Variant::CallError::CALL_ERROR_INVALID_METHOD;
					} else {
						err.error=Variant::CallError::CALL_OK;
					}
				}


				if (err.error!=Variant::CallError::CALL_OK) {


					String methodstr = *methodname;
					err_text=_get_call_error(err,"function '"+methodstr+"'",(const Variant**)argptrs);

					break;
				}

				ip+=4+argc;
				continue;
			}
			case OPCODE_YIELD:
			case OPCODE_YIELD_SIGNAL: {

				int ipofs=1;
				if (_code_ptr[ip]==OPCODE_YIELD_SIGNAL) {
					CHECK_SPACE(4);
					ipofs+=2;
				} else {
					CHECK_SPACE(2);

				}

				Ref<GDFunctionState> gdfs = memnew( GDFunctionState );
				gdfs->function=this;

				gdfs->state.stack.resize(alloca_size);
				//copy variant stack
				for(int i=0;i<_stack_size;i++) {
					memnew_placement(&gdfs->state.stack[sizeof(Variant)*i],Variant(stack[i]));
				}
				gdfs->state.stack_size=_stack_size;
				gdfs->state.self=self;
				gdfs->state.alloca_size=alloca_size;
				gdfs->state._class=_class;
				gdfs->state.ip=ip+ipofs;
				gdfs->state.line=line;
				gdfs->state.instance_id=(p_instance && p_instance->get_owner())?p_instance->get_owner()->get_instance_ID():0;
				gdfs->state.script_id=_class->get_instance_ID();
				//gdfs->state.result_pos=ip+ipofs-1;
				gdfs->state.defarg=defarg;
				gdfs->state.instance=p_instance;
				gdfs->function=this;

				retvalue=gdfs;

				if (_code_ptr[ip]==OPCODE_YIELD_SIGNAL) {
					GET_VARIANT_PTR(argobj,1);
					GET_VARIANT_PTR(argname,2);
					//do the oneshot connect

					if (argobj->get_type()!=Variant::OBJECT) {
						err_text="First argument of yield() not of type object.";
						break;
					}
					if (argname->get_type()!=Variant::STRING) {
						err_text="Second argument of yield() not a string (for signal name).";
						break;
					}
					Object *obj=argobj->operator Object *();
					String signal = argname->operator String();
#ifdef DEBUG_ENABLED

					if (!obj) {
						err_text="First argument of yield() is null.";
						break;
					}
					if (ScriptDebugger::get_singleton()) {
						if (!ObjectDB::instance_validate(obj)) {
							err_text="First argument of yield() is a previously freed instance.";
							break;
						}
					}
					if (signal.length()==0) {

						err_text="Second argument of yield() is an empty string (for signal name).";
						break;
					}

#endif
					Error err = obj->connect(signal,gdfs.ptr(),"_signal_callback",varray(gdfs),Object::CONNECT_ONESHOT);
					if (err!=OK) {
						err_text="Error connecting to signal: "+signal+" during yield().";
						break;
					}


				}

				exit_ok=true;
				break;
			}
			case OPCODE_YIELD_RESUME: {

				CHECK_SPACE(2);
				if (!p_state) {
					err_text=("Invalid Resume (bug?)");
					break;
				}
				GET_VARIANT_PTR(result,1);
				*result=p_state->result;
				ip+=2;
				continue;
			}
			case OPCODE_JUMP: {

				CHECK_SPACE(2);
				int to = _code_ptr[ip+1];

				ERR_BREAK(to<0 || to>_code_size);
				ip=to;
				continue;
			}
			case OPCODE_JUMP_IF: {

				CHECK_SPACE(3);

				GET_VARIANT_PTR(test,1);

				bool valid;
				bool result = test->booleanize(valid);
#ifdef DEBUG_ENABLED
				if (!valid) {

					err_text="cannot evaluate conditional expression of type: "+Variant::get_type_name(test->get_type());
					break;
				}
#endif
				if (result) {
					int to = _code_ptr[ip+2];
					ERR_BREAK(to<0 || to>_code_size);
					ip=to;
					continue;
				}
				ip+=3;
				continue;
			}
			case OPCODE_JUMP_IF_NOT: {

				CHECK_SPACE(3);

				GET_VARIANT_PTR(test,1);

				bool valid;
				bool result = test->booleanize(valid);
#ifdef DEBUG_ENABLED
				if (!valid) {

					err_text="cannot evaluate conditional expression of type: "+Variant::get_type_name(test->get_type());
					break;
				}
#endif
				if (!result) {
					int to = _code_ptr[ip+2];
					ERR_BREAK(to<0 || to>_code_size);
					ip=to;
					continue;
				}
				ip+=3;
				continue;
			}
			case OPCODE_JUMP_TO_DEF_ARGUMENT: {

				CHECK_SPACE(2);
				ip=_default_arg_ptr[defarg];
				continue;
			}
			case OPCODE_RETURN: {

				CHECK_SPACE(2);
				GET_VARIANT_PTR(r,1);
				retvalue=*r;
				exit_ok=true;
				break;
			}
			case OPCODE_ITERATE_BEGIN: {

				CHECK_SPACE(8); //space for this an regular iterate

				GET_VARIANT_PTR(counter,1);
				GET_VARIANT_PTR(container,2);

				bool valid;
				if (!container->iter_init(*counter,valid)) {
					if (!valid) {
						err_text="Unable to iterate on object of type  "+Variant::get_type_name(container->get_type())+"'.";
						break;
					}
					int jumpto=_code_ptr[ip+3];
					ERR_BREAK(jumpto<0 || jumpto>_code_size);
					ip=jumpto;
					continue;
				}
				GET_VARIANT_PTR(iterator,4);


				*iterator=container->iter_get(*counter,valid);
				if (!valid) {
					err_text="Unable to obtain iterator object of type  "+Variant::get_type_name(container->get_type())+"'.";
					break;
				}


				ip+=5; //skip regular iterate which is always next
				continue;
			}
			case OPCODE_ITERATE: {

				CHECK_SPACE(4);

				GET_VARIANT_PTR(counter,1);
				GET_VARIANT_PTR(container,2);

				bool valid;
				if (!container->iter_next(*counter,valid)) {
					if (!valid) {
						err_text="Unable to iterate on object of type  "+Variant::get_type_name(container->get_type())+"' (type changed since first iteration?).";
						break;
					}
					int jumpto=_code_ptr[ip+3];
					ERR_BREAK(jumpto<0 || jumpto>_code_size);
					ip=jumpto;
					continue;
				}
				GET_VARIANT_PTR(iterator,4);

				*iterator=container->iter_get(*counter,valid);
				if (!valid) {
					err_text="Unable to obtain iterator object of type  "+Variant::get_type_name(container->get_type())+"' (but was obtained on first iteration?).";
					break;
				}

				ip+=5; //loop again
				continue;
			}
			case OPCODE_ASSERT: {
				CHECK_SPACE(2);
				GET_VARIANT_PTR(test,1);

#ifdef DEBUG_ENABLED
				bool valid;
				bool result = test->booleanize(valid);


				if (!valid) {

					err_text="cannot evaluate conditional expression of type: "+Variant::get_type_name(test->get_type());
					break;
				}


				if (!result) {

					err_text="Assertion failed.";
					break;
				}

#endif

				ip+=2;
				continue;
			}
			case OPCODE_BREAKPOINT: {
#ifdef DEBUG_ENABLED
				if (ScriptDebugger::get_singleton()) {
					GDScriptLanguage::get_singleton()->debug_break("Breakpoint Statement",true);
				}
#endif
				ip+=1;
				continue;
			}
			case OPCODE_LINE: {
				CHECK_SPACE(2);

				line=_code_ptr[ip+1];
				ip+=2;

				if (ScriptDebugger::get_singleton()) {
					// line
					bool do_break=false;

					if (ScriptDebugger::get_singleton()->get_lines_left()>0) {

						if (ScriptDebugger::get_singleton()->get_depth()<=0)
							ScriptDebugger::get_singleton()->set_lines_left( ScriptDebugger::get_singleton()->get_lines_left() -1 );
						if (ScriptDebugger::get_singleton()->get_lines_left()<=0)
							do_break=true;
					}

					if (ScriptDebugger::get_singleton()->is_breakpoint(line,source))
						do_break=true;

					if (do_break) {
						GDScriptLanguage::get_singleton()->debug_break("Breakpoint",true);
					}

					ScriptDebugger::get_singleton()->line_poll();

				}
				continue;
			}
			case OPCODE_END: {

				exit_ok=true;
				break;

			}
			default: {

				err_text="Illegal opcode "+itos(_code_ptr[ip])+" at address "+itos(ip);
				break;
			}

		}

		if (exit_ok)
			break;
		//error
		// function, file, line, error, explanation
		String err_file;
		if (p_instance)
			err_file=p_instance->script->path;
		else if (_class)
			err_file=_class->path;
		if (err_file=="")
			err_file="<built-in>";
		String err_func = name;
		if (p_instance && p_instance->script->name!="")
			err_func=p_instance->script->name+"."+err_func;
		int err_line=line;
		if (err_text=="") {
			err_text="Internal Script Error! - opcode #"+itos(last_opcode)+" (report please).";
		}

		if (!GDScriptLanguage::get_singleton()->debug_break(err_text,false)) {
			// debugger break did not happen

			_err_print_error(err_func.utf8().get_data(),err_file.utf8().get_data(),err_line,err_text.utf8().get_data(),ERR_HANDLER_SCRIPT);
		}


		break;
	}

#ifdef DEBUG_ENABLED
	if (GDScriptLanguage::get_singleton()->profiling) {
		uint64_t time_taken = OS::get_singleton()->get_ticks_usec() - function_start_time;
		profile.total_time+=time_taken;
		profile.self_time+=time_taken-function_call_time;
		profile.frame_total_time+=time_taken;
		profile.frame_self_time+=time_taken-function_call_time;
		GDScriptLanguage::get_singleton()->script_frame_time+=time_taken-function_call_time;

	}

#endif
	if (ScriptDebugger::get_singleton())
		GDScriptLanguage::get_singleton()->exit_function();


	if (_stack_size) {
		//free stack
		for(int i=0;i<_stack_size;i++)
			stack[i].~Variant();
	}

	return retvalue;

}

const int* GDFunction::get_code() const {

	return _code_ptr;
}
int GDFunction::get_code_size() const{

	return _code_size;
}

Variant GDFunction::get_constant(int p_idx) const {

	ERR_FAIL_INDEX_V(p_idx,constants.size(),"<errconst>");
	return constants[p_idx];
}

StringName GDFunction::get_global_name(int p_idx) const {

	ERR_FAIL_INDEX_V(p_idx,global_names.size(),"<errgname>");
	return global_names[p_idx];
}

int GDFunction::get_default_argument_count() const {

	return default_arguments.size();
}
int GDFunction::get_default_argument_addr(int p_arg) const{

	ERR_FAIL_INDEX_V(p_arg,default_arguments.size(),-1);
	return default_arguments[p_arg];
}


StringName GDFunction::get_name() const {

	return name;
}

int GDFunction::get_max_stack_size() const {

	return _stack_size;
}

struct _GDFKC {

	int order;
	List<int> pos;
};

struct _GDFKCS {

	int order;
	StringName id;
	int pos;

	bool operator<(const _GDFKCS &p_r) const {

		return order<p_r.order;
	}
};

void GDFunction::debug_get_stack_member_state(int p_line,List<Pair<StringName,int> > *r_stackvars) const {


	int oc=0;
	Map<StringName,_GDFKC> sdmap;
	for( const List<StackDebug>::Element *E=stack_debug.front();E;E=E->next()) {

		const StackDebug &sd=E->get();
		if (sd.line>p_line)
			break;

		if (sd.added) {

			if (!sdmap.has(sd.identifier)) {
				_GDFKC d;
				d.order=oc++;
				d.pos.push_back(sd.pos);
				sdmap[sd.identifier]=d;

			} else {
				sdmap[sd.identifier].pos.push_back(sd.pos);
			}
		} else {


			ERR_CONTINUE(!sdmap.has(sd.identifier));

			sdmap[sd.identifier].pos.pop_back();
			if (sdmap[sd.identifier].pos.empty())
				sdmap.erase(sd.identifier);
		}

	}


	List<_GDFKCS> stackpositions;
	for(Map<StringName,_GDFKC>::Element *E=sdmap.front();E;E=E->next() ) {

		_GDFKCS spp;
		spp.id=E->key();
		spp.order=E->get().order;
		spp.pos=E->get().pos.back()->get();
		stackpositions.push_back(spp);
	}

	stackpositions.sort();

	for(List<_GDFKCS>::Element *E=stackpositions.front();E;E=E->next()) {

		Pair<StringName,int> p;
		p.first=E->get().id;
		p.second=E->get().pos;
		r_stackvars->push_back(p);
	}


}

#if 0
void GDFunction::clear() {

	name=StringName();
	constants.clear();
	_stack_size=0;
	code.clear();
	_constants_ptr=NULL;
	_constant_count=0;
	_global_names_ptr=NULL;
	_global_names_count=0;
	_code_ptr=NULL;
	_code_size=0;

}
#endif
GDFunction::GDFunction() : function_list(this) {

	_stack_size=0;
	_call_size=0;
	rpc_mode=ScriptInstance::RPC_MODE_DISABLED;
	name="<anonymous>";
#ifdef DEBUG_ENABLED
	_func_cname=NULL;

	if (GDScriptLanguage::get_singleton()->lock) {
		GDScriptLanguage::get_singleton()->lock->lock();
	}
	GDScriptLanguage::get_singleton()->function_list.add(&function_list);

	if (GDScriptLanguage::get_singleton()->lock) {
		GDScriptLanguage::get_singleton()->lock->unlock();
	}

	profile.call_count=0;
	profile.self_time=0;
	profile.total_time=0;
	profile.frame_call_count=0;
	profile.frame_self_time=0;
	profile.frame_total_time=0;
	profile.last_frame_call_count=0;
	profile.last_frame_self_time=0;
	profile.last_frame_total_time=0;

#endif
}

GDFunction::~GDFunction()  {
#ifdef DEBUG_ENABLED
	if (GDScriptLanguage::get_singleton()->lock) {
		GDScriptLanguage::get_singleton()->lock->lock();
	}
	GDScriptLanguage::get_singleton()->function_list.remove(&function_list);

	if (GDScriptLanguage::get_singleton()->lock) {
		GDScriptLanguage::get_singleton()->lock->unlock();
	}
#endif
}

/////////////////////


Variant GDFunctionState::_signal_callback(const Variant** p_args, int p_argcount, Variant::CallError& r_error) {

#ifdef DEBUG_ENABLED
	if (state.instance_id && !ObjectDB::get_instance(state.instance_id)) {
		ERR_EXPLAIN("Resumed after yield, but class instance is gone");
		ERR_FAIL_V(Variant());
	}

	if (state.script_id && !ObjectDB::get_instance(state.script_id)) {
		ERR_EXPLAIN("Resumed after yield, but script is gone");
		ERR_FAIL_V(Variant());
	}
#endif

	Variant arg;
	r_error.error=Variant::CallError::CALL_OK;

	ERR_FAIL_COND_V(!function,Variant());

	if (p_argcount==0) {
		r_error.error=Variant::CallError::CALL_ERROR_TOO_FEW_ARGUMENTS;
		r_error.argument=1;
		return Variant();
	} else if (p_argcount==1) {
		//noooneee
	} else if (p_argcount==2) {
		arg=*p_args[0];
	} else {
		Array extra_args;
		for(int i=0;i<p_argcount-1;i++) {
			extra_args.push_back(*p_args[i]);
		}
		arg=extra_args;
	}

	Ref<GDFunctionState> self = *p_args[p_argcount-1];

	if (self.is_null()) {
		r_error.error=Variant::CallError::CALL_ERROR_INVALID_ARGUMENT;
		r_error.argument=p_argcount-1;
		r_error.expected=Variant::OBJECT;
		return Variant();
	}

	state.result=arg;
	Variant ret = function->call(NULL,NULL,0,r_error,&state);
	function=NULL; //cleaned up;
	state.result=Variant();
	return ret;
}


bool GDFunctionState::is_valid() const {

	return function!=NULL;
}

Variant GDFunctionState::resume(const Variant& p_arg) {

	ERR_FAIL_COND_V(!function,Variant());
#ifdef DEBUG_ENABLED
	if (state.instance_id && !ObjectDB::get_instance(state.instance_id)) {
		ERR_EXPLAIN("Resumed after yield, but class instance is gone");
		ERR_FAIL_V(Variant());
	}

	if (state.script_id && !ObjectDB::get_instance(state.script_id)) {
		ERR_EXPLAIN("Resumed after yield, but script is gone");
		ERR_FAIL_V(Variant());
	}
#endif

	state.result=p_arg;
	Variant::CallError err;
	Variant ret = function->call(NULL,NULL,0,err,&state);
	function=NULL; //cleaned up;
	state.result=Variant();
	return ret;
}


void GDFunctionState::_bind_methods() {

	ClassDB::bind_method(_MD("resume:Variant","arg"),&GDFunctionState::resume,DEFVAL(Variant()));
	ClassDB::bind_method(_MD("is_valid"),&GDFunctionState::is_valid);
	ClassDB::bind_vararg_method(METHOD_FLAGS_DEFAULT,"_signal_callback",&GDFunctionState::_signal_callback,MethodInfo("_signal_callback"));

}

GDFunctionState::GDFunctionState() {

	function=NULL;
}

GDFunctionState::~GDFunctionState() {

	if (function!=NULL) {
		//never called, deinitialize stack
		for(int i=0;i<state.stack_size;i++) {
			Variant *v=(Variant*)&state.stack[sizeof(Variant)*i];
			v->~Variant();
		}
	}
}

