template struct BoundingCircle {
    vecD center;
    float radius;
};

typedef BoundingCircle bcircle;
#define tuDbcircle tuD bcircleD

tuDbcircle BOUNDING_CIRCLE_MAXIMALLY_NEGATIVE_AREA() {
    return { { 0, 0 }, -1000 };
}

tuD bool bbox_contains(bboxD A, vecD point) {
    for_(d, D) {
        if (!IS_BETWEEN(point[d], A.min[d], A.max[d])) return false;
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
