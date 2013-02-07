#undef _get_member_def
#undef _get_member_impl

#define _get_member_def(type, name) public: type get##name() const;
#define _get_member_impl(cls, type, name) type cls::get##name() const\
{\
   return name;\
}

#undef _set_member
#undef _set_member_def

#define _set_member_def(type, name) public: params &set##name(type t);
#define _set_member_impl(cls, type, name) cls & cls::set##name(type t)\
{\
   name = t;\
   return *this;\
} 

#undef _set_const_ref_member_def
#undef _set_const_ref_member_impl

#define _set_const_ref_member_def(type, name) public: params &set##name(const type &t);
#define _set_const_ref_member_impl(cls, type, name) cls & cls::set##name(const type &t)\
{\
   name = t.clone();\
   return *this;\
}

#undef _member_def
#define _member_def(type, name) private: type name;

#undef param_member
#define param_member(type, name) _member_def(type, name) _set_member_def(type, name) _get_member_def(type, name)

#undef param_member_impl
#define param_member_impl(cls, type, name) _set_member_impl(cls, type, name) _get_member_impl(cls, type, name)

#undef param_ref_member
#define param_ref_member(type, name) _member_def(type *, name)  _set_const_ref_member_def(type, name) _get_member_def(type *, name)

#undef param_ref_member_impl
#define param_ref_member_impl(cls, type, name) _set_const_ref_member_impl(cls, type, name) _get_member_impl(cls, type *, name)


