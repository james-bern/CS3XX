////////////////////////////////////////////////////////////////////////////////
// vectors and matrices ////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

template <uint D> union Vector {
    real data[D];
    real &operator [](uint index) { return data[index]; }
};

template <> union Vector<2> {
    real data[2];
    struct { real x, y; };
    real &operator [](uint index) { return data[index]; }
};

template <> union Vector<3> {
    real data[3];
    struct { real x, y, z; };
    real &operator [](uint index) { return data[index]; }
};

template <> union Vector<4> {
    real data[4];
    struct { real x, y, z, w; };
    real &operator [](uint index) { return data[index]; }
};


template <uint D> union Matrix {
    real data[D * D];
    real &operator ()(uint row, uint col) { return data[D * row + col]; }
    const real &operator ()(uint row, uint col) const { return data[D * row + col]; }
};



template <uint D> union SnailTupleOfUnsignedInts {
    uint data[D];
    uint &operator [](uint index) { return data[index]; }
};

template <> union SnailTupleOfUnsignedInts<2> {
    uint data[2];
    struct { uint i, j; };
    uint &operator [](uint index) { return data[index]; }
};

template <> union SnailTupleOfUnsignedInts<3> {
    uint data[3];
    struct { uint i, j, k; };
    uint &operator [](uint index) { return data[index]; }
};

typedef SnailTupleOfUnsignedInts<2> uint2;
typedef SnailTupleOfUnsignedInts<3> uint3;
typedef SnailTupleOfUnsignedInts<4> uint4;

// "constructors" //////////////////////////////////////////////////////////////

vec2 V2(real x, real y) { return { x, y }; }
vec3 V3(real x, real y, real z) { return { x, y, z }; }
vec4 V4(real x, real y, real z, real w) { return { x, y, z, w }; }
vec3 V3(vec2 xy, real z) { return { xy.x, xy.y, z }; }
vec4 V4(vec3 xyz, real w) { return { xyz.x, xyz.y, xyz.z, w }; }
vec2 V2(real x) { return { x, x }; }
vec3 V3(real x) { return { x, x, x }; }
vec4 V4(real x) { return { x, x, x, x }; }
vec2 _V2(vec3 xyz) { return { xyz.x, xyz.y }; }
vec3 _V3(vec4 xyzw) { return { xyzw.x, xyzw.y, xyzw.z }; }

mat2 M2(real a0, real a1, real a2, real a3) {
    return { a0, a1, a2, a3 };
}
mat3 M3(real a0, real a1, real a2, real a3, real a4, real a5, real a6, real a7, real a8) {
    return { a0, a1, a2, a3, a4, a5, a6, a7, a8 };
}
mat4 M4(real a0, real a1, real a2, real a3, real a4, real a5, real a6, real a7, real a8, real a9, real a10, real a11, real a12, real a13, real a14, real a15) {
    return { a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15 };
}

mat2 hstack(vec2 col0, vec2 col1) { return { col0.x, col1.x, col0.y, col1.y }; }
mat3 hstack(vec3 col0, vec3 col1, vec3 col2) { return { col0.x, col1.x, col2.x, col0.y, col1.y, col2.y, col0.z, col1.z, col2.z }; }
mat4 hstack(vec4 col0, vec4 col1, vec4 col2, vec4 col3) { return { col0.x, col1.x, col2.x, col3.x, col0.y, col1.y, col2.y, col3.y, col0.z, col1.z, col2.z, col3.z, col0.w, col1.w, col2.w, col3.w }; }

// arithmetic operators ////////////////////////////////////////////////////////

// vectors
tuDv  operator +  (vecD A, vecD B) {
    vecD result;
    for_(i, D) {
        result[i] = A.data[i] + B.data[i];
    }
    return result;
}
tuDv &operator += (vecD &A, vecD B) {
    A = A + B;
    return A;
}

tuDv  operator -  (vecD A, vecD B) {
    vecD result;
    for_(i, D) {
        result[i] = A.data[i] - B.data[i];
    }
    return result;
}
tuDv &operator -= (vecD &A, vecD B) {
    A = A - B;
    return A;
}

tuDv  operator *  (real scalar, vecD A) {
    vecD result;
    for_(i, D) {
        result[i]  = scalar * A.data[i];
    }
    return result;
}
tuDv  operator *  (vecD A, real scalar) {
    vecD result = scalar * A;
    return result;
}
tuDv &operator *= (vecD &A, real scalar) {
    A = scalar * A;
    return A;
}
tuDv  operator -  (vecD A) {
    return -1 * A;
}

tuDv  operator /  (vecD A, real scalar) {
    vecD result;
    for_(i, D) {
        result[i]  = A[i] / scalar;
    }
    return result;
}
tuDv &operator /= (vecD &v, real scalar) {
    v = v / scalar;
    return v;
}

// matrices
tuDm  operator +  (matD A, matD B) {
    matD result = {};
    for_(k, D * D) {
        result.data[k] = A.data[k] + B.data[k];
    }
    return result;
}
tuDm &operator += (matD &A, matD B) {
    A = A + B;
    return A;
}

tuDm  operator -  (matD A, matD B) {
    matD result = {};
    for_(i, D * D) {
        result.data[i] = A.data[i] - B.data[i];
    }
    return result;
}
tuDm &operator -= (matD &A, matD B) {
    A = A + B;
    return A;
}

tuDm  operator *  (matD A, matD B) {
    matD result = {};
    for_(row, D) {
        for_(col, D) {
            for_(i, D) {
                result(row, col) += A(row, i) * B(i, col);
            }
        }
    }
    return result;
}
tuDm &operator *= (matD &A, matD B) {
    A = A * B;
    return A;
}
tuDv  operator *  (matD A, vecD b) { // A b
    vecD result = {};
    for_(row, D) {
        for_(col, D) {
            result[row] += A(row, col) * b[col];
        }
    }
    return result;
}
tuDv  operator *  (vecD b, matD A) { // b^D A
    vecD result = {};
    for_(row, D) {
        for_(col, D) {
            result[row] += A(col, row) * b[col];
        }
    }
    return result;
}
tuDm  operator *  (real scalar, matD M) {
    matD result = {};
    for_(k, D * D) {
        result.data[k] = scalar * M.data[k];
    }
    return result;
}
tuDm  operator *  (matD M, real scalar) {
    return scalar * M;
}
tuDm &operator *= (matD &M, real scalar) {
    M = scalar * M;
    return M;
}
tuDm  operator -  (matD M) {
    return -1 * M;
}

tuDm  operator /  (matD M, real scalar) {
    return (1 / scalar) * M;
}
tuDm &operator /= (matD &M, real scalar) {
    M = M / scalar;
    return M;
}

// important vector functions //////////////////////////////////////////////////

template <uint D> real dot(vecD A, vecD B) {
    real result = 0.0f;
    for_(i, D) {
        result += A.data[i] * B.data[i];
    }
    return result;
}
tuDm outer(vecD A, vecD B) {
    matD result = {};
    for_(row, D) {
        for_(col, D) {
            result(row, col) = A[row] * B[col];
        }
    }
    return result;
}

real cross(vec2 A, vec2 B) {
    return A.x * B.y - A.y * B.x;
}
vec3 cross(vec3 A, vec3 B) {
    return { A.y * B.z - A.z * B.y, A.z * B.x - A.x * B.z, A.x * B.y - A.y * B.x };
}

template <uint D> real squaredNorm(vecD A) {
    return dot(A, A);
}
template <uint D> real norm(vecD A) {
    return sqrt(squaredNorm(A));
}
template <uint D> real sum(vecD A) {
    real result = 0.0;
    for_(i, D) result += A[i];
    return result;
}
tuDv normalized(vecD A) {
    real norm_A = norm(A);
    // ASSERT(fabs(norm_v) > 1e-7);
    return (1 / norm_A) * A;
}

// ALIASES
// template <uint D> real length(vecD v) { return norm(v); }
// template <uint D> real squared_length(vecD v) { return squaredNorm(v); }

// important matrix functions //////////////////////////////////////////////////

tuDm transpose(matD M) {
    matD result = {};
    for_(row, D) {
        for_(col, D) {
            result(row, col) = M(col, row);
        }
    }
    return result;
}

real determinant(mat2 M) {
    return M(0, 0) * M(1, 1) - M(0, 1) * M(1, 0);
}
real determinant(mat3 M) {
    return M(0, 0) * (M(1, 1) * M(2, 2) - M(2, 1) * M(1, 2))
        - M(0, 1) * (M(1, 0) * M(2, 2) - M(1, 2) * M(2, 0))
        + M(0, 2) * (M(1, 0) * M(2, 1) - M(1, 1) * M(2, 0));
}
real determinant(mat4 M) {
    real A2323 = M(2, 2) * M(3, 3) - M(2, 3) * M(3, 2);
    real A1323 = M(2, 1) * M(3, 3) - M(2, 3) * M(3, 1);
    real A1223 = M(2, 1) * M(3, 2) - M(2, 2) * M(3, 1);
    real A0323 = M(2, 0) * M(3, 3) - M(2, 3) * M(3, 0);
    real A0223 = M(2, 0) * M(3, 2) - M(2, 2) * M(3, 0);
    real A0123 = M(2, 0) * M(3, 1) - M(2, 1) * M(3, 0);
    return M(0, 0) * ( M(1, 1) * A2323 - M(1, 2) * A1323 + M(1, 3) * A1223 ) 
        - M(0, 1) * ( M(1, 0) * A2323 - M(1, 2) * A0323 + M(1, 3) * A0223 ) 
        + M(0, 2) * ( M(1, 0) * A1323 - M(1, 1) * A0323 + M(1, 3) * A0123 ) 
        - M(0, 3) * ( M(1, 0) * A1223 - M(1, 1) * A0223 + M(1, 2) * A0123 ) ;
}

mat2 inverse(mat2 M) {
    real invdet = 1 / determinant(M);
    return { invdet * M(1, 1), 
        invdet * -M(0, 1), 
        invdet * -M(1, 0), 
        invdet * M(0, 0) };
}
mat3 inverse(mat3 M) {
    real invdet = 1 / determinant(M);
    return { invdet * (M(1, 1) * M(2, 2) - M(2, 1) * M(1, 2)),
        invdet * (M(0, 2) * M(2, 1) - M(0, 1) * M(2, 2)),
        invdet * (M(0, 1) * M(1, 2) - M(0, 2) * M(1, 1)),
        invdet * (M(1, 2) * M(2, 0) - M(1, 0) * M(2, 2)),
        invdet * (M(0, 0) * M(2, 2) - M(0, 2) * M(2, 0)),
        invdet * (M(1, 0) * M(0, 2) - M(0, 0) * M(1, 2)),
        invdet * (M(1, 0) * M(2, 1) - M(2, 0) * M(1, 1)),
        invdet * (M(2, 0) * M(0, 1) - M(0, 0) * M(2, 1)),
        invdet * (M(0, 0) * M(1, 1) - M(1, 0) * M(0, 1)) };
}
mat4 inverse(mat4 M) {
    real invdet = 1 / determinant(M);
    real A2323 = M(2, 2) * M(3, 3) - M(2, 3) * M(3, 2) ;
    real A1323 = M(2, 1) * M(3, 3) - M(2, 3) * M(3, 1) ;
    real A1223 = M(2, 1) * M(3, 2) - M(2, 2) * M(3, 1) ;
    real A0323 = M(2, 0) * M(3, 3) - M(2, 3) * M(3, 0) ;
    real A0223 = M(2, 0) * M(3, 2) - M(2, 2) * M(3, 0) ;
    real A0123 = M(2, 0) * M(3, 1) - M(2, 1) * M(3, 0) ;
    real A2313 = M(1, 2) * M(3, 3) - M(1, 3) * M(3, 2) ;
    real A1313 = M(1, 1) * M(3, 3) - M(1, 3) * M(3, 1) ;
    real A1213 = M(1, 1) * M(3, 2) - M(1, 2) * M(3, 1) ;
    real A2312 = M(1, 2) * M(2, 3) - M(1, 3) * M(2, 2) ;
    real A1312 = M(1, 1) * M(2, 3) - M(1, 3) * M(2, 1) ;
    real A1212 = M(1, 1) * M(2, 2) - M(1, 2) * M(2, 1) ;
    real A0313 = M(1, 0) * M(3, 3) - M(1, 3) * M(3, 0) ;
    real A0213 = M(1, 0) * M(3, 2) - M(1, 2) * M(3, 0) ;
    real A0312 = M(1, 0) * M(2, 3) - M(1, 3) * M(2, 0) ;
    real A0212 = M(1, 0) * M(2, 2) - M(1, 2) * M(2, 0) ;
    real A0113 = M(1, 0) * M(3, 1) - M(1, 1) * M(3, 0) ;
    real A0112 = M(1, 0) * M(2, 1) - M(1, 1) * M(2, 0) ;
    return { invdet * ( M(1, 1) * A2323 - M(1, 2) * A1323 + M(1, 3) * A1223 ),
        invdet * - ( M(0, 1) * A2323 - M(0, 2) * A1323 + M(0, 3) * A1223 ),
        invdet *   ( M(0, 1) * A2313 - M(0, 2) * A1313 + M(0, 3) * A1213 ),
        invdet * - ( M(0, 1) * A2312 - M(0, 2) * A1312 + M(0, 3) * A1212 ),
        invdet * - ( M(1, 0) * A2323 - M(1, 2) * A0323 + M(1, 3) * A0223 ),
        invdet *   ( M(0, 0) * A2323 - M(0, 2) * A0323 + M(0, 3) * A0223 ),
        invdet * - ( M(0, 0) * A2313 - M(0, 2) * A0313 + M(0, 3) * A0213 ),
        invdet *   ( M(0, 0) * A2312 - M(0, 2) * A0312 + M(0, 3) * A0212 ),
        invdet *   ( M(1, 0) * A1323 - M(1, 1) * A0323 + M(1, 3) * A0123 ),
        invdet * - ( M(0, 0) * A1323 - M(0, 1) * A0323 + M(0, 3) * A0123 ),
        invdet *   ( M(0, 0) * A1313 - M(0, 1) * A0313 + M(0, 3) * A0113 ),
        invdet * - ( M(0, 0) * A1312 - M(0, 1) * A0312 + M(0, 3) * A0112 ),
        invdet * - ( M(1, 0) * A1223 - M(1, 1) * A0223 + M(1, 2) * A0123 ),
        invdet *   ( M(0, 0) * A1223 - M(0, 1) * A0223 + M(0, 2) * A0123 ),
        invdet * - ( M(0, 0) * A1213 - M(0, 1) * A0213 + M(0, 2) * A0113 ),
        invdet *   ( M(0, 0) * A1212 - M(0, 1) * A0212 + M(0, 2) * A0112 ) };
}

// using 4x4 transforms ////////////////////////////////////////////////////////

tuDv transformPoint(const mat4 &M, vecD p) {
    vec4 p_hom = {};
    memcpy(&p_hom, &p, D * sizeof(real));
    p_hom.w = 1;
    vec4 ret_hom = M * p_hom;
    ret_hom /= ret_hom.w;
    vecD result = {};
    memcpy(&result, &ret_hom, D * sizeof(real));
    return result;
}
tuDv transformVector(const mat4 &M, vecD v) {
    vec3 v_3D = {};
    memcpy(&v_3D, &v, D * sizeof(real));
    vec3 ret_hom = M3(M(0, 0), M(0, 1), M(0, 2), M(1, 0), M(1, 1), M(1, 2), M(2, 0), M(2, 1), M(2, 2)) * v_3D;
    vecD result = {};
    memcpy(&result, &ret_hom, D * sizeof(real));
    return result;
}
tuDv transformNormal(const mat4 &M, vecD n) {
    vec3 ret_hom = inverse(transpose(M3(M(0, 0), M(0, 1), M(0, 2), M(1, 0), M(1, 1), M(1, 2), M(2, 0), M(2, 1), M(2, 2)))) * n;
    vecD result = {};
    memcpy(&result, &ret_hom, D * sizeof(real));
    return result;
}

// 4x4 transform cookbook //////////////////////////////////////////////////////

tuDm identityMatrix() {
    matD result = {};
    for_(i, D) {
        result(i, i) = 1;
    }
    return result;
}
const mat4 _Identity4x4 = identityMatrix<4>();

mat4 M4_Identity() {
    return _Identity4x4; // FORNOW
}

mat4 M4_Translation(real x, real y, real z = 0) {
    mat4 result = _Identity4x4;
    result(0, 3) = x;
    result(1, 3) = y;
    result(2, 3) = z;
    return result;
}
mat4 M4_Translation(vec2 xy) {
    return M4_Translation(xy.x, xy.y);
}
mat4 M4_Translation(vec3 xyz) {
    return M4_Translation(xyz.x, xyz.y, xyz.z);
}
mat4 M4_Scaling(real x, real y, real z = 1) {
    mat4 result = {};
    result(0, 0) = x;
    result(1, 1) = y;
    result(2, 2) = z;
    result(3, 3) = 1;
    return result;
}
mat4 M4_Scaling(real s) {
    return M4_Scaling(s, s, s);
}
mat4 M4_Scaling(vec2 xy) {
    return M4_Scaling(xy.x, xy.y);
}
mat4 M4_Scaling(vec3 xyz) {
    return M4_Scaling(xyz.x, xyz.y, xyz.z);
}
mat4 M4_RotationAboutXAxis(real t) {
    mat4 result = _Identity4x4;
    result(1, 1) = COS(t); result(1, 2) = -SIN(t);
    result(2, 1) = SIN(t); result(2, 2) =  COS(t);
    return result;
}
mat4 M4_RotationAboutYAxis(real t) {
    mat4 result = _Identity4x4;
    result(0, 0) =  COS(t); result(0, 2) = SIN(t);
    result(2, 0) = -SIN(t); result(2, 2) = COS(t);
    return result;
}
mat4 M4_RotationAboutZAxis(real t) {
    mat4 result = _Identity4x4;
    result(0, 0) = COS(t); result(0, 1) = -SIN(t);
    result(1, 0) = SIN(t); result(1, 1) =  COS(t);
    return result;
}

mat4 M4_RotationAbout(vec3 axis, real angle) {
    real x = axis.x;
    real y = axis.y;
    real z = axis.z;
    real x2 = x * x;
    real y2 = y * y;
    real z2 = z * z;
    real xy = x * y;
    real xz = x * z;
    real yz = y * z;
    real col = COS(angle);
    real s = SIN(angle);
    real d = 1-col;
    return { col+x2*d, xy*d-z*s, xz*d+y*s, 0,
        xy*d+z*s, col+y2*d, yz*d-x*s, 0,
        xz*d-y*s, yz*d+x*s, col+z2*d, 0,
        0, 0, 0, 1
    };
}

mat4 M4_RotationFrom(vec3 a, vec3 b) {
    // NOTE: twist dof is whatever
    // https://math.stackexchange.com/questions/180418/calculate-rotation-matrix-to-align-vector-a-to-vector-b-in-3d

    // FORNOW
    a = normalized(a);
    b = normalized(b);

    vec3 v = cross(a, b);
    real col = dot(a, b);
    if (ABS(col + 1.0f) < 1e-5f) return M4_Identity();
    mat3 v_x = { 0.0, -v.z, v.y, v.z, 0.0, -v.x, -v.y, v.x, 0.0 };
    mat3 R = identityMatrix<3>() + v_x + v_x * v_x / (1 + col);
    return {
        R.data[0], R.data[1], R.data[2], 0.0,
            R.data[3], R.data[4], R.data[5], 0.0,
            R.data[6], R.data[7], R.data[8], 0.0,
            0.0,       0.0,       0.0, 1.0 };
}

// optimization stuff //////////////////////////////////////////////////////////

tuDm firstDerivativeofUnitVector(vecD v) {
    vecD tmp = normalized(v);
    return (1 / norm(v)) * (identityMatrix<D>() - outer(tmp, tmp));
}
#define firstDerivativeOfNorm normalized
#define secondDerivativeOfNorm firstDerivativeofUnitVector

template <uint D> real squaredNorm(matD M) {
    real result = 0;
    for(uint i = 0; i < D * D; ++i) {
        result += M.data[i] * M.data[i];
    }
    return result;
}

// misc functions //////////////////////////////////////////////////////////////

template <uint D> real minComponent(vecD A) {
    real result = HUGE_VAL;
    for(uint i = 0; i < D; ++i) result = MIN(result, A.data[i]);
    return result;
}

template <uint D> real maxComponent(vecD A) {
    real result = -HUGE_VAL;
    for(uint i = 0; i < D; ++i) result = MAX(result, A.data[i]);
    return result;
}

tuDv cwiseAbs(vecD A) {
    for(uint i = 0; i < D; ++i) A.data[i] = abs(A.data[i]);
    return A;
}
tuDv cwiseMin(vecD A, vecD B) {
    vecD result = {};
    for(uint i = 0; i < D; ++i) result[i] = (A.data[i] < B.data[i]) ? A.data[i] : B.data[i];
    return result;
}
tuDv cwiseMax(vecD A, vecD B) {
    vecD result = {};
    for(uint i = 0; i < D; ++i) result[i] = (A.data[i] > B.data[i]) ? A.data[i] : B.data[i];
    return result;
}
tuDv cwiseProduct(vecD a, vecD b) {
    vecD result = {};
    for(uint i = 0; i < D; ++i) result[i] = a[i] * b[i];
    return result;
}
vec2 e_theta(real theta) {
    return { COS(theta), SIN(theta) };
}
real atan2(vec2 a) {
    return atan2f(a.y, a.x);
}
vec2 rotated(vec2 a, real theta) {
    return { COS(theta) * a.x - SIN(theta) * a.y, SIN(theta) * a.x + COS(theta) * a.y };
}
mat2 R_theta_2x2(real theta) {
    return { COS(theta), -SIN(theta), SIN(theta), COS(theta) };
}
vec2 perpendicularTo(vec2 v) {
    return { v.y, -v.x };
}
mat4 xyzo2mat4(vec3 x, vec3 y, vec3 z, vec3 o) {
    return {
        x[0], y[0], z[0], o[0],
        x[1], y[1], z[1], o[1],
        x[2], y[2], z[2], o[2],
        0, 0, 0, 1
    };
}
#define M4_xyzo xyzo2mat4
tuDv magClamped(vecD a, real col) {
    real norm_a = norm(a);
    if (ABS(norm_a) < col) { return a; }
    return a / norm_a * MAG_CLAMP(norm_a, col);
}

// utility /////////////////////////////////////////////////////////////////////

template <uint D> void pprint(vecD A) {
    printf("V%d(", D);
    for_(i, D) {
        printf("%lf", A[i]);
        if (i != D - 1) printf(", ");
    }
    printf(")\n");
}
template <uint D> void pprint(matD M) {
    for_(row, D) {
        printf("| ");
        for_(col, D) {
            printf("%lf", M(row, col));
            if (col != D - 1) printf(", ");
        }
        printf(" |\n");
    }
}

// math math ///////////////////////////////////////////////////////////////////

struct RayTriangleIntersectionResult {
    bool hit;
    real distance;
};
RayTriangleIntersectionResult ray_triangle_intersection(vec3 o, vec3 dir, vec3 a, vec3 b, vec3 c) {
    RayTriangleIntersectionResult result = {};
    vec4 w_t = inverse(M4(
                a[0], b[0], c[0], -dir[0],
                a[1], b[1], c[1], -dir[1],
                a[2], b[2], c[2], -dir[2],
                1.0f, 1.0f, 1.0f,     0.0))
        * V4(o, 1.0f);
    result.hit = ((w_t.x > 0) && (w_t.y > 0) && (w_t.z > 0) && (w_t.w > 0));
    result.distance = w_t.w;
    return result;
}


