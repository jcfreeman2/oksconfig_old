#include <jni.h>

#include <iostream>
#include <sstream>

#include <config/ConfigObject.h>
#include <oksconfig/OksConfigObject.h>
#include <oksconfig/OksConfiguration.h>

#include "oksconfig_utils.h"


//////////////////////////////////////////////////////////////////////////////////////////

OksConfigObject * get_impl_object(ConfigObject& obj)
{
  ConfigObjectImpl * impl_obj = const_cast<ConfigObjectImpl *>(obj.implementation());
  OksConfigObject * oks_conf_obj = static_cast<OksConfigObject *>(impl_obj);

//  oks_conf_obj->ref();  // take ownership

  return oks_conf_obj;
}

//////////////////////////////////////////////////////////////////////////////////////////

/*
 * Class:     oksconfig_OksConfiguration
 * Method:    initialize
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_oksconfig_OksConfiguration_initialize
(JNIEnv * env, jobject obj)
{
  if(jfieldID fid = env->GetFieldID(env->GetObjectClass(obj), "ptr", "J")) {
    env->SetLongField(obj, fid, reinterpret_cast<jlong>(new OksConfiguration()));
  }
  else {
    throw_jexception(env, "config/SystemException", "JNI GetFieldID() failed", "OksConfiguration::initialize()");
  }
}

/*
 * Class:     oksconfig_OksConfiguration
 * Method:    destroy
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_oksconfig_OksConfiguration_destroy__
(JNIEnv * env, jobject obj)
{
  if(jfieldID fid = env->GetFieldID(env->GetObjectClass(obj), "ptr", "J")) {
    delete reinterpret_cast<OksConfiguration *>(env->GetLongField(obj, fid));
    env->SetLongField(obj, fid, 0);
  }
  else {
    throw_jexception(env, "config/SystemException", "JNI GetFieldID() failed", "OksConfiguration::destroy()");
  }
}

/*
 * Class:     oksconfig_OksConfiguration
 * Method:    open_db
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_oksconfig_OksConfiguration_open_1db
(JNIEnv * env, jobject obj, jstring str)
{
  if(OksConfiguration * conf = get_conf_object(env, obj)) {
    JString db_name(env, str);
    try {
      conf->open_db(db_name.c_str());
    }
    catch (daq::config::Exception & ex) {
      std::ostringstream text;
      text << "cannot load \"" << db_name.c_str() << "\" file\n\twas caused by: " << ex;
      throw_jexception(env, "config/SystemException", text.str().c_str(), "OksConfiguration::open_db()");
    }
  }
  else {
    throw_jexception(env, "config/SystemException", "joksconfig.so::get_conf_object() failed", "OksConfiguration::open_db()");
  }
}

/*
 * Class:     oksconfig_OksConfiguration
 * Method:    close_db
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_oksconfig_OksConfiguration_close_1db
(JNIEnv * env, jobject obj)
{
  if(OksConfiguration * conf = get_conf_object(env, obj)) {
    conf->close_db();
  }
  else {
    throw_jexception(env, "config/SystemException", "joksconfig.so::get_conf_object() failed", "OksConfiguration::close_db()");
  }
}

/*
 * Class:     oksconfig_OksConfiguration
 * Method:    loaded
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_oksconfig_OksConfiguration_loaded
(JNIEnv * env, jobject obj)
{
  OksConfiguration * conf = get_conf_object(env, obj);
  return (conf ? conf->loaded() : false);
}


/*
 * Class:     oksconfig_OksConfiguration
 * Method:    get_list_of_classes
 * Signature: ()[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_oksconfig_OksConfiguration_get_1list_1of_1classes
(JNIEnv * env, jobject obj)
{
  if(OksConfiguration * conf = get_conf_object(env, obj)) {
    const OksClass::Map& cs = conf->get_oks_kernel().classes();

    jobjectArray classes_jstrs = (jobjectArray)env->NewObjectArray(
      cs.size(),
      get_jclass(env, "java/lang/String", "oksconfig_OksConfiguration::get_list_of_classes()"),
      env->NewStringUTF("")
    );

    unsigned int count = 0;
    for(OksClass::Map::const_iterator i = cs.begin(); i != cs.end(); ++i, ++count) {
      env->SetObjectArrayElement(classes_jstrs, count, env->NewStringUTF(i->first)) ;
    }

    return classes_jstrs;
  }
  else {
    throw_jexception(env, "config/SystemException", "joksconfig.so::get_conf_object() failed", "OksConfiguration::add_include()");
    return 0;
  }
}


/*
 * Class:     oksconfig_OksConfiguration
 * Method:    get_superclasses
 * Signature: (Ljava/lang/String;)[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_oksconfig_OksConfiguration_get_1superclasses
(JNIEnv * env, jobject obj, jstring class_name_jstr)
{
  if(OksConfiguration * conf = get_conf_object(env, obj)) {
    JString class_name(env, class_name_jstr);
    
    if(OksClass * c = conf->get_oks_kernel().find_class(class_name.c_str())) {
      if(const OksClass::FList * scl = c->all_super_classes()) {
        jobjectArray classes_jstrs = (jobjectArray)env->NewObjectArray(
          scl->size(),
          get_jclass(env, "java/lang/String", "oksconfig_OksConfiguration::get_superclasses()"),
          env->NewStringUTF("")
        );

        unsigned int count = 0;
        for(OksClass::FList::const_iterator i = scl->begin(); i != scl->end(); ++i, ++count) {
          env->SetObjectArrayElement(classes_jstrs, count, env->NewStringUTF((*i)->get_name().c_str())) ;
        }

        return classes_jstrs;
      }
    }
    else {
      std::ostringstream text;
      text << "Cannot find class \'" << class_name.c_str() << '\'';
      throw_jexception(env, "config/SystemException", text.str().c_str(), "OksConfiguration::get_superclasses()");
    }
  }
  else {
    throw_jexception(env, "config/SystemException", "joksconfig.so::get_conf_object() failed", "OksConfiguration::get_superclasses()");
  }

  return 0;
}


/*
 * Class:     oksconfig_OksConfiguration
 * Method:    create
 * Signature: (Ljava/lang/String;[Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_oksconfig_OksConfiguration_create
(JNIEnv * env, jobject obj, jstring db_name_jstr, jobjectArray includes_jstrs)
{
  if(OksConfiguration * conf = get_conf_object(env, obj)) {
    JString db_name(env, db_name_jstr);
    std::list<std::string> includes;
    jsize len = env->GetArrayLength(includes_jstrs);
    for(int i = 0; i < len; ++i) {
      jstring o = (jstring)env->GetObjectArrayElement(includes_jstrs, i);
      JString name(env, o);
      includes.push_back(name.c_str());
      env->DeleteLocalRef(o);  /* no longer needs o; can be freed by garbage collection */
    }

    try {
      conf->create(db_name.c_str(), includes);
    }
    catch (daq::config::Exception & ex) {
      std::ostringstream text;
      text << "Configuration::create(\"" << db_name.c_str() << "\", (...) ) failed\n\twas caused by: " << ex;
      throw_jexception(env, "config/SystemException", text.str().c_str(), "OksConfiguration::create()");
    }
  }
  else {
    throw_jexception(env, "config/SystemException", "joksconfig.so::get_conf_object() failed", "OksConfiguration::create()");
  }
}

//    virtual bool get_includes(const std::string& db_name, std::list<std::string>& includes) const;

/*
 * Class:     oksconfig_OksConfiguration
 * Method:    add_include
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_oksconfig_OksConfiguration_add_1include
(JNIEnv * env, jobject obj, jstring db_name_jstr, jstring include_jstr)
{
  if(OksConfiguration * conf = get_conf_object(env, obj)) {
    JString db_name(env, db_name_jstr);
    JString include(env, include_jstr);
    try {
      conf->add_include(db_name.c_str(), include.c_str());
    }
    catch (daq::config::Exception & ex) {
      std::ostringstream text;
      text << "Configuration::add_include(\"" << db_name.c_str() << "\", \"" << include.c_str() << "\") failed\n\twas caused by: " << ex;
      throw_jexception(env, "config/SystemException", text.str().c_str(), "OksConfiguration::add_include()");
    }
  }
  else {
    throw_jexception(env, "config/SystemException", "joksconfig.so::get_conf_object() failed", "OksConfiguration::add_include()");
  }
}

/*
 * Class:     oksconfig_OksConfiguration
 * Method:    remove_include
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_oksconfig_OksConfiguration_remove_1include
(JNIEnv * env, jobject obj, jstring db_name_jstr, jstring include_jstr)
{
  if(OksConfiguration * conf = get_conf_object(env, obj)) {
    JString db_name(env, db_name_jstr);
    JString include(env, include_jstr);

    try {
      conf->remove_include(db_name.c_str(), include.c_str());
    }
    catch (daq::config::Exception & ex) {
      std::ostringstream text;
      text << "Configuration::remove_include(\"" << db_name.c_str() << "\", \"" << include.c_str() << "\") failed\n\twas caused by: " << ex;
      throw_jexception(env, "config/SystemException", text.str().c_str(), "OksConfiguration::remove_include()");
    }
  }
  else {
    throw_jexception(env, "config/SystemException", "joksconfig.so::get_conf_object() failed", "OksConfiguration::remove_include()");
  }
}

/*
 * Class:     oksconfig_OksConfiguration
 * Method:    get_includes
 * Signature: (Ljava/lang/String;)[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_oksconfig_OksConfiguration_get_1includes
(JNIEnv * env, jobject obj, jstring db_name_jstr)
{
  if(OksConfiguration * conf = get_conf_object(env, obj)) {
    JString db_name(env, db_name_jstr);

    std::list<std::string> includes;

    try {
      conf->get_includes(db_name.c_str(), includes);
    }
    catch (daq::config::Exception & ex) {
      std::ostringstream text;
      text << "Configuration::get_includes(\"" << db_name.c_str() << "\") failed\n\twas caused by: " << ex;
      throw_jexception(env, "config/SystemException", text.str().c_str(), "OksConfiguration::get_includes()");
      return 0;
    }

    jobjectArray includes_jstrs = (jobjectArray)env->NewObjectArray(
      includes.size(),
      get_jclass(env, "java/lang/String", "oksconfig_OksConfiguration::get_includes()"),
      env->NewStringUTF("")
    );

    unsigned int count = 0;
    for(std::list<std::string>::iterator i = includes.begin(); i != includes.end(); ++i, ++count) {
      env->SetObjectArrayElement(includes_jstrs, count, env->NewStringUTF((*i).c_str())) ;
    }

    return includes_jstrs;
  }
  else {
    throw_jexception(env, "config/SystemException", "joksconfig.so::get_conf_object() failed", "OksConfiguration::get_includes()");
    return 0;
  }
}

/*
 * Class:     oksconfig_OksConfiguration
 * Method:    get_updated_dbs
 * Signature: ()[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_oksconfig_OksConfiguration_get_1updated_1dbs
  (JNIEnv * env, jobject obj)
{
  if(OksConfiguration * conf = get_conf_object(env, obj)) {
    std::list<std::string> dbs;

    try {
      conf->get_updated_dbs(dbs);
    }
    catch (daq::config::Exception & ex) {
      std::ostringstream text;
      text << "Configuration::get_updated_dbs() failed\n\twas caused by: " << ex;
      throw_jexception(env, "config/SystemException", text.str().c_str(), "OksConfiguration::get_updated_dbs()");
      return 0;
    }

    jobjectArray dbs_jstrs = (jobjectArray)env->NewObjectArray(
      dbs.size(),
      get_jclass(env, "java/lang/String", "oksconfig_OksConfiguration::get_updated_dbs()"),
      env->NewStringUTF("")
    );

    unsigned int count = 0;
    for(std::list<std::string>::iterator i = dbs.begin(); i != dbs.end(); ++i, ++count) {
      env->SetObjectArrayElement(dbs_jstrs, count, env->NewStringUTF((*i).c_str())) ;
    }

    return dbs_jstrs;
  }
  else {
    throw_jexception(env, "config/SystemException", "joksconfig.so::get_conf_object() failed", "OksConfiguration::get_updated_dbs()");
    return 0;
  }
}


/*
 * Class:     oksconfig_OksConfiguration
 * Method:    set_commit_credentials
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_oksconfig_OksConfiguration_set_1commit_1credentials
  (JNIEnv * env, jobject obj, jstring _user, jstring _password)
{
  if(OksConfiguration * conf = get_conf_object(env, obj)) {
    JString user(env, _user);
    JString password(env, _password);

    try {
      conf->set_commit_credentials(user.c_str(), password.c_str());
    }
    catch (daq::config::Exception & ex) {
      std::ostringstream text;
      text << "Configuration::set_commit_credentials() failed\n\twas caused by: " << ex;
      throw_jexception(env, "config/SystemException", text.str().c_str(), "OksConfiguration::set_commit_credentials()");
    }
  }
  else {
    throw_jexception(env, "config/SystemException", "joksconfig.so::get_conf_object() failed", "OksConfiguration::set_commit_credentials()");
  }
}


/*
 * Class:     oksconfig_OksConfiguration
 * Method:    commit
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_oksconfig_OksConfiguration_commit
  (JNIEnv * env, jobject obj, jstring why)
{
  if(OksConfiguration * conf = get_conf_object(env, obj)) {
    JString log_message(env, why);
    try {
      conf->commit(log_message.c_str());
      return true;
    }
    catch (daq::config::Exception & ex) {
      std::ostringstream text;
      text << "Configuration::commit() failed\n\twas caused by: " << ex;
      throw_jexception(env, "config/SystemException", text.str().c_str(), "OksConfiguration::commit()");
      return false;
    }
  }
  else {
    throw_jexception(env, "config/SystemException", "joksconfig.so::get_conf_object() failed", "OksConfiguration::commit()");
    return false;
  }
}

/*
 * Class:     oksconfig_OksConfiguration
 * Method:    abort
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_oksconfig_OksConfiguration_abort
(JNIEnv * env, jobject obj)
{
  if(OksConfiguration * conf = get_conf_object(env, obj)) {
    try {
      conf->abort();
      return true;
    }
    catch (daq::config::Exception & ex) {
      std::ostringstream text;
      text << "Configuration::abort() failed\n\twas caused by: " << ex;
      throw_jexception(env, "config/SystemException", text.str().c_str(), "OksConfiguration::abort()");
      return false;
    }
  }
  else {
    throw_jexception(env, "config/SystemException", "joksconfig.so::get_conf_object() failed", "OksConfiguration::abort()");
    return false;
  }
}

/*
 * Class:     oksconfig_OksConfiguration
 * Method:    get_object
 * Signature: (Ljava/lang/String;Ljava/lang/String;)Loksconfig/OksConfigObject;
 */
JNIEXPORT jobject JNICALL Java_oksconfig_OksConfiguration_get_1object
(JNIEnv * env, jobject obj, jstring class_name_jstr, jstring id_jstr)
{
  if(OksConfiguration * conf = get_conf_object(env, obj)) {
    JString class_name(env, class_name_jstr);
    JString id(env, id_jstr);

    try {
      ConfigObject config_obj;
      conf->get(class_name.c_str(), id.c_str(), config_obj, 1 /*FIXME*/, 0);
      return create_oks_object(env, get_impl_object(config_obj), "OksConfiguration::get_object()");
    }
    catch (daq::config::Exception & ex) {
      std::ostringstream text;
      text << "OksConfiguration::get(class-name: \"" << class_name.c_str() << "\", id: \"" << id.c_str() << "\") failed\n\twas caused by: " << ex;
      throw_jexception(env, "config/NotFoundException", text.str().c_str(), "OksConfiguration::get_object()");
      return 0;
    }
  }
  else {
    throw_jexception(env, "config/SystemException", "joksconfig.so::get_conf_object() failed", "OksConfiguration::get_object()");
    return 0;
  }
}

/*
 * Class:     oksconfig_OksConfiguration
 * Method:    get_objects
 * Signature: (Ljava/lang/String;Ljava/lang/String;)[Loksconfig/OksConfigObject;
 */
JNIEXPORT jobjectArray JNICALL Java_oksconfig_OksConfiguration_get_1objects
(JNIEnv * env, jobject obj, jstring class_name_jstr, jstring query_jstr)
{
  const char * fname = "oksconfig_OksConfiguration::get_objects()";
  const char * jname = "oksconfig/OksConfigObject";

  if(OksConfiguration * conf = get_conf_object(env, obj)) {
    JString class_name(env, class_name_jstr);
    JString query(env, query_jstr);
    
    try {
      std::vector<ConfigObject> objects;

      conf->get(class_name.c_str(), objects, query.c_str(), 1 /*FIXME*/, 0);

      jclass oks_conf_obj_class = get_jclass(env, jname, fname);
      if(!oks_conf_obj_class) { return 0 ; }

      jmethodID oks_conf_obj_init = get_jclass_constructor(env, oks_conf_obj_class, jname, fname);
      if(!oks_conf_obj_init) { return 0 ; }

      jobject dummu_jobj = create_jobject(env, oks_conf_obj_class, oks_conf_obj_init, 0, jname, fname);
      if(!dummu_jobj) { return 0 ; }

      jobjectArray objs = env->NewObjectArray(objects.size(), oks_conf_obj_class, dummu_jobj) ;

      unsigned int count  = 0;
      for(std::vector<ConfigObject>::iterator i = objects.begin(); i != objects.end(); ++i, ++count) {
        if(jobject o = create_jobject(env, oks_conf_obj_class, oks_conf_obj_init, get_impl_object(*i), jname, fname)) {
          env->SetObjectArrayElement(objs, count, o) ;
	}
        else {
          throw_jexception(env, "config/SystemException", "joksconfig.so::create_jobject() failed", "OksConfiguration::get_object()");
          return 0 ;
        }
      }

      return objs;
    }
    catch (daq::config::Exception & ex) {
      std::ostringstream text;
      text << "Configuration::get(class-name: \"" << class_name.c_str() << "\", query: \"" << query.c_str() << "\") failed\n\twas caused by: " << ex;
      throw_jexception(env, "config/NotFoundException", text.str().c_str(), "OksConfiguration::get_objects()");
      return 0;
    }
  }
  else {
    throw_jexception(env, "config/SystemException", "joksconfig.so::get_conf_object() failed", "OksConfiguration::get_objects()");
    return 0;
  }
}

/*
 * Class:     oksconfig_OksConfiguration
 * Method:    get_path_objects
 * Signature: (Loksconfig/OksConfigObject;Ljava/lang/String;)[Loksconfig/OksConfigObject;
 */
JNIEXPORT jobjectArray JNICALL Java_oksconfig_OksConfiguration_get_1path_1objects
  (JNIEnv * env, jobject obj, jobject from_jobj, jstring query_jstr)
{
  const char * fname = "oksconfig_OksConfiguration::get_path_objects()";
  const char * jname = "oksconfig/OksConfigObject";

  if(OksConfiguration * conf = get_conf_object(env, obj)) {
    JString query(env, query_jstr);
    std::vector<ConfigObject> objects;

    if(OksConfigObject * from = get_oks_object(env, from_jobj)) {
      try {
        ConfigObject dummu(from);
        conf->get(dummu, query.c_str(), objects, 1 /*FIXME*/, 0);
        jclass oks_conf_obj_class = get_jclass(env, jname, fname);
        if(!oks_conf_obj_class) { return 0 ; }

        jmethodID oks_conf_obj_init = get_jclass_constructor(env, oks_conf_obj_class, jname, fname);
        if(!oks_conf_obj_init) { return 0 ; }

        jobject dummu_jobj = create_jobject(env, oks_conf_obj_class, oks_conf_obj_init, 0, jname, fname);
        if(!dummu_jobj) { return 0 ; }

        jobjectArray objs = env->NewObjectArray(objects.size(), oks_conf_obj_class, dummu_jobj) ;

        unsigned int count  = 0;
        for(std::vector<ConfigObject>::iterator i = objects.begin(); i != objects.end(); ++i, ++count) {
          if(jobject o = create_jobject(env, oks_conf_obj_class, oks_conf_obj_init, get_impl_object(*i), jname, fname)) {
            env->SetObjectArrayElement(objs, count, o) ;
	  }
          else {
            throw_jexception(env, "config/SystemException", "joksconfig.so::create_jobject() failed", "OksConfiguration::get_object()");
            return 0 ;
          }
        }

        return objs;
      }
      catch (daq::config::Exception & ex) {
        std::ostringstream text;
        text << "Configuration::get(object-from: \"" << from->UID() << '@'  << from->class_name() << "\", query: \"" << query.c_str() << "\") failed\n\twas caused by: " << ex;
        throw_jexception(env, "config/NotFoundException", text.str().c_str(), "OksConfiguration::get_objects()");
        return 0;
      }
    }
    else {
      throw_jexception(env, "config/SystemException", "joksconfig.so::get_conf_object(\'FROM\') failed", "OksConfiguration::get_objects()");
      return 0 ;
    }
  }
  else {
    throw_jexception(env, "config/SystemException", "joksconfig.so::get_conf_object() failed", "OksConfiguration::get_objects()");
    return 0;
  }
}

/*
 * Class:     oksconfig_OksConfiguration
 * Method:    create_object
 * Signature: (Ljava/lang/String;Loksconfig/OksConfigObject;Ljava/lang/String;Ljava/lang/String;)Loksconfig/OksConfigObject;
 */
JNIEXPORT jobject JNICALL Java_oksconfig_OksConfiguration_create_1object
(JNIEnv * env, jobject obj, jstring db_jstr, jobject at, jstring class_name_jstr, jstring id_jstr)
{
  if(OksConfiguration * conf = get_conf_object(env, obj)) {
    JString class_name(env, class_name_jstr);
    JString id(env, id_jstr);
    ConfigObject config_obj;

    if(db_jstr != 0) {
      JString db(env, db_jstr);
      try {
        conf->create(db.c_str(), class_name.c_str(), id.c_str(), config_obj);
      }
      catch (daq::config::Exception & ex) {
        std::ostringstream text;
        text << "Configuration::create(db: \"" << db.c_str() << "\", object: \"" << id.c_str() << '@' << class_name.c_str() << "\") failed\n\twas caused by: " << ex;
        throw_jexception(env, "config/SystemException", text.str().c_str(), "OksConfiguration::create_object()");
        return 0 ;
      }
    }
    else {
      if(OksConfigObject * o = get_oks_object(env, at)) {
        try {
          conf->create(o, class_name.c_str(), id.c_str(), config_obj);
	}
        catch (daq::config::Exception & ex) {
          std::ostringstream text;
          text << "Configuration::create(existing object: \"" << o->UID() << '@' << o->class_name()
	       << "\", new object: \"" << id.c_str() << '@' << class_name.c_str() << "\") failed\n\twas caused by: " << ex;
          throw_jexception(env, "config/SystemException", text.str().c_str(), "OksConfiguration::create_object()");
          return 0 ;
        }
      }
      else {
        throw_jexception(env, "config/SystemException", "joksconfig.so::get_conf_object(\'AT\') failed", "OksConfiguration::create_object()");
        return 0 ;
      }
    }

    return create_oks_object(env, get_impl_object(config_obj), "oksconfig_OksConfiguration::create_object()");
  }
  else {
    throw_jexception(env, "config/SystemException", "joksconfig.so::get_conf_object() failed", "OksConfiguration::create_object()");
    return 0;
  }
}

/*
 * Class:     oksconfig_OksConfiguration
 * Method:    destroy_object
 * Signature: (Loksconfig/OksConfigObject;)V
 */
JNIEXPORT void JNICALL Java_oksconfig_OksConfiguration_destroy_1object
(JNIEnv * env, jobject obj, jobject o)
{
  if(OksConfiguration * conf = get_conf_object(env, obj)) {
    if(OksConfigObject * cobj = get_oks_object(env, o)) {
      try {
        ConfigObject dummu(cobj);
        conf->destroy(dummu);
      }
      catch (daq::config::Exception & ex) {
        std::ostringstream text;
        text << "Configuration::destroy(\"" << cobj->UID() << '@' << cobj->class_name() << "\") failed\n\twas caused by: " << ex;
        throw_jexception(env, "config/SystemException", text.str().c_str(), "OksConfiguration::destroy()");
      }
    }
    else {
      throw_jexception(env, "config/SystemException", "joksconfig.so::get_oks_object() failed", "OksConfiguration::destroy()");
    }
  }
  else {
    throw_jexception(env, "config/SystemException", "joksconfig.so::get_conf_object() failed", "OksConfiguration::destroy()");
  }
}

/*
 * Class:     oksconfig_OksConfiguration
 * Method:    move_object
 * Signature: (Loksconfig/OksConfigObject;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_oksconfig_OksConfiguration_move_1object
  (JNIEnv * env, jobject, jobject o, jstring at_jstr)
{
  JString at(env, at_jstr);

  if(OksConfigObject * cobj = get_oks_object(env, o)) {
    try {
      ConfigObject dummu(cobj);
      dummu.move(at.c_str());
    }
    catch (daq::config::Exception & ex) {
       std::ostringstream text;
       text << "Configuration::move(obj: \"" << cobj->UID() << '@' << cobj->class_name() << "\", db: \"" << at.c_str() << "\") failed\n\twas caused by: " << ex;
       throw_jexception(env, "config/SystemException", text.str().c_str(), "ConfigObject::move()");
    }
  }
  else {
    throw_jexception(env, "config/SystemException", "joksconfig.so::get_oks_object() failed", "ConfigObject::move()");
  }

}

/*
 * Class:     oksconfig_OksConfiguration
 * Method:    subscribe
 * Signature: ([Ljava/lang/String;[Ljava/lang/String;Lconfig/Configuration;)V
 */
JNIEXPORT void JNICALL Java_oksconfig_OksConfiguration_subscribe
(JNIEnv * env, jobject, jobjectArray, jobjectArray, jobject)
{
  throw_jexception(env, "config/SystemException", "Method oksconfig_OksConfiguration::subscribe() is not yet implemented", "OksConfiguration::subscribe()");
}

/*
 * Class:     oksconfig_OksConfiguration
 * Method:    unsubscribe
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_oksconfig_OksConfiguration_unsubscribe
  (JNIEnv * env, jobject)
{
  throw_jexception(env, "config/SystemException", "Method oksconfig_OksConfiguration::unsubscribe() is not yet implemented", "OksConfiguration::unsubscribe()");
}
