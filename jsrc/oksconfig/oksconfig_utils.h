#include <jni.h>

  // generated files

#include "oksconfig_OksConfigObject.h"
#include "oksconfig_OksConfiguration.h"

class OksConfiguration;
class OksConfigObject;
class ConfigObject;

  // utilities to find object's class and constructor, and to create an object

jclass get_jclass(JNIEnv *, const char *, const char *);
jmethodID get_jclass_constructor(JNIEnv *, jclass, const char *, const char *);
jobject create_jobject(JNIEnv *, jclass, jmethodID, OksConfigObject *, const char *, const char *);


  // get implementation objects

OksConfiguration * get_conf_object(JNIEnv *, jobject);
OksConfigObject * get_oks_object(JNIEnv *, jobject);

  // create implementation object

jobject create_oks_object(JNIEnv *, OksConfigObject *, const char *);

  // convert ConfigObject to OksConfigObject

OksConfigObject * get_impl_object(ConfigObject&);

  // throw exception
  
void throw_jexception(JNIEnv *, const char *, const char *, const char *);


class JString {

  public:

    JString(JNIEnv *, jstring);
    ~JString();

    const char * c_str() const {return c_string;}

  private:

    JNIEnv * e;
    jstring j_string;
    const char * c_string;
};
