import java.util.*;
class Test {
    static class Foo {
        int num;

        Foo (int num) {
            this.num = num;
        }
    }

    public static void main(String[] arguments) {
        ArrayList<Foo> list = new ArrayList<>();
        for (int i = 0; i < 300; ++i) {
            for (int j = 0; j < 300; ++j) {
                for (int k = 0; k < 300; ++k) {
                    list.add(new Foo(i + j + k));
                }
            }
        }
        System.out.println(list.get(26999999).num);
    }
}
