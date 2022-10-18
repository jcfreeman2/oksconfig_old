#include <jni.h>

#include <config/ConfigObject.h>
#include <oksconfig/OksConfigObject.h>

#include "oksconfig_utils.h"


/*
 * Class:     oksconfig_OksConfigObject
 * Method:    destroy
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_oksconfig_OksConfigObject_destroy
(JNIEnv * env, jobject obj)
{
  if (jfieldID fid = env->GetFieldID(env->GetObjectClass(obj), "ptr", "J")) {
    OksConfigObject * o = reinterpret_cast<OksConfigObject *>(env->GetLongField(obj, fid));
    delete o;
    env->SetLongField(obj, fid, 0);
  }
  else {
    throw_jexception(env, "config/SystemException", "JNI GetFieldID() failed", "OksConfigObject::destroy()");
  }
}

/*
 * Class:     oksconfig_OksConfigObject
 * Method:    get_UID
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_oksconfig_OksConfigObject_get_1UID
(JNIEnv * env, jobject obj)
{
  if(OksConfigObject * o = get_oks_object(env, obj)) {
    return env->NewStringUTF(o->UID().c_str());
  }
  else {
    throw_jexception(env, "config/SystemException", "joksconfig.so::get_oks_object() failed", "OksConfigObject::get_UID()");
    return 0; // if everething will failed, then give up
  }
}

/*
 * Class:     oksconfig_OksConfigObject
 * Method:    get_class_name
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_oksconfig_OksConfigObject_get_1class_1name
(JNIEnv * env, jobject obj)
{
  if(OksConfigObject * o = get_oks_object(env, obj)) {
    return env->NewStringUTF(o->class_name().c_str());
  }
  else {
    throw_jexception(env, "config/SystemException", "joksconfig.so::get_oks_object() failed", "OksConfigObject::get_class_name()");
    return 0; // if everething will failed, then give up
  }
}

std::string
make_error_txt(OksConfigObject * o, const char * name, const char * reason = 0)
{
  std::string msg("Can not get value of \'");
  msg += name;
  msg += "\' for object \'";
  msg += o->UID();
  msg += '@';
  msg += o->class_name();
  msg += '\'';
  
  if(reason) {
    msg += '\n';
    msg += reason;
  }

  return msg;
}

template<class T> T get_value(JNIEnv * env, jobject obj, jstring name_jstr)
{
  JString name(env, name_jstr);
  if(OksConfigObject * o = get_oks_object(env, obj)) {
    try {
      T value;
      o->get(name.c_str(), value);
      return value;
    }
    catch (daq::config::Exception & ex) {
      std::string msg = make_error_txt(o, name.c_str(), ex.what());
      throw_jexception(env, "config/NotFoundException", msg.c_str(), "OksConfigObject::get<T>");
      return 0; // if everething will failed, then give up
    }
  }

  throw_jexception(env, "config/SystemException", "joksconfig.so::get_oks_object() failed", "OksConfigObject::get<T>");
  return 0; // if everething will failed, then give up
}


/*
 * Class:     oksconfig_OksConfigObject
 * Method:    get_bool
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_oksconfig_OksConfigObject_get_1bool
(JNIEnv * env, jobject obj, jstring name_jstr)
{
  return get_value<bool>(env, obj, name_jstr);
}

/*
 * Class:     oksconfig_OksConfigObject
 * Method:    get_char
 * Signature: (Ljava/lang/String;)C
 */
JNIEXPORT jchar JNICALL Java_oksconfig_OksConfigObject_get_1char
(JNIEnv * env, jobject obj, jstring name_jstr)
{
  return get_value<int8_t>(env, obj, name_jstr);
}

/*
 * Class:     oksconfig_OksConfigObject
 * Method:    get_byte
 * Signature: (Ljava/lang/String;)B
 */
JNIEXPORT jbyte JNICALL Java_oksconfig_OksConfigObject_get_1byte
(JNIEnv * env, jobject obj, jstring name_jstr)
{
  return get_value<uint8_t>(env, obj, name_jstr);
}

/*
 * Class:     oksconfig_OksConfigObject
 * Method:    get_short
 * Signature: (Ljava/lang/String;)S
 */
JNIEXPORT jshort JNICALL Java_oksconfig_OksConfigObject_get_1short
(JNIEnv * env, jobject obj, jstring name_jstr)
{
  return get_value<int16_t>(env, obj, name_jstr);
}

/*
 * Class:     oksconfig_OksConfigObject
 * Method:    get_int
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_oksconfig_OksConfigObject_get_1int
(JNIEnv * env, jobject obj, jstring name_jstr)
{
  return get_value<uint32_t>(env, obj, name_jstr);
}

/*
 * Class:     oksconfig_OksConfigObject
 * Method:    get_long
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_oksconfig_OksConfigObject_get_1long
  (JNIEnv * env, jobject obj, jstring name_jstr)
{
  return get_value<uint64_t>(env, obj, name_jstr);
}

/*
 * Class:     oksconfig_OksConfigObject
 * Method:    get_float
 * Signature: (Ljava/lang/String;)F
 */
JNIEXPORT jfloat JNICALL Java_oksconfig_OksConfigObject_get_1float
(JNIEnv * env, jobject obj, jstring name_jstr)
{
  return get_value<float>(env, obj, name_jstr);
}

/*
 * Class:     oksconfig_OksConfigObject
 * Method:    get_double
 * Signature: (Ljava/lang/String;)D
 */
JNIEXPORT jdouble JNICALL Java_oksconfig_OksConfigObject_get_1double
(JNIEnv * env, jobject obj, jstring name_jstr)
{
  return get_value<double>(env, obj, name_jstr);
}

/*
 * Class:     oksconfig_OksConfigObject
 * Method:    get_string
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_oksconfig_OksConfigObject_get_1string
(JNIEnv * env, jobject obj, jstring name_jstr)
{
  std::string s = get_value<std::string>(env, obj, name_jstr);
  return env->NewStringUTF(s.c_str());
}


#define GetValues(CppType, JavaType, JArrayType, Constructor, SetRegion)	\
  if(OksConfigObject * o = get_oks_object(env, obj)) {				\
    JString name(env,name_jstr);						\
    try {						                        \
      std::vector<CppType> value;						\
      o->get(name.c_str(), value);					        \
      JArrayType rarray = env->Constructor(value.size());			\
      env->SetRegion(rarray, 0, value.size(), (JavaType *)&value[0]);		\
      return rarray;								\
    }										\
    catch(daq::config::Exception & ex) {					\
      std::string msg = make_error_txt(o, name.c_str(), ex.what());		\
      throw_jexception(env, "config/NotFoundException", msg.c_str(),            \
                       "OksConfigObject::get_T()");                             \
      return 0;									\
    }										\
  }										\
  else {									\
    throw_jexception(env, "config/SystemException",                             \
                     "joksconfig.so::get_oks_object() failed",                  \
		     "OksConfigObject::get_T()");                               \
    return 0;									\
  }


/*
 * Class:     oksconfig_OksConfigObject
 * Method:    get_bools
 * Signature: (Ljava/lang/String;)[Z
 */
JNIEXPORT jbooleanArray JNICALL Java_oksconfig_OksConfigObject_get_1bools
(JNIEnv * env, jobject obj, jstring name_jstr)
{
  JString name(env, name_jstr);
  if(OksConfigObject * o = get_oks_object(env, obj)) {
    try {
      std::vector<bool> value;
      o->get(name.c_str(), value);
      jbooleanArray rarray = env->NewBooleanArray(value.size());
      jboolean * barray = new jboolean [value.size()];
      for(unsigned int i = 0; i < value.size(); ++i) {
        barray[i] = value[i]; 
      }
      env->SetBooleanArrayRegion(rarray, 0, value.size(), barray);
      return rarray;
    }
    catch(daq::config::Exception & ex) {
      std::string msg = make_error_txt(o, name.c_str(), ex.what());
      throw_jexception(env, "config/NotFoundException", msg.c_str(), "OksConfigObject::get_bools()");
      return 0; // if everething will failed, then give up
    }
  }
  else {
    throw_jexception(env, "config/SystemException", "joksconfig.so::get_oks_object() failed", "OksConfigObject::get_bools()");
    return 0; // if everething will failed, then give up
  }
}

/*
 * Class:     oksconfig_OksConfigObject
 * Method:    get_chars
 * Signature: (Ljava/lang/String;)[C
 */
JNIEXPORT jcharArray JNICALL Java_oksconfig_OksConfigObject_get_1chars
(JNIEnv * env, jobject obj, jstring name_jstr)
{
  GetValues(int8_t, jchar, jcharArray, NewCharArray, SetCharArrayRegion)
}

/*
 * Class:     oksconfig_OksConfigObject
 * Method:    get_bytes
 * Signature: (Ljava/lang/String;)[B
 */
JNIEXPORT jbyteArray JNICALL Java_oksconfig_OksConfigObject_get_1bytes
(JNIEnv * env, jobject obj, jstring name_jstr)
{
  GetValues(uint8_t, jbyte, jbyteArray, NewByteArray, SetByteArrayRegion)
}

/*
 * Class:     oksconfig_OksConfigObject
 * Method:    get_shorts
 * Signature: (Ljava/lang/String;)[S
 */
JNIEXPORT jshortArray JNICALL Java_oksconfig_OksConfigObject_get_1shorts
(JNIEnv * env, jobject obj, jstring name_jstr)
{
  GetValues(uint16_t, jshort, jshortArray, NewShortArray, SetShortArrayRegion)
}

/*
 * Class:     oksconfig_OksConfigObject
 * Method:    get_ints
 * Signature: (Ljava/lang/String;)[I
 */
JNIEXPORT jintArray JNICALL Java_oksconfig_OksConfigObject_get_1ints
(JNIEnv * env, jobject obj, jstring name_jstr)
{
  GetValues(uint32_t, jint, jintArray, NewIntArray, SetIntArrayRegion)
}

/*
 * Class:     oksconfig_OksConfigObject
 * Method:    get_longs
 * Signature: (Ljava/lang/String;)[J
 */
JNIEXPORT jlongArray JNICALL Java_oksconfig_OksConfigObject_get_1longs
  (JNIEnv * env, jobject obj, jstring name_jstr)
{
  GetValues(uint64_t, jlong, jlongArray, NewLongArray, SetLongArrayRegion)
}

/*
 * Class:     oksconfig_OksConfigObject
 * Method:    get_floats
 * Signature: (Ljava/lang/String;)[F
 */
JNIEXPORT jfloatArray JNICALL Java_oksconfig_OksConfigObject_get_1floats
(JNIEnv * env, jobject obj, jstring name_jstr)
{
  GetValues(float, jfloat, jfloatArray, NewFloatArray, SetFloatArrayRegion)
}

/*
 * Class:     oksconfig_OksConfigObject
 * Method:    get_doubles
 * Signature: (Ljava/lang/String;)[D
 */
JNIEXPORT jdoubleArray JNICALL Java_oksconfig_OksConfigObject_get_1doubles
(JNIEnv * env, jobject obj, jstring name_jstr)
{
  GetValues(double, jdouble, jdoubleArray, NewDoubleArray, SetDoubleArrayRegion)
}

/*
 * Class:     oksconfig_OksConfigObject
 * Method:    get_strings
 * Signature: (Ljava/lang/String;)[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_oksconfig_OksConfigObject_get_1strings
(JNIEnv * env, jobject obj, jstring name_jstr)
{
  JString name(env, name_jstr);
  if(OksConfigObject * o = get_oks_object(env, obj)) {
    try {
      std::vector<std::string> value;
      o->get(name.c_str(), value);
      jobjectArray rarray = (jobjectArray)env->NewObjectArray(
        value.size(),
        get_jclass(env, "java/lang/String", "oksconfig_OksConfigObject::get_strings()"),
        env->NewStringUTF("")
      );

      for(unsigned int i = 0; i < value.size(); ++i) {
        env->SetObjectArrayElement(rarray, i, env->NewStringUTF(value[i].c_str())) ;
      }

      return rarray;
    }
    catch(daq::config::Exception & ex) {
      std::string msg = make_error_txt(o, name.c_str(), ex.what());
      throw_jexception(env, "config/NotFoundException", msg.c_str(), "OksConfigObject::get_strings()");
      return 0; // if everething will failed, then give up
    }
  }
  else {
    throw_jexception(env, "config/SystemException", "joksconfig.so::get_oks_object() failed", "OksConfigObject::get_strings()");
    return 0; // if everething fails, then give up
  }
}

/*
 * Class:     oksconfig_OksConfigObject
 * Method:    get_impl_object
 * Signature: (Ljava/lang/String;)Loksconfig/OksConfigObject;
 */
JNIEXPORT jobject JNICALL Java_oksconfig_OksConfigObject_get_1impl_1object
(JNIEnv * env, jobject obj, jstring name_jstr)
{
  JString name(env,name_jstr);
  if(OksConfigObject * o = get_oks_object(env, obj)) {
    try {
      ConfigObject obj;
      o->get(name.c_str(), obj);
      if(obj.is_null()) {
        return 0;
      }
      else {
        return create_oks_object(env, get_impl_object(obj), "oksconfig_OksConfigObject::get_impl_object()");
      }
    }
    catch(daq::config::Exception & ex) {
      std::string msg = make_error_txt(o, name.c_str(), ex.what());
      throw_jexception(env, "config/NotFoundException", msg.c_str(), "OksConfigObject::get_impl_object()");
      return 0; // if everething fails, then give up
    }
  }
  else {
    throw_jexception(env, "config/SystemException", "joksconfig.so::get_oks_object() failed", "OksConfigObject::get_impl_object()");
    return 0; // if everething fails, then give up
  }
}

/*
 * Class:     oksconfig_OksConfigObject
 * Method:    get_referenced_by_impl_objects
 * Signature: (Ljava/lang/String;Z)[Loksconfig/OksConfigObject;
 */
JNIEXPORT jobjectArray JNICALL Java_oksconfig_OksConfigObject_get_1referenced_1by_1impl_1objects
  (JNIEnv * env, jobject obj, jstring rel_name_jstr, jboolean check_composite_only_jbool)
{
  const char * fname = "oksconfig_OksConfigObject::get_referenced_by_impl_objects()";
  const char * jname = "oksconfig/OksConfigObject";

  if(OksConfigObject * o = get_oks_object(env, obj)) {
    JString rel_name(env,rel_name_jstr);

    try {
      std::vector<ConfigObject> objects;

      o->referenced_by(objects, rel_name.c_str(), static_cast<bool>(check_composite_only_jbool), 0, nullptr);

      jclass oks_conf_obj_class = get_jclass(env, jname, fname);
      if(!oks_conf_obj_class) { return 0 ; }

      jmethodID oks_conf_obj_init = get_jclass_constructor(env, oks_conf_obj_class, jname, fname);
      if(!oks_conf_obj_init) { return 0 ; }

      jobject dummu_jobj = create_jobject(env, oks_conf_obj_class, oks_conf_obj_init, 0, jname, fname);
      if(!dummu_jobj) { return 0 ; }

      jobjectArray objs = (jobjectArray)env->NewObjectArray(objects.size(), oks_conf_obj_class, dummu_jobj) ;

      unsigned int count  = 0;
      for(std::vector<ConfigObject>::iterator i = objects.begin(); i != objects.end(); ++i, ++count) {
        if(jobject o = create_jobject(env, oks_conf_obj_class, oks_conf_obj_init, get_impl_object(*i), jname, fname)) {
          env->SetObjectArrayElement(objs, count, o) ;
        }
        else {
          throw_jexception(env, "config/SystemException", "joksconfig.so::create_jobject() failed", "OksConfigObject::get_impl_objects()");
          return 0; // if everything will failed, then give up
        }
      }

      return objs;
    }
    catch(daq::config::Exception & ex)  {
      throw_jexception(env, "config/SystemException", "joksconfig.so::referenced_by()", "OksConfigObject::get_referenced_by_impl_objects()");
      return 0; // if everything will failed, then give up
    }
  }
  else {
    throw_jexception(env, "config/SystemException", "joksconfig.so::get_oks_object() failed", "OksConfigObject::get_referenced_by_impl_objects()");
    return 0; // if everething will failed, then give up
  }
}









/*
 * Class:     oksconfig_OksConfigObject
 * Method:    get_impl_objects
 * Signature: (Ljava/lang/String;)[Loksconfig/OksConfigObject;
 */
JNIEXPORT jobjectArray JNICALL Java_oksconfig_OksConfigObject_get_1impl_1objects
(JNIEnv * env, jobject obj, jstring name_jstr)
{
  const char * fname = "oksconfig_OksConfigObject::get_impl_objects()";
  const char * jname = "oksconfig/OksConfigObject";

  if(OksConfigObject * o = get_oks_object(env, obj)) {
    JString name(env,name_jstr);

    try {
      std::vector<ConfigObject> objects;

      o->get(name.c_str(), objects);

      jclass oks_conf_obj_class = get_jclass(env, jname, fname);
      if(!oks_conf_obj_class) { return 0 ; }

      jmethodID oks_conf_obj_init = get_jclass_constructor(env, oks_conf_obj_class, jname, fname);
      if(!oks_conf_obj_init) { return 0 ; }

      jobject dummu_jobj = create_jobject(env, oks_conf_obj_class, oks_conf_obj_init, 0, jname, fname);
      if(!dummu_jobj) { return 0 ; }

      jobjectArray objs = (jobjectArray)env->NewObjectArray(objects.size(), oks_conf_obj_class, dummu_jobj) ;

      unsigned int count  = 0;
      for(std::vector<ConfigObject>::iterator i = objects.begin(); i != objects.end(); ++i, ++count) {
        if(jobject o = create_jobject(env, oks_conf_obj_class, oks_conf_obj_init, get_impl_object(*i), jname, fname)) {
          env->SetObjectArrayElement(objs, count, o) ;
	}
        else {
          throw_jexception(env, "config/SystemException", "joksconfig.so::create_jobject() failed", "OksConfigObject::get_impl_objects()");
          return 0; // if everething will failed, then give up
        }
      }

      return objs;
    }
    catch(daq::config::Exception & ex)  {
      std::string msg = make_error_txt(o, name.c_str(), ex.what());
      throw_jexception(env, "config/NotFoundException", msg.c_str(), "OksConfigObject::get_impl_objects()");
      return 0; // if everething will failed, then give up
    }
  }
  else {
    throw_jexception(env, "config/SystemException", "joksconfig.so::get_oks_object() failed", "OksConfigObject::get_impl_objects()");
    return 0; // if everething will failed, then give up
  }
}

template<class T> void set_value(JNIEnv * env, jobject obj, jstring name_jstr, T value)
{
  JString name(env, name_jstr);
  if(OksConfigObject * o = get_oks_object(env, obj)) {
    try {
      o->set(name.c_str(), value);
    }
    catch (daq::config::Generic & ex) {
      std::string msg = make_error_txt(o, name.c_str(), ex.what());
      throw_jexception(env, "config/SystemException", msg.c_str(), "OksConfigObject::set<T>()");
    }
  }
  else {
    throw_jexception(env, "config/SystemException", "joksconfig.so::get_oks_object() failed", "OksConfigObject::set<T>()");
  }
}


/*
 * Class:     oksconfig_OksConfigObject
 * Method:    set_bool
 * Signature: (Ljava/lang/String;Z)V
 */
JNIEXPORT void JNICALL Java_oksconfig_OksConfigObject_set_1bool
(JNIEnv * env, jobject obj, jstring name_jstr, jboolean value)
{
  set_value(env, obj, name_jstr, static_cast<bool>(value));
}

/*
 * Class:     oksconfig_OksConfigObject
 * Method:    set_char
 * Signature: (Ljava/lang/String;C)V
 */
JNIEXPORT void JNICALL Java_oksconfig_OksConfigObject_set_1char
(JNIEnv * env, jobject obj, jstring name_jstr, jchar value)
{
  set_value(env, obj, name_jstr, static_cast<int8_t>(value));
}

/*
 * Class:     oksconfig_OksConfigObject
 * Method:    set_byte
 * Signature: (Ljava/lang/String;B)V
 */
JNIEXPORT void JNICALL Java_oksconfig_OksConfigObject_set_1byte
(JNIEnv * env, jobject obj, jstring name_jstr, jbyte value)
{
  set_value(env, obj, name_jstr, static_cast<uint8_t>(value));
}

/*
 * Class:     oksconfig_OksConfigObject
 * Method:    set_short
 * Signature: (Ljava/lang/String;S)V
 */
JNIEXPORT void JNICALL Java_oksconfig_OksConfigObject_set_1short
(JNIEnv * env, jobject obj, jstring name_jstr, jshort value)
{
  JString name(env, name_jstr);

  if(OksConfigObject * o = get_oks_object(env, obj)) {
    try {
      if(o->get_type(name.c_str()) == OksData::s16_int_type) {
        o->set(name.c_str(), static_cast<int16_t>(value));
      }
      else {
        o->set(name.c_str(), static_cast<uint16_t>(value));
      }
    }
    catch (daq::config::Generic & ex) {
      std::string msg = make_error_txt(o, name.c_str(), ex.what());
      throw_jexception(env, "config/SystemException", msg.c_str(), "OksConfigObject::set_short()");
    }
  }
  else {
    throw_jexception(env, "config/SystemException", "joksconfig.so::get_oks_object() failed", "OksConfigObject::set_short()");
  }
//  set_value(env, obj, name_jstr, static_cast<unsigned short>(value));
}

/*
 * Class:     oksconfig_OksConfigObject
 * Method:    set_int
 * Signature: (Ljava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_oksconfig_OksConfigObject_set_1int
(JNIEnv * env, jobject obj, jstring name_jstr, jint value)
{
  JString name(env, name_jstr);

  if(OksConfigObject * o = get_oks_object(env, obj)) {
    try {
      if(o->get_type(name.c_str()) == OksData::s32_int_type) {
        o->set(name.c_str(), static_cast<int32_t>(value));
      }
      else {
        o->set(name.c_str(), static_cast<uint32_t>(value));
      }
    }
    catch (daq::config::Generic & ex) {
      std::string msg = make_error_txt(o, name.c_str(), ex.what());
      throw_jexception(env, "config/SystemException", msg.c_str(), "OksConfigObject::set_int()");
    }
  }
  else {
    throw_jexception(env, "config/SystemException", "joksconfig.so::get_oks_object() failed", "OksConfigObject::set_int()");
  }
//  set_value(env, obj, name_jstr, static_cast<unsigned long>(value));
}

/*
 * Class:     oksconfig_OksConfigObject
 * Method:    set_long
 * Signature: (Ljava/lang/String;J)V
 */
JNIEXPORT void JNICALL Java_oksconfig_OksConfigObject_set_1long
(JNIEnv * env, jobject obj, jstring name_jstr, jlong value)
{
  JString name(env, name_jstr);

  if(OksConfigObject * o = get_oks_object(env, obj)) {
    try {
      if(o->get_type(name.c_str()) == OksData::s64_int_type) {
        o->set(name.c_str(), static_cast<int64_t>(value));
      }
      else {
        o->set(name.c_str(), static_cast<uint64_t>(value));
      }
    }
    catch (daq::config::Generic & ex) {
      std::string msg = make_error_txt(o, name.c_str(), ex.what());
      throw_jexception(env, "config/SystemException", msg.c_str(), "OksConfigObject::set_long()");
    }
  }
  else {
    throw_jexception(env, "config/SystemException", "joksconfig.so::get_oks_object() failed", "OksConfigObject::set_long()");
  }
//  set_value(env, obj, name_jstr, static_cast<uint64_t>(value));
}

/*
 * Class:     oksconfig_OksConfigObject
 * Method:    set_float
 * Signature: (Ljava/lang/String;F)V
 */
JNIEXPORT void JNICALL Java_oksconfig_OksConfigObject_set_1float
(JNIEnv * env, jobject obj, jstring name_jstr, jfloat value)
{
  set_value(env, obj, name_jstr, static_cast<float>(value));
}

/*
 * Class:     oksconfig_OksConfigObject
 * Method:    set_double
 * Signature: (Ljava/lang/String;D)V
 */
JNIEXPORT void JNICALL Java_oksconfig_OksConfigObject_set_1double
(JNIEnv * env, jobject obj, jstring name_jstr, jdouble value)
{
  set_value(env, obj, name_jstr, static_cast<double>(value));
}

/*
 * Class:     oksconfig_OksConfigObject
 * Method:    set_string
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_oksconfig_OksConfigObject_set_1string
(JNIEnv * env, jobject obj, jstring name_jstr, jstring v)
{
  JString value(env, v);
  JString name(env, name_jstr);

  if(OksConfigObject * o = get_oks_object(env, obj)) {
    try {
      std::string s(value.c_str());
      switch(o->get_type(name.c_str())) {
        case OksData::string_type:  o->set(name.c_str(), s);        break;
        case OksData::enum_type:    o->set_enum(name.c_str(), s);   break;
        case OksData::date_type:    o->set_date(name.c_str(), s);   break;
        case OksData::time_type:    o->set_time(name.c_str(), s);   break;
        case OksData::class_type:   o->set_class(name.c_str(), s);  break;
	default:
	  throw_jexception(env, "config/SystemException", "unsupported type of string", "OksConfigObject::set_string()");
      }
    }
    catch (daq::config::Generic & ex) {
      std::string msg = make_error_txt(o, name.c_str(), ex.what());
      throw_jexception(env, "config/SystemException", msg.c_str(), "OksConfigObject::set_string()");
    }
  }
  else {
    throw_jexception(env, "config/SystemException", "joksconfig.so::get_oks_object() failed", "OksConfigObject::set_string()");
  }
//  set_value(env, obj, name_jstr, std::string(value.c_str()));
}

#define SetValues(CppType, JavaType, GetArrayF, ReleaseArrayF)                  \
  if(OksConfigObject * o = get_oks_object(env, obj)) {                          \
    JString name(env,name_jstr);                                                \
    try {                                                                       \
      std::vector<CppType> cpp_value;                                           \
      JavaType *body = env->GetArrayF(arr, 0);                                  \
      for (int i=0; i<env->GetArrayLength(arr); i++) {                          \
        cpp_value.push_back(body[i]);                                           \
      }                                                                         \
      env->ReleaseArrayF(arr, body, 0);                                         \
      o->set(name.c_str(), cpp_value);                                          \
    }										\
    catch(daq::config::Exception & ex) {					\
      std::string msg = make_error_txt(o, name.c_str(), ex.what());		\
      throw_jexception(env, "config/SystemException", msg.c_str(),              \
                       "OksConfigObject::set_T()");                             \
    }										\
  }										\
  else {									\
    throw_jexception(env, "config/SystemException",                             \
                     "joksconfig.so::get_oks_object() failed",                  \
		     "OksConfigObject::set_T()");                               \
  }


/*
 * Class:     oksconfig_OksConfigObject
 * Method:    set_bools
 * Signature: (Ljava/lang/String;[Z)V
 */
JNIEXPORT void JNICALL Java_oksconfig_OksConfigObject_set_1bools
(JNIEnv * env, jobject obj, jstring name_jstr, jbooleanArray arr)
{
  SetValues(bool, jboolean, GetBooleanArrayElements, ReleaseBooleanArrayElements)
}

/*
 * Class:     oksconfig_OksConfigObject
 * Method:    set_chars
 * Signature: (Ljava/lang/String;[C)V
 */
JNIEXPORT void JNICALL Java_oksconfig_OksConfigObject_set_1chars
(JNIEnv * env, jobject obj, jstring name_jstr, jcharArray arr)
{
  SetValues(int8_t, jchar, GetCharArrayElements, ReleaseCharArrayElements)
}

/*
 * Class:     oksconfig_OksConfigObject
 * Method:    set_bytes
 * Signature: (Ljava/lang/String;[B)V
 */
JNIEXPORT void JNICALL Java_oksconfig_OksConfigObject_set_1bytes
(JNIEnv * env, jobject obj, jstring name_jstr, jbyteArray arr)
{
  SetValues(uint8_t, jbyte, GetByteArrayElements, ReleaseByteArrayElements)
}

/*
 * Class:     oksconfig_OksConfigObject
 * Method:    set_shorts
 * Signature: (Ljava/lang/String;[S)V
 */
JNIEXPORT void JNICALL Java_oksconfig_OksConfigObject_set_1shorts
(JNIEnv * env, jobject obj, jstring name_jstr, jshortArray arr)
{
  if(OksConfigObject * o = get_oks_object(env, obj)) {
    JString name(env,name_jstr);
    try {
      jshort * body = env->GetShortArrayElements(arr, 0);

      std::vector<int16_t> cpp_s_value;
      std::vector<uint16_t> cpp_u_value;

      OksData::Type type = o->get_type(name.c_str());

      for (int i=0; i<env->GetArrayLength(arr); i++) {
        if(type == OksData::s16_int_type) {
          cpp_s_value.push_back(static_cast<int16_t>(body[i]));
	}
	else {
          cpp_u_value.push_back(static_cast<uint16_t>(body[i]));
	}
      }

      env->ReleaseShortArrayElements(arr, body, 0);

      if(type == OksData::s16_int_type) {
        o->set(name.c_str(), cpp_s_value);
      }
      else {
        o->set(name.c_str(), cpp_u_value);
      }
    }
    catch(daq::config::Exception & ex) {
      std::string msg = make_error_txt(o, name.c_str(), ex.what());
      throw_jexception(env, "config/SystemException", msg.c_str(), "OksConfigObject::set_shorts()");
    }
  }
  else {
    throw_jexception(env, "config/SystemException", "joksconfig.so::get_oks_object() failed", "OksConfigObject::set_shorts()");
  }

//  SetValues(unsigned short, jshort, GetShortArrayElements, ReleaseShortArrayElements)
}

/*
 * Class:     oksconfig_OksConfigObject
 * Method:    set_ints
 * Signature: (Ljava/lang/String;[I)V
 */
JNIEXPORT void JNICALL Java_oksconfig_OksConfigObject_set_1ints
(JNIEnv * env, jobject obj, jstring name_jstr, jintArray arr)
{
  if(OksConfigObject * o = get_oks_object(env, obj)) {
    JString name(env,name_jstr);
    try {
      jint * body = env->GetIntArrayElements(arr, 0);

      std::vector<int32_t> cpp_s_value;
      std::vector<uint32_t> cpp_u_value;

      OksData::Type type = o->get_type(name.c_str());

      for (int i=0; i<env->GetArrayLength(arr); i++) {
        if(type == OksData::s32_int_type) {
          cpp_s_value.push_back(static_cast<int32_t>(body[i]));
	}
	else {
          cpp_u_value.push_back(static_cast<uint32_t>(body[i]));
	}
      }

      env->ReleaseIntArrayElements(arr, body, 0);

      if(type == OksData::s32_int_type) {
        o->set(name.c_str(), cpp_s_value);
      }
      else {
        o->set(name.c_str(), cpp_u_value);
      }
    }
    catch(daq::config::Exception & ex) {
      std::string msg = make_error_txt(o, name.c_str(), ex.what());
      throw_jexception(env, "config/SystemException", msg.c_str(), "OksConfigObject::set_ints()");
    }
  }
  else {
    throw_jexception(env, "config/SystemException", "joksconfig.so::get_oks_object() failed", "OksConfigObject::set_ints()");
  }

//  SetValues(unsigned long, jint, GetIntArrayElements, ReleaseIntArrayElements)
}

/*
 * Class:     oksconfig_OksConfigObject
 * Method:    set_longs
 * Signature: (Ljava/lang/String;[J)V
 */
JNIEXPORT void JNICALL Java_oksconfig_OksConfigObject_set_1longs
(JNIEnv * env, jobject obj, jstring name_jstr, jlongArray arr)
{
  if(OksConfigObject * o = get_oks_object(env, obj)) {
    JString name(env,name_jstr);
    try {
      jlong * body = env->GetLongArrayElements(arr, 0);

      std::vector<int64_t> cpp_s_value;
      std::vector<uint64_t> cpp_u_value;

      OksData::Type type = o->get_type(name.c_str());

      for (int i=0; i<env->GetArrayLength(arr); i++) {
        if(type == OksData::s64_int_type) {
          cpp_s_value.push_back(static_cast<int64_t>(body[i]));
	}
	else {
          cpp_u_value.push_back(static_cast<uint64_t>(body[i]));
	}
      }

      env->ReleaseLongArrayElements(arr, body, 0);

      if(type == OksData::s64_int_type) {
        o->set(name.c_str(), cpp_s_value);
      }
      else {
        o->set(name.c_str(), cpp_u_value);
      }
    }
    catch(daq::config::Exception & ex) {
      std::string msg = make_error_txt(o, name.c_str(), ex.what());
      throw_jexception(env, "config/SystemException", msg.c_str(), "OksConfigObject::set_longs()");
    }
  }
  else {
    throw_jexception(env, "config/SystemException", "joksconfig.so::get_oks_object() failed", "OksConfigObject::set_longs()");
  }

//  SetValues(uint64_t, jlong, GetLongArrayElements, ReleaseLongArrayElements)
}

/*
 * Class:     oksconfig_OksConfigObject
 * Method:    set_floats
 * Signature: (Ljava/lang/String;[F)V
 */
JNIEXPORT void JNICALL Java_oksconfig_OksConfigObject_set_1floats
(JNIEnv * env, jobject obj, jstring name_jstr, jfloatArray arr)
{
  SetValues(float, jfloat, GetFloatArrayElements, ReleaseFloatArrayElements)
}

/*
 * Class:     oksconfig_OksConfigObject
 * Method:    set_doubles
 * Signature: (Ljava/lang/String;[D)V
 */
JNIEXPORT void JNICALL Java_oksconfig_OksConfigObject_set_1doubles
(JNIEnv * env, jobject obj, jstring name_jstr, jdoubleArray arr)
{
  SetValues(double, jdouble, GetDoubleArrayElements, ReleaseDoubleArrayElements)
}

/*
 * Class:     oksconfig_OksConfigObject
 * Method:    set_strings
 * Signature: (Ljava/lang/String;[Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_oksconfig_OksConfigObject_set_1strings
(JNIEnv * env, jobject obj, jstring name_jstr, jobjectArray arr)
{
  if(OksConfigObject * o = get_oks_object(env, obj)) {
    JString name(env,name_jstr);
    try {
      std::vector<std::string> cpp_value;
      for (int i=0; i<env->GetArrayLength(arr); i++) {
        jstring js = (jstring)env->GetObjectArrayElement(arr, i);
        JString s(env, js);
        cpp_value.push_back(s.c_str());
        env->DeleteLocalRef(js);
      }

      switch(o->get_type(name.c_str())) {
        case OksData::string_type:  o->set(name.c_str(), cpp_value);        break;
        case OksData::enum_type:    o->set_enum(name.c_str(), cpp_value);   break;
        case OksData::date_type:    o->set_date(name.c_str(), cpp_value);   break;
        case OksData::time_type:    o->set_time(name.c_str(), cpp_value);   break;
        case OksData::class_type:   o->set_class(name.c_str(), cpp_value);  break;
	default:
	  throw_jexception(env, "config/SystemException", "unsupported type of string", "OksConfigObject::set_strings()");
      }
    }
    catch(daq::config::Exception & ex) {
      std::string msg = make_error_txt(o, name.c_str(), ex.what());
      throw_jexception(env, "config/SystemException", msg.c_str(), "OksConfigObject::set_strings()");
    }
  }
  else {
    throw_jexception(env, "config/SystemException", "joksconfig.so::get_oks_object() failed", "OksConfigObject::set_strings()");                               \
  }
}


/*
 * Class:     oksconfig_OksConfigObject
 * Method:    set_impl_object
 * Signature: (Ljava/lang/String;Loksconfig/OksConfigObject;)V
 */
JNIEXPORT void JNICALL Java_oksconfig_OksConfigObject_set_1impl_1object
(JNIEnv * env, jobject obj, jstring name_jstr, jobject val)
{
  if(OksConfigObject * o = get_oks_object(env, obj)) {
    JString name(env,name_jstr);

    try {
      if(val != 0) {
	if(OksConfigObject * v = get_oks_object(env, val)) {
          ConfigObject p(v);
          o->set(name.c_str(), &p, false);
	}
	else {
	  throw_jexception(env, "config/SystemException", "joksconfig.so::get_oks_object(val) failed", "OksConfigObject::set_object()"); 
	}
      }
      else {
        o->set(name.c_str(), static_cast<ConfigObject *>(0), false);
      }
    }
    catch(daq::config::Exception & ex) {
      std::string msg = make_error_txt(o, name.c_str(), ex.what());
      throw_jexception(env, "config/SystemException", msg.c_str(), "OksConfigObject::set_object()");
    }
  }
  else {
    throw_jexception(env, "config/SystemException", "joksconfig.so::get_oks_object() failed", "OksConfigObject::set_object()");                               \
  }
}


/*
 * Class:     oksconfig_OksConfigObject
 * Method:    set_impl_objects
 * Signature: (Ljava/lang/String;[Loksconfig/OksConfigObject;)V
 */
JNIEXPORT void JNICALL Java_oksconfig_OksConfigObject_set_1impl_1objects
(JNIEnv * env, jobject obj, jstring name_jstr, jobjectArray arr)
{
  if(OksConfigObject * o = get_oks_object(env, obj)) {
    JString name(env,name_jstr);

    std::vector<const ConfigObject*> cpp_value;

    int len = env->GetArrayLength(arr);
    for (int i=0; i<len; i++) {
      jobject jo = (jobject)env->GetObjectArrayElement(arr, i);
      if(OksConfigObject * v = get_oks_object(env, jo)) {
        cpp_value.push_back(new ConfigObject(v));
      }
      else {
        throw_jexception(env, "config/SystemException", "joksconfig.so::get_oks_objects(val) failed", "OksConfigObject::set_objects()"); 
      }
      env->DeleteLocalRef(jo);
    }

    o->set(name.c_str(), cpp_value, false);

    for(std::vector<const ConfigObject*>::const_iterator j = cpp_value.begin(); j != cpp_value.end(); ++j) {
      delete const_cast<ConfigObject*>(*j);
    }
  }
  else {
    throw_jexception(env, "config/SystemException", "joksconfig.so::get_oks_objects() failed", "OksConfigObject::set_objects()");                               \
  }
}

/*
 * Class:     oksconfig_OksConfigObject
 * Method:    clean
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_oksconfig_OksConfigObject_clean
(JNIEnv *, jobject)
{
  ; // nothing to do
}
