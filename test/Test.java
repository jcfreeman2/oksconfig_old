public class Test {

  private static void show_svalues(config.ConfigObject obj, String dx) throws config.NotFoundException, config.SystemException {
    System.out.println(dx + "Dump values of object \'" + obj.UID() + "@" + obj.class_name() + "\' attributes:");
    System.out.println(dx + " - \"uc\" value is \'" + (char)obj.get_byte("uc") + "\' (byte type)");
    System.out.println(dx + " - \"b\" value is \'" + obj.get_bool("b") + "\' (boolean type)");
    System.out.println(dx + " - \"us\" value is \'" + obj.get_short("us") + "\' (unsigned 16 bits integer type)");
    System.out.println(dx + " - \"ss\" value is \'" + obj.get_short("ss") + "\' (signed 16 bits integer type)");
    System.out.println(dx + " - \"ui\" value is \'" + obj.get_int("ui") + "\' (unsigned 32 bits integer type)");
    System.out.println(dx + " - \"si\" value is \'" + obj.get_int("si") + "\' (signed 32 bits integer type)");
    System.out.println(dx + " - \"ul\" value is \'" + obj.get_long("ul") + "\' (unsigned 64 bits integer type)");
    System.out.println(dx + " - \"sl\" value is \'" + obj.get_long("sl") + "\' (signed 64 bits integer type)");
    System.out.println(dx + " - \"f\" value is \'" + obj.get_float("f") + "\' (float type)");
    System.out.println(dx + " - \"d\" value is \'" + obj.get_double("d") + "\' (double type)");
    System.out.println(dx + " - \"str\" value is \'" + obj.get_string("str") + "\' (string type)");
    System.out.println(dx + " - \"cls\" value is \'" + obj.get_string("cls") + "\' (class type)");
    System.out.println("");
  }

  private static void show_mvalues(config.ConfigObject obj, String dx) throws config.NotFoundException, config.SystemException {
    System.out.println(dx + "Dump values of object \'" + obj.UID() + "@" + obj.class_name() + "\' attributes:");


    short[] numbers = obj.get_shorts("ui");

    System.out.println(dx + " - \"ui\" value consists of \'" +  numbers.length + "\' items (ushort type)");
    for(int i = 0; i < numbers.length; i++) {
      System.out.println(dx + "  " + (i+1) + ") " + numbers[i]);
    }

    String[] strings = obj.get_strings("str");

    System.out.println(dx + " - \"str\" value consists of \'" +  numbers.length + "\' items (string type)");
    for(int i = 0; i < strings.length; i++) {
      System.out.println(dx + "  " + (i+1) + ") \'" + strings[i] + "\'");
    }

    System.out.println("");
  }


  private static void show_rvalues(config.ConfigObject obj) throws config.NotFoundException, config.SystemException {
    System.out.println("Dump values of object \'" + obj.UID() + "@" + obj.class_name() + "\' relationships:");

    config.ConfigObject obj1 = obj.get_object("prim");
    System.out.println(" - \"prim\" value is \'" + obj1.UID() + "@" + obj1.class_name() + "\'");
    show_svalues(obj1, "  ");

    config.ConfigObject[] objs = obj.get_objects("mult");
    System.out.println(" - \"mult\" value contains " + objs.length + " items:");
    for(int i = 0; i < objs.length; i++) {
      System.out.println("  " + (i+1) + ") \'" + objs[i].UID() + "@" + objs[i].class_name() + "\'");
      show_mvalues(objs[i], "    ");
    }

    System.out.println("");
  }

  public static void main(String args[]) {

    if(args.length == 0) {
      System.err.println( "ERROR [Test.main()]: no arguments given (file[:file*])" );
      System.exit( 1 );
    }


    try {

        // create configuration object and check it's status

      config.Configuration db = new config.Configuration(args[0]);

      if(db.loaded() == false) {
        System.err.println( "ERROR [Test.main()]: failed to load database \'" + args[0] + "\'" );
        System.exit(1);
      }

        // get object with single-value attributes

      config.ConfigObject obj1 = db.get_object("TestPrimitive", "primitive");

      if(obj1 == null) {
        System.err.println( "ERROR [Test.main()]: can not get object \'primitive@TestPrimitive\'" );
        System.exit(1);
      }
      else {
        show_svalues(obj1, "");
      }


        // get object with multi-value attributes

      config.ConfigObject obj2 = db.get_object("TestMultiple", "multiple");

      if(obj1 == null) {
        System.err.println( "ERROR [Test.main()]: can not get object \'multiple@TestMultiple\'" );
        System.exit(1);
      }
      else {
        show_mvalues(obj2, "");
      }


        // get object with relationships

      config.ConfigObject obj3 = db.get_object("TestRelation", "relation");

      if(obj3 == null) {
        System.err.println( "ERROR [Test.main()]: can not get object \'relation@TestRelation\'" );
        System.exit(1);
      }
      else {
        show_rvalues(obj3);
      }
    }
    catch (config.SystemException ex) {
      System.err.println( "ERROR [Test.main()]: caught \'config.SystemException\':");
      System.err.println( "*** " + ex.getMessage() + " ***" );
      System.err.println( "exiting Test.main() ..." );
      System.exit( 1 );
    }
    catch (config.NotFoundException ex) {
      System.err.println( "ERROR [Test.main()]: caught \'config.NotFoundException\':");
      System.err.println( "*** " + ex.getMessage() + " ***" );
      System.err.println( "exiting Test.main() ..." );
      System.exit( 1 );
    }
  }
}
