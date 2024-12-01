Objects
-------
i have two kinds of objects

- objects that behave like primitives (in my head, i imagine they *are* primitives)
  vec3 a = { 1.0f, 2.0f, 3.0f };
  vec3 b = a; // works just like a was a float; no aliasing! i shouldn't have to think!
  a += b; 

- plain old data structs
  (no constructors, no destructors, no operator overloading, no blah blah blah)
  Mesh mesh = {}; // clears struct to default values*
  Camera camera_2D = make_Camera2D(...); // fills struct with something nice

  *i almost always use 0 (unspecified) default value, but String buffer members are a big exception
  struct Foo {
      // String bar = { (char *) calloc(1, BAR_LENGTH) };
      STRING_STRUCT_CALLOC(bar, BAR_LENGTH);
  }
  

asserts, etc.
-------------
- i love ASSERT(...);
  ASSERT(2 + 2 == 5); // crash! tell me where! break if i'm in a debugger!

- i like (automatic) bounds checks
  vec3 v;
  v[3] = 5; // crash!

strings
-------
- prefer my String struct ({ char *data; uint length; }) over "C string" (null-terminated char *)
- printf("hi!\n"), messagef("hello!"), etc. is okay tho (at least FORNOW)

