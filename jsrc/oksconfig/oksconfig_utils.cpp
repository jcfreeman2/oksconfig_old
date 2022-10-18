#include <iostream>
#include <string>

#include "oksconfig_utils.h"

OksConfigObject *
get_oks_object(JNIEnv * env, jobject obj)
{
  jfieldID fid = env->GetFieldID(env->GetObjectClass(obj), "ptr", "J");
  return ((fid == 0) ? 0 : reinterpret_cast<OksConfigObject *>(env->GetLongField(obj, fid)));
}

OksConfiguration *
get_conf_object(JNIEnv *env, jobject obj)
{
  jfieldID fid = env->GetFieldID(env->GetObjectClass(obj), "ptr", "J");
  return ((fid == 0) ? 0 : reinterpret_cast<OksConfiguration *>(env->GetLongField(obj, fid)));
}

jclass
get_jclass(JNIEnv * env, const char * class_name, const char * fname)
{
  jclass java_class = env->FindClass(class_name);

  if(!java_class) {
    std::cerr << "ERROR [" << fname << "]: cannot find java class " << class_name << std::endl;
  }

  return java_class;
}

jmethodID
get_jclass_constructor(JNIEnv * env, jclass java_class, const char * class_name, const char * fname)
{
  jmethodID constructor = env->GetMethodID(java_class, "<init>", "(J)V");

  if(!constructor) {
    std::cerr << "ERROR [" << fname << "]: cannot find constructor of java class " << class_name << std::endl;
  }

  return constructor;
}

jobject
create_jobject(JNIEnv * env, jclass java_class, jmethodID constructor, OksConfigObject * obj, const char * class_name, const char * fname)
{
  jobject jobj = env->NewObject(
    java_class,
    constructor,
    reinterpret_cast<long>(obj)
  );

  if(!jobj) {
    std::cerr << "ERROR [" << fname << "]: cannot create an object of java class " << class_name << std::endl;
  }

  return jobj;
}

jobject
create_oks_object(JNIEnv * env, OksConfigObject * oks_conf_obj, const char * fname)
{
  const char * jname = "oksconfig/OksConfigObject";

  jclass oks_conf_obj_class = get_jclass(env, jname, fname);
  if(!oks_conf_obj_class) { return 0 ; }

  jmethodID oks_conf_obj_init = get_jclass_constructor(env, oks_conf_obj_class, jname, fname);
  if(!oks_conf_obj_init) { return 0 ; }

  return create_jobject(env, oks_conf_obj_class, oks_conf_obj_init, oks_conf_obj, jname, fname);
}

void
throw_jexception(JNIEnv * env, const char * name, const char * msg, const char * fname)
{
  if (jclass ex_jclass = get_jclass(env, name, fname))
    env->ThrowNew(ex_jclass, msg);
}

JString::JString(JNIEnv * env, jstring s) :
  e        (env),
  j_string (s),
  c_string (e->GetStringUTFChars(s,0))
{
  ;
}

JString::~JString()
{
  e->ReleaseStringUTFChars(j_string, c_string);
}
