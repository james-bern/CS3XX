template <int D> struct BoundingBox {
    SnailVector<D> min;
    SnailVector<D> max;
};

typedef BoundingBox<2> box2;
typedef BoundingBox<3> box3;

template <int D> BoundingBox<D> BOUNDING_BOX_MAXIMALLY_NEGATIVE_AREA() {
    BoundingBox<D> result;
    for (uint32 d = 0; d < D; ++d) {
        result.min[d] = HUGE_VAL;
        result.max[d] = -HUGE_VAL;
    }
    return result;
}

template <int D> void bounding_box_add_point(BoundingBox<D> *bounding_box, SnailVector<D> p) {
    for (uint32 d = 0; d < D; ++d) {
        bounding_box->min[d] = MIN(bounding_box->min[d], p[d]);
        bounding_box->max[d] = MAX(bounding_box->max[d], p[d]);
    }
}


// TODO: extend to BoundingBox<D>

void pprint(box2 bounding_box) {
    printf("(%f, %f) <-> (%f, %f)\n", bounding_box.min[0], bounding_box.min[1], bounding_box.max[0], bounding_box.max[1]);
}

void bounding_box_center(box2 bounding_box, real32 *x, real32 *y) {
    *x = (bounding_box.min[0] + bounding_box.max[0]) / 2;
    *y = (bounding_box.min[1] + bounding_box.max[1]) / 2;
}

bool bounding_box_contains(box2 outer, box2 inner) {
    for (uint32 d = 0; d < 2; ++d) {
        if (outer.min[d] > inner.min[d]) return false;
        if (outer.max[d] < inner.max[d]) return false;
    }
    return true;
}

bool bounding_box_contains(box2 bounding_box, vec2 point) {
    for (uint32 d = 0; d < 2; ++d) {
        if (!IS_BETWEEN(point[d], bounding_box.min[d], bounding_box.max[d])) return false;
    }
    return true;
}

vec2 bounding_box_clamp(vec2 p, box2 bounding_box) {
    for (uint32 d = 0; d < 2; ++d) {
        p[d] = CLAMP(p[d], bounding_box.min[d], bounding_box.max[d]);
    }
    return p;
}

box2 bounding_box_union(box2 a, box2 b) {
    for (uint32 d = 0; d < 2; ++d) {
        a.min[d] = MIN(a.min[d], b.min[d]);
        a.max[d] = MAX(a.max[d], b.max[d]);
    }
    return a;
}
