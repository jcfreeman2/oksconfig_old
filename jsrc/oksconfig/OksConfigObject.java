package oksconfig;

import java.util.List;

import config.ConfigurationImpl;


public class OksConfigObject implements config.ConfigObjectImpl
{

  private long ptr;                 // c++ pointer to OksConfigObject
  private String m_uid;
  private String m_class_name;
  OksConfiguration m_db;

  private native void destroy();    // delete conf_ptr

  public OksConfigObject(long ref) {
    ptr = ref;
    m_uid = null;
    m_class_name = null;
    m_db = null;
  }

  public String UID() {
    if(m_uid == null) {
      m_uid = get_UID();
    }
    return m_uid;
  }

  public String class_name() {
    if(m_class_name == null) {
      m_class_name = get_class_name();
    }
    return m_class_name;
  }
  
  public native String get_UID();

  public native String get_class_name();

  public ConfigurationImpl get_configuration_impl() {
    return m_db;
  }

  public native boolean get_bool(String name) throws config.NotFoundException, config.SystemException;

  public native char get_char(String name) throws config.NotFoundException, config.SystemException;

  public native byte get_byte(String name) throws config.NotFoundException, config.SystemException;

  public native short get_short (String name) throws config.NotFoundException, config.SystemException;

  public native int get_int(String name) throws config.NotFoundException, config.SystemException;

  public native long get_long(String name) throws config.NotFoundException, config.SystemException;

  public native float get_float(String name) throws config.NotFoundException, config.SystemException;

  public native double get_double(String name) throws config.NotFoundException, config.SystemException;

  public native String get_string(String name) throws config.NotFoundException, config.SystemException;

  public native boolean[] get_bools(String name) throws config.NotFoundException, config.SystemException;

  public native char[] get_chars(String name) throws config.NotFoundException, config.SystemException;

  public native byte[] get_bytes(String name) throws config.NotFoundException, config.SystemException;

  public native short[] get_shorts(String name) throws config.NotFoundException, config.SystemException;

  public native int[] get_ints(String name) throws config.NotFoundException, config.SystemException;

  public native long[] get_longs(String name) throws config.NotFoundException, config.SystemException;

  public native float[] get_floats(String name) throws config.NotFoundException, config.SystemException;

  public native double[] get_doubles(String name) throws config.NotFoundException, config.SystemException;

  public native String[] get_strings(String name) throws config.NotFoundException, config.SystemException;

  public native OksConfigObject get_impl_object(String name)  throws config.NotFoundException, config.SystemException;

  public config.ConfigObject get_object(String name)  throws config.NotFoundException, config.SystemException {
    OksConfigObject o = get_impl_object(name);

    if(o != null) {
      o.m_db = m_db;
      return new config.ConfigObject(o);
    }
    else {
      return null;
    }
  }

  public native OksConfigObject[] get_referenced_by_impl_objects(String name, boolean check_composite_only) throws config.SystemException;

  public void referenced_by(List<config.ConfigObject> value, String relationship_name, boolean check_composite_only) throws config.SystemException
  {
    OksConfigObject[] oks_config_objects = get_referenced_by_impl_objects(relationship_name, check_composite_only);

    if(oks_config_objects != null)
      {
        for(int i = 0; i < oks_config_objects.length; i++)
          {
            oks_config_objects[i].m_db = m_db;
            value.add(new config.ConfigObject(oks_config_objects[i]));
          }
      }
  }

  
  public native OksConfigObject[] get_impl_objects(String name) throws config.NotFoundException, config.SystemException;

  public config.ConfigObject[] get_objects(String name) throws config.NotFoundException, config.SystemException {
    OksConfigObject[] oks_config_objects = get_impl_objects(name);

    if(oks_config_objects != null) {
      config.ConfigObject[] objs = new config.ConfigObject[oks_config_objects.length];
      for(int i = 0; i < oks_config_objects.length; i++) {
        oks_config_objects[i].m_db = m_db;
        objs[i] = new config.ConfigObject(oks_config_objects[i]);
      }

      return objs;
    }
    else {
      throw new config.NotFoundException(
        "ERROR [OksConfigObject.get_objects() \"" + UID() + "@" + class_name() + "\"]: failed to get value of relationship \"" + name + "\""
      );
    }
  }

  public native void set_bool(String name, boolean value) throws config.NotFoundException, config.NotAllowedException, config.SystemException;

  public native void set_char(String name, char value) throws config.NotFoundException, config.NotAllowedException, config.SystemException;

  public native void set_byte(String name, byte value) throws config.NotFoundException, config.NotAllowedException, config.SystemException;

  public native void set_short(String name, short value) throws config.NotFoundException, config.NotAllowedException, config.SystemException;

  public native void set_int(String name, int value) throws config.NotFoundException, config.NotAllowedException, config.SystemException;

  public native void set_long(String name, long value) throws config.NotFoundException, config.NotAllowedException, config.SystemException;

  public native void set_float(String name, float value) throws config.NotFoundException, config.NotAllowedException, config.SystemException;

  public native void set_double(String name, double value) throws config.NotFoundException, config.NotAllowedException, config.SystemException;

  public native void set_string(String name, String value) throws config.NotFoundException, config.NotAllowedException, config.SystemException;

  public native void set_bools(String name, boolean[] value) throws config.NotFoundException, config.NotAllowedException, config.SystemException;

  public native void set_chars(String name, char[] value) throws config.NotFoundException, config.NotAllowedException, config.SystemException;

  public native void set_bytes(String name, byte[] value) throws config.NotFoundException, config.NotAllowedException, config.SystemException;

  public native void set_shorts(String name, short[] value) throws config.NotFoundException, config.NotAllowedException, config.SystemException;

  public native void set_ints(String name, int[] value) throws config.NotFoundException, config.NotAllowedException, config.SystemException;

  public native void set_longs(String name, long[] value) throws config.NotFoundException, config.NotAllowedException, config.SystemException;

  public native void set_floats(String name, float[] value) throws config.NotFoundException, config.NotAllowedException, config.SystemException;

  public native void set_doubles(String name, double[] value) throws config.NotFoundException, config.NotAllowedException, config.SystemException;

  public native void set_strings(String name, String[] value) throws config.NotFoundException, config.NotAllowedException, config.SystemException;

  public native void set_impl_object(String name, OksConfigObject value) throws config.NotFoundException, config.NotAllowedException, config.SystemException;

  public void set_object(String name, config.ConfigObject value) throws config.NotFoundException, config.NotAllowedException, config.SystemException {
    if(value != null) {
      OksConfigObject obj = (OksConfigObject)value.implementation();
      obj.m_db = m_db;
      set_impl_object(name, obj);
    }
    else {
      set_impl_object(name, null);
    }
  }

  public native void set_impl_objects(String name, OksConfigObject[] value) throws config.NotFoundException, config.NotAllowedException, config.SystemException;

  public void set_objects(String name, config.ConfigObject[] value) throws config.NotFoundException, config.NotAllowedException, config.SystemException {
    OksConfigObject[] objs = new OksConfigObject[value.length];
    for(int i = 0; i < value.length; i++) {
      objs[i] = (OksConfigObject)(value[i].implementation());
      objs[i].m_db = m_db;
    }
    set_impl_objects(name, objs);
  }

  public native void clean();

}
