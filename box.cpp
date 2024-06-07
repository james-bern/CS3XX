template <uint D> struct Box {
    vecD min;
    vecD max;
};

typedef Box<2> box2;
typedef Box<3> box3;
#define boxD Box<D>
#define tuD template <uint D>
#define tuDb tuD boxD

tuDb BOUNDING_BOX_MAXIMALLY_NEGATIVE_AREA() {
    Box<D> result;
    for_(d, D) {
        result.min[d] = HUGE_VAL;
        result.max[d] = -HUGE_VAL;
    }
    return result;
}

tuD bool box_contains(boxD box, vecD point) {
    for_(d, D) {
        if (!IS_BETWEEN(point[d], box.min[d], box.max[d])) return false;
    }
    return true;
}

tuD bool box_contains(boxD box, boxD other) {
    for_(d, D) {
        if (box.min[d] > other.min[d]) return false;
        if (box.max[d] < other.max[d]) return false;
    }
    return true;
}

tuD void bounding_box_add_point(boxD *box, vecD p) {
    for_(d, D) {
        box->min[d] = MIN(box->min[d], p[d]);
        box->max[d] = MAX(box->max[d], p[d]);
    }
}

tuDb bounding_box_union(boxD box, boxD other) {
    for_(d, D) {
        box.min[d] = MIN(box.min[d], other.min[d]);
        box.max[d] = MAX(box.max[d], other.max[d]);
    }
    return box;
}

void eso_vertex(real x, real y);
void eso_box__SOUP_QUADS(box2 box) {
    eso_vertex(box.min.x, box.min.y);
    eso_vertex(box.min.x, box.max.y);
    eso_vertex(box.max.x, box.max.y);
    eso_vertex(box.max.x, box.min.y);
}
