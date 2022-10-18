#include <stdlib.h>
#include <iostream>

#include "config/Configuration.h"
#include "config/ConfigObject.h"
#include "config/Schema.h"


static const char *
all2string(bool direct)
{
  return (direct ? "direct " : "all ");
}

static void
show_info(Configuration& config, const char * name, bool direct)
{
  const daq::config::class_t& c = config.get_class_info(name, direct);

  std::cout << "class " << c.p_name << std::endl;

  if (!c.p_superclasses.empty())
    {
      std::cout << "  " << all2string(direct) << "superclasses:\n";
      for (const auto& x : c.p_superclasses)
        std::cout << "    " << x << std::endl;
    }

  if (!c.p_subclasses.empty())
    {
      std::cout << "  " << all2string(direct) << "subclasses:\n";
      for (const auto& x : c.p_subclasses)
        std::cout << "    " << x << std::endl;
    }

  if (!c.p_attributes.empty())
    {
      std::cout << "  " << all2string(direct) << "attributes:\n";
      for (const auto& x : c.p_attributes)
        {
          x.print(std::cout, "    ");
          std::cout << std::endl;
        }
    }

  if (!c.p_relationships.empty())
    {
      std::cout << "  " << all2string(direct) << "relationships:\n";
      for (const auto& x : c.p_relationships)
        {
          x.print(std::cout, "    ");
          std::cout << std::endl;
        }
    }

  std::cout << std::endl;
}

static void
check_meta_info(Configuration& config)
{
  show_info(config, "A", true);
  show_info(config, "B", true);
  show_info(config, "B", false);
  show_info(config, "C", true);
  show_info(config, "C", false);
}

template<class T>
void get_attribute(ConfigObject& obj, const std::string& name)
{
  try {
    T value;
    obj.get(name,value);
    std::cout << name << " = " << value << std::endl;
  }
  catch(ers::Issue & ex) {
    std::cerr << "ERROR: cannot find attribute " << name << ":\n" << ex << std::endl;
  }
}

static void show_primitive(ConfigObject& obj)
{
  get_attribute<uint8_t>(obj, "uc");
  get_attribute<bool>(obj, "b");
  get_attribute<uint16_t>(obj, "us");
  get_attribute<int16_t>(obj, "ss");
  get_attribute<uint32_t>(obj, "ui");
  get_attribute<int32_t>(obj, "si");
  get_attribute<uint64_t>(obj, "ul");
  get_attribute<int64_t>(obj, "sl");
  get_attribute<float>(obj, "f");
  get_attribute<double>(obj, "d");
  get_attribute<std::string>(obj, "str");
  get_attribute<std::string>(obj, "cls");
}

static void check_primitives(Configuration& config)
{
  ConfigObject obj;

  try {
    config.get("TestPrimitive", "primitive", obj);
    show_primitive(obj);
    obj.print_ref(std::cout, config);
  }
  catch(ers::Issue & ex) {
    std::cerr << "ERROR: cannot find object 'primitive' of type 'TestPrimitive':\n" << ex << std::endl;
  }
}

static void show_multiple(ConfigObject& obj)
{
  try {
    std::vector<unsigned short> numbers;
    obj.get("ui", numbers);
    for(size_t i = 0; i < numbers.size(); i++) {
      std::cout << "ui[" << i << "] = " << numbers[i] << std::endl;
    }
    std::cout << std::endl;
  }
  catch(ers::Issue & ex) {
    std::cerr << "ERROR: cannot find attribute 'ui':\n" << ex << std::endl;
  }

  try {
    std::vector<std::string> strings;
    obj.get("str", strings);
    for(size_t i = 0; i < strings.size(); i++) {
      std::cout << "str[" << i << "] = " << strings[i] << std::endl;
    }
    std::cout << std::endl;
  }
  catch(ers::Issue & ex) {
    std::cerr << "ERROR: cannot find attribute 'str':\n" << ex << std::endl;
  }
}

static void check_multiple(Configuration& config)
{
  try {
    ConfigObject obj;
    config.get("TestMultiple", "multiple", obj);
    show_multiple(obj);
    obj.print_ref(std::cout, config);
  }
  catch(ers::Issue & ex) {
    std::cerr << "ERROR: cannot find object 'multiple' of type 'TestMultiple':\n" << ex << std::endl;
  }
}

static void check_relations(Configuration& config)
{
  ConfigObject obj;

  try {
    config.get("TestRelation", "relation", obj);
    obj.print_ref(std::cout, config);
  }
  catch(ers::Issue & ex) {
    std::cerr << "ERROR: cannot find object 'relation' of type 'TestRelation':\n" << ex << std::endl;
    return;
  }


  try {
    ConfigObject prim;
    obj.get("prim", prim);
    show_primitive(prim);
    prim.print_ref(std::cout, config);
  }
  catch(ers::Issue & ex) {
    std::cerr << "ERROR: cannot find relation 'prim' in object 'relation':\n" << ex << std::endl;
  }

  try {
    std::vector<ConfigObject> mult;
    obj.get("mult", mult);

    for(std::vector<ConfigObject>::iterator it = mult.begin(); it != mult.end(); ++it) {
      show_multiple(*it);
      (*it).print_ref(std::cout, config);
    }
  }
  catch(ers::Issue & ex) {
    std::cerr << "ERROR: cannot find relation 'mult' in object 'relation':\n" << ex << std::endl;
  }
}

static void check_enums(Configuration& config)
{
  ConfigObject obj;
  
  try {
    config.get("EnumTest", "e1", obj);
  }
  catch(ers::Issue & ex) {
    std::cerr << "ERROR: cannot find 'e1' of type 'EnumTest':\n" << ex << std::endl;
    return;
  }
    
  try {
    std::string value;
    obj.get("enum1", value);
    std::cout << "enum1 = " << value << std::endl;
  }
  catch(ers::Issue & ex) {
    std::cerr << "ERROR: cannot find 'enum1' attribute:\n" << ex << std::endl;
    return;
  }

  try {
    std::vector<std::string> values;
    std::cout << "enum2 = ";
    for(std::vector<std::string>::iterator it = values.begin(); it != values.end(); ++it) {
      std::cout << *it << " ";
    }
    std::cout << std::endl;
  }
  catch(ers::Issue & ex) {
    std::cerr << "ERROR: cannot find 'enum2' attribute:\n" << ex << std::endl;
    return;
  }
}


int
main(int argc, char *argv[])
{
  std::string dbname = "oksconfig:../../oksconfig/test/okstest.data.xml:../../oksconfig/test/test2.data.xml";

  if (argc != 1)
    dbname = argv[1];

  try
    {
      Configuration config(dbname);

      check_primitives(config);
      check_multiple(config);
      check_relations(config);
      check_enums(config);
      check_meta_info(config);

      try
        {
          ConfigObject obj;
          config.get("OtherClass", "test2", obj);
          std::cout << "could get Test2 object" << std::endl;
        }
      catch (ers::Issue & ex)
        {
          std::cerr << "ERROR: cannot get Test2 object:\n" << ex << std::endl;
        }
    }
  catch (const ers::Issue & ex)
    {
      std::cerr << "ERROR: " << ex << std::endl;
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
