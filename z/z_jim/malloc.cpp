
{
    Arena arena = {};

    Foo *foo = arena_alloc(sizeof(Foo), &arena);

    // blah blah


    Foo *foo2 = arena_alloc(sizeof(Foo), &arena);
    Foo *foo3 = arena_alloc(sizeof(Foo), &arena);

    // blah blah 

    Foo *foo5 = arena_alloc(sizeof(Foo), &arena);
    Foo *foo6 = arena_alloc(sizeof(Foo), &arena);


    // ...


    arena_free(&arena);
}
