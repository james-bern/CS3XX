struct HistoryState {
    ManifoldManifold *manifold_manifold;
    FancyMesh fancy_mesh;
};

struct History {
    List<HistoryState> undo_stack;
    List<HistoryState> redo_stack;
};
void history_record_state(History *history, ManifoldManifold **manifold_manifold, FancyMesh *fancy_mesh) {
    list_push_back(&history->undo_stack, { *manifold_manifold, *fancy_mesh });
    { // free redo_stack
        for (u32 i = 0; i < history->redo_stack.length; ++i) {
            fancy_mesh_free(&history->redo_stack.data[i].fancy_mesh);
            // TODO: manifold_manifold
        }
        list_free(&history->redo_stack);
    }
}
void history_undo(History *history, ManifoldManifold **manifold_manifold, FancyMesh *fancy_mesh) {
    if (history->undo_stack.length != 0) {
        list_push_back(&history->redo_stack, { *manifold_manifold, *fancy_mesh });
        HistoryState state = list_pop_back(&history->undo_stack);
        *manifold_manifold = state.manifold_manifold;
        *fancy_mesh = state.fancy_mesh;
    } else {
        conversation_messagef("[history] undo stack is empty");
    }
}
void history_redo(History *history, ManifoldManifold **manifold_manifold, FancyMesh *fancy_mesh) {
    if (history->redo_stack.length != 0) {
        list_push_back(&history->undo_stack, { *manifold_manifold, *fancy_mesh });
        HistoryState state = list_pop_back(&history->redo_stack);
        *manifold_manifold = state.manifold_manifold;
        *fancy_mesh = state.fancy_mesh;
    } else {
        conversation_messagef("[history] redo stack is empty");
    }
}
void history_free(History *history) {
    { // free undo_stack
        for (u32 i = 0; i < history->undo_stack.length; ++i) {
            fancy_mesh_free(&history->undo_stack.data[i].fancy_mesh);
            // TODO: manifold_manifold
        }
        list_free(&history->undo_stack);
    }

    { // free redo_stack
        for (u32 i = 0; i < history->redo_stack.length; ++i) {
            fancy_mesh_free(&history->redo_stack.data[i].fancy_mesh);
            // TODO: manifold_manifold
        }
        list_free(&history->redo_stack);
    }
}
