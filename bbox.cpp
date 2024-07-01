template <uint D> struct BoundingBox {
    vecD min;
    vecD max;
};

typedef BoundingBox<2> bbox2;
typedef BoundingBox<3> bbox3;
#define bboxD BoundingBox<D>
#define tuDb tuD bboxD

tuDb BOUNDING_BOX_MAXIMALLY_NEGATIVE_AREA() {
    BoundingBox<D> result;
    for_(d, D) {
        result.min[d] = HUGE_VAL;
        result.max[d] = -HUGE_VAL;
    }
    return result;
}

tuD bool bbox_contains(bboxD A, vecD point) {
    for_(d, D) {
        if (!IS_BETWEEN_LOOSE(point[d], A.min[d], A.max[d])) return false;
    }
    return true;
}

tuD bool bbox_contains(bboxD A, bboxD other) {
    for_(d, D) {
        if (A.min[d] > other.min[d]) return false;
        if (A.max[d] < other.max[d]) return false;
    }
    return true;
}

// // add
tuDb &operator += (bboxD &A, vecD p) {
    for_(d, D) {
        A.min[d] = MIN(A.min[d], p[d]);
        A.max[d] = MAX(A.max[d], p[d]);
    }
    return A;
}

// // union
// +
tuDb  operator +  (bboxD A, bboxD B) {
    bboxD result;
    for_(d, D) {
        result.min[d] = MIN(A.min[d], B.min[d]);
        result.max[d] = MAX(A.max[d], B.max[d]);
    }
    return result;
}
// +=
tuDb &operator += (bboxD &A, bboxD B) {
    A = A + B;
    return A;
}

void eso_vertex(real x, real y);
void eso_bbox_SOUP_QUADS(bbox2 A) {
    eso_vertex(A.min.x, A.min.y);
    eso_vertex(A.min.x, A.max.y);
    eso_vertex(A.max.x, A.max.y);
    eso_vertex(A.max.x, A.min.y);
}
