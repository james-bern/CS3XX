// TODO: consider different ElephantStack internals

template <typename T> struct ElephantStack {
    Stack<T> _undo_stack;
    Stack<T> _redo_stack;
};

template <typename T> uint elephant_length_undo(ElephantStack<T> *elephant) {
    return elephant->_undo_stack.length;
}

template <typename T> uint elephant_length_redo(ElephantStack<T> *elephant) {
    return elephant->_redo_stack.length;
}

template <typename T> bool elephant_is_empty_undo(ElephantStack<T> *elephant) {
    return (elephant_length_undo(elephant) == 0);
}

template <typename T> bool elephant_is_empty_redo(ElephantStack<T> *elephant) {
    return (elephant_length_redo(elephant) == 0);
}

template <typename T> T *elephant_undo_ptr_begin(ElephantStack<T> *elephant) {
    return elephant->_undo_stack.array;
}

template <typename T> T *elephant_undo_ptr_one_past_end(ElephantStack<T> *elephant) {
    return elephant->_undo_stack.array + elephant->_undo_stack.length;
}

template <typename T> T *elephant_redo_ptr_one_past_end(ElephantStack<T> *elephant) {
    return elephant->_redo_stack.array + elephant->_redo_stack.length;
}

template <typename T> T *elephant_peek_undo(ElephantStack<T> *elephant) {
    ASSERT(elephant->_undo_stack.length);
    return elephant_undo_ptr_one_past_end(elephant) - 1;
}

template <typename T> T *elephant_peek_redo(ElephantStack<T> *elephant) {
    ASSERT(elephant->_redo_stack.length);
    return elephant_redo_ptr_one_past_end(elephant) - 1;
}

template <typename T> T *elephant_pop_undo_onto_redo(ElephantStack<T> *elephant) {
    ASSERT(elephant->_undo_stack.length);
    stack_push(&elephant->_redo_stack, stack_pop(&elephant->_undo_stack));
    return elephant_peek_redo(elephant);
}

template <typename T> T *elephant_pop_redo_onto_undo(ElephantStack<T> *elephant) {
    ASSERT(elephant->_redo_stack.length);
    stack_push(&elephant->_undo_stack, stack_pop(&elephant->_redo_stack));
    return elephant_peek_undo(elephant);
}

template <typename T> void elephant_clear_redo(ElephantStack<T> *elephant) {
    stack_free_AND_zero(&elephant->_redo_stack);
}

template <typename T> void elephant_push_undo_clear_redo(ElephantStack<T> *elephant, T item) {
    stack_push(&elephant->_undo_stack, item);
    elephant_clear_redo(elephant);
}

