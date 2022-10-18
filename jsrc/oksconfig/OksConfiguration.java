package oksconfig;

import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.TreeSet;

import config.Configuration;

  /**
   *  The interface defines based methods to be supported by a database implementation.
   *
   *  @author  <Igor.Soloviev@cern.ch>
   *  @since   online release 01-00-00
   */

public class OksConfiguration implements config.ConfigurationImpl
{
    // load native methods

  static {
    System.loadLibrary("joksconfig");
  }

  private long ptr;                         // c++ pointer to OksConfiguration
  private config.Configuration m_config;    // is used for subscription, on which notify() is invoked

  private native void initialize();         // initialize 'ptr'
  private native void destroy();            // delete 'ptr'

  public OksConfiguration() {
    initialize();
  }

  public native void open_db(String db_name) throws config.SystemException;

  public native void close_db() throws config.SystemException;

  public native boolean loaded();

  public void set_configuration(Configuration db) {
    m_config = db;
  }

  public Configuration get_configuration() {
    return m_config;
  }

  public native String[] get_list_of_classes() throws config.SystemException;

  public native String[] get_superclasses(String class_name) throws config.SystemException;

  public java.util.TreeMap<String,java.util.TreeSet<String>> get_superclasses() throws config.SystemException {
    java.util.TreeMap<String,java.util.TreeSet<String>> result = new java.util.TreeMap<String,java.util.TreeSet<String>>();
    String[] classes = get_list_of_classes();
    for(int i = 0; i < classes.length; ++i) {
      String[] names = get_superclasses(classes[i]);
      if(names != null) {
        java.util.TreeSet<String> set = new java.util.TreeSet<String>();
        for(int j = 0; j < names.length; ++j) {
          set.add(names[j]);
	    }
	    result.put(classes[i], set);
      }
    }
    return result;
  }

  public native void create(String db_name, String[] includes) throws config.SystemException, config.NotAllowedException, config.AlreadyExistsException;

  public void create(String server_name, String db_name, String[] includes) throws config.SystemException, config.NotAllowedException, config.AlreadyExistsException {
    create(db_name, includes);
  }
  
  public native void add_include(String db_name, String include) throws config.SystemException, config.NotFoundException, config.NotAllowedException;

  public native void remove_include(String db_name, String include) throws config.SystemException, config.NotFoundException, config.NotAllowedException;

  public native String[] get_includes(String db_name) throws config.SystemException, config.NotFoundException;

  public native String[] get_updated_dbs() throws config.SystemException, config.NotAllowedException;

  public native void set_commit_credentials(String user, String password) throws config.SystemException, config.NotAllowedException;

  public native boolean commit(String why) throws config.SystemException, config.NotAllowedException;

  public native boolean abort() throws config.SystemException, config.NotAllowedException;

  public native OksConfigObject get_object(String class_name, String id) throws config.SystemException;

  public config.ConfigObject get(String class_name, String id) throws config.SystemException, config.NotFoundException {
    OksConfigObject o = get_object(class_name, id);

    if(o != null) {
      o.m_db = this;
      return new config.ConfigObject(o);
    }
    else {
      throw new config.NotFoundException(
        "object \"" + id + "\"@\"" + class_name + "\" not found"
      );
    }
  }

  public native OksConfigObject[] get_objects(String class_name, String query) throws config.SystemException;

  public config.ConfigObject[] get(String class_name, config.Query query) throws config.SystemException, config.NotFoundException {
    String q;
    if(query == null) {
      q = "";
    }
    else {
      q = query.get_query_string();
    }

    OksConfigObject[] oks_config_objects = get_objects(class_name, q);

    if(oks_config_objects != null) {
      config.ConfigObject[] objs = new config.ConfigObject[oks_config_objects.length];
      for(int i = 0; i < oks_config_objects.length; i++) {
        oks_config_objects[i].m_db = this;
        objs[i] = new config.ConfigObject(oks_config_objects[i]);
      }

      return objs;
    }
    else {
      throw new config.NotFoundException(
        "failed to get objects of class \"" + class_name + "\" with query \"" + q + "\""
      );
    }
  }

  public native OksConfigObject[] get_path_objects(OksConfigObject obj, String query) throws config.SystemException;

  public config.ConfigObject[] get(config.ConfigObject from, config.Query query) throws config.SystemException, config.NotFoundException {
    String q = query.get_query_string();

    OksConfigObject[] oks_config_objects = get_path_objects((OksConfigObject)(from.implementation()), q);

    if(oks_config_objects != null) {
      config.ConfigObject[] objs = new config.ConfigObject[oks_config_objects.length];
      for(int i = 0; i < oks_config_objects.length; i++) {
        oks_config_objects[i].m_db = this;
        objs[i] = new config.ConfigObject(oks_config_objects[i]);
      }

      return objs;
    }
    else {
      throw new config.NotFoundException(
        "failed to get path \"" + q + "\" from \"" + from + "\" object"
      );
    }
  }

  public native OksConfigObject create_object(String db, OksConfigObject at, String class_name, String object_id) throws config.SystemException, config.NotFoundException, config.NotAllowedException;

  public config.ConfigObject create(String db, config.ConfigObject at, String class_name, String object_id) throws config.SystemException, config.NotFoundException, config.NotAllowedException {
    if(db != null) {
      OksConfigObject o = create_object(db, null, class_name, object_id);
      if(o != null) {
        o.m_db = this;
        return new config.ConfigObject(o);
      }
      else {
        throw new config.SystemException(
          "failed to create object \"" + object_id + "@" + class_name + "\" at \"" + db + "\""
        );
      }
    }
    else {
      OksConfigObject o = create_object(null, (OksConfigObject)(at.implementation()), class_name, object_id);
      if(o != null) {
        o.m_db = this;
        return new config.ConfigObject(o);
      }
      else {
        throw new config.SystemException(
          "failed to create object \"" + object_id + "@" + class_name + "\" at database of \"" + at.UID() + "@" + at.class_name() + "\""
        );
      }
    }
  }

  public native void destroy_object(OksConfigObject object) throws config.SystemException, config.NotFoundException, config.NotAllowedException;

  public void destroy(config.ConfigObject object) throws config.SystemException, config.NotFoundException, config.NotAllowedException {
    destroy_object((OksConfigObject)object.implementation());
  }

  public native void move_object(OksConfigObject object, String at) throws config.SystemException, config.NotFoundException, config.NotAllowedException;

  public void move(config.ConfigObject object, String at) throws config.SystemException, config.NotFoundException, config.NotAllowedException {
    move_object((OksConfigObject)object.implementation(), at);
  }

  public native void subscribe(String classes[], String[] objects, Configuration conf) throws config.SystemException;

  public void subscribe(TreeSet<String> classes, Hashtable<String,TreeSet<String>> objects, Configuration conf) throws config.SystemException {
    m_config = conf;

    String[] cl = null;
    String[] ol = null;

    if(classes != null) {
      int j = 0;
      cl = new String[classes.size()];
      for(Iterator<String> i = classes.iterator(); i.hasNext(); ++j) {
        cl[j] = (String)i.next();
      }
    }

    if(objects != null) {
      Enumeration<String> keys;


        // calculate total number of objects

      int count = 0;

      for(keys = objects.keys(); keys.hasMoreElements() ;) {
        String cl_name = (String)keys.nextElement();
        java.util.TreeSet<String> cl_objs = objects.get(cl_name);
        count += cl_objs.size();
      }

      if(count > 0) {
        ol = new String[count*2];
      }


        // iterate by all objects and fill array of rdb objects

      count = 0;

      for(keys = objects.keys(); keys.hasMoreElements() ;) {
        String cl_name = (String)keys.nextElement();
        java.util.TreeSet<String> cl_objs = objects.get(cl_name);
	
        for(Iterator<String> i = cl_objs.iterator(); i.hasNext(); ) {
          ol[count++] = (String)i.next();
          ol[count++] = cl_name;
        }
      }
    }

    subscribe(cl, ol, m_config);
  }

  public native void unsubscribe() throws config.SystemException;
  
  public config.class_t get(String class_name, boolean direct_only) throws config.SystemException, config.NotFoundException
  {
    throw new config.SystemException("the method \'class_t OksConfiguration.get(String, boolean)\' is not implemented; use rdbconfig plugin to work with OKS schema description");
  }

	public config.Version[] get_changes() throws config.SystemException {
		throw new config.SystemException(
				"the method \'config.Version[] get_changes()\' is not implemented yet; use rdbconfig plugin to work wirh versions");
	}

	public config.Version[] get_versions(String since, String until, config.Version.QueryType type, boolean skip) throws config.SystemException {
		throw new config.SystemException(
				"the method \'config.Version[] get_versions()\' is not implemented yet; use rdbconfig plugin to work with versions");
	}
}
