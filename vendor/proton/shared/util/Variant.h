//  ***************************************************************
//  Variant - Creation date: 04/13/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#pragma once
#include <cstdint>
#include <string>
#include <cassert>
#include <cstring>
#include <functional>

#include "../common.h"

enum eVariantType {
    TYPE_UNUSED,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_VECTOR2,
    TYPE_VECTOR3,
    TYPE_UINT32,
    /*TYPE_ENTITY,
    TYPE_COMPONENT,*/
    TYPE_RECT = 8,
    TYPE_INT32
};

enum eInterpolateType {
    INTERPOLATE_LINEAR = 0,
    INTERPOLATE_SMOOTHSTEP,
    INTERPOLATE_EASE_TO,
    INTERPOLATE_EASE_FROM,
    INTERPOLATE_EASE_TO_QUARTIC,
    INTERPOLATE_EASE_FROM_QUARTIC,
    INTERPOLATE_BOUNCE_TO,
    INTERPOLATE_SMOOTHSTEP_AS_COLOR,
    INTERPOLATE_LINEAR_AS_COLOR
};

#define C_VAR_SPACE_BYTES 16 // Enough to fit a rect

// Signals are ignored when copying these
class Variant {
    friend class VariantList;
public:
    Variant() { SetDefaults(); }
    ~Variant();

    Variant(uint32_t var) { SetDefaults(); Set(var); }
    Variant(int32_t var) { SetDefaults(); Set(var); }
    Variant(float var) { SetDefaults(); Set(var); }
    Variant(float x, float y) { SetDefaults(); Set(CL_Vec2f(x, y)); }
    Variant(float x, float y, float z) { SetDefaults(); Set(CL_Vec3f(x, y, z)); }
    Variant(float x, float y, float w, float z) { SetDefaults(); Set(CL_Rectf(x, y, w, z)); }
    Variant(const char *var) { SetDefaults(); Set(var); }
    Variant(const std::string &var) { SetDefaults(); Set(var); }
    Variant(const CL_Vec2f &v2) { SetDefaults(); Set(v2); }
    Variant(const CL_Vec3f &v3) { SetDefaults(); Set(v3); }
    Variant(const CL_Rectf &r) { SetDefaults(); Set(r); }
    /*Variant(Entity *pEnt) { SetDefaults(); Set(pEnt); }
    Variant(EntityComponent *pEnt) { SetDefaults(); Set(pEnt); }*/
    Variant(const Variant &v) {
        SetDefaults();
        // Our special copy won't include the sig crap, that stuff can't be copied
        *this = v;
    }

    void Reset() {
        m_type = TYPE_UNUSED;
        if (m_pSig_onChanged) {
            delete m_pSig_onChanged;
            m_pSig_onChanged = nullptr;
        }
    }

    std::function<void (Variant*)> *GetSigOnChanged();

    void Set(const Variant &v);
    void SetVariant(Variant *pVar) /* Needed this because boost was confused... */;

    void Set(uint32_t var) {
        assert(m_type == TYPE_UNUSED || m_type == TYPE_UINT32);

        m_type = TYPE_UINT32;
        *((uint32_t*)m_var) = var;

        if (m_pSig_onChanged) {
            (*m_pSig_onChanged)(this);
        }
    }

    uint32_t &GetUINT32() {
        if (m_type == TYPE_UNUSED) {
            Set(uint32_t(0));
        }

        assert(m_type == TYPE_UINT32);
        return *((uint32_t*)m_var);
    }

    [[nodiscard]] const uint32_t &GetUINT32() const {
        assert(m_type == TYPE_UINT32);
        return *((uint32_t*)m_var);
    }

    void Set(int32_t var) {
        assert(m_type == TYPE_UNUSED || m_type == TYPE_INT32);

        m_type = TYPE_INT32;
        *((int32_t*)m_var) = var;

        if (m_pSig_onChanged) {
            (*m_pSig_onChanged)(this);
        }
    }

    int32_t &GetINT32() {
        if (m_type == TYPE_UNUSED) {
            Set(int32_t(0));
        }

        assert(m_type == TYPE_INT32);
        return *((int32_t*)m_var);
    }

    [[nodiscard]] const int32_t &GetINT32() const {
        assert(m_type == TYPE_INT32);
        return *((int32_t*)m_var);
    }

    void Set(float var) {
        assert(m_type == TYPE_UNUSED || m_type == TYPE_FLOAT);

        m_type = TYPE_FLOAT;
        *((float*)m_var) = var;

        if (m_pSig_onChanged) {
            (*m_pSig_onChanged)(this);
        }
    }

    float &GetFloat() {
        if (m_type == TYPE_UNUSED) {
            Set(float(0));
        }

        assert(m_type == TYPE_FLOAT);
        return *((float*)m_var);
    }

    [[nodiscard]] const float &GetFloat() const {
        assert(m_type == TYPE_FLOAT);
        return *((float*)m_var);
    }

    void Set(const char *var) {
        Set(std::string{ var });
    }

    void Set(std::string const &var) {
        assert(m_type == TYPE_UNUSED || m_type == TYPE_STRING);

        m_type = TYPE_STRING;
        m_string = var;

        if (m_pSig_onChanged) {
            (*m_pSig_onChanged)(this);
        }
    }

    std::string &GetString() {
        if (m_type == TYPE_UNUSED) {
            Set("");
        }

        assert(m_type == TYPE_STRING);
        return m_string;
    }

    [[nodiscard]] const std::string &GetString() const {
        assert(m_type == TYPE_STRING);
        return m_string;
    }

    void Set(CL_Vec2f const &var) {
        assert(m_type == TYPE_UNUSED || m_type == TYPE_VECTOR2);

        m_type = TYPE_VECTOR2;
        *((CL_Vec2f*)m_var) = var;

        if (m_pSig_onChanged) {
            (*m_pSig_onChanged)(this);
        }
    }

    CL_Vec2f &GetVector2() {
        if (m_type == TYPE_UNUSED) {
            // Set a default
            Set(CL_Vec2f(0,0));
        }

        assert(m_type == TYPE_VECTOR2);
        return *((CL_Vec2f*)m_var);
    }

    [[nodiscard]] const CL_Vec2f &GetVector2() const {
        assert(m_type == TYPE_VECTOR2);
        return *((CL_Vec2f*)m_var);
    }

    void Set(float x, float y) {
        Set(CL_Vec2f(x,y));
    }

    void Set(CL_Vec3f const &var) {
        assert(m_type == TYPE_UNUSED || m_type == TYPE_VECTOR3);

        m_type = TYPE_VECTOR3;
        *((CL_Vec3f*)m_var) = var;

        if (m_pSig_onChanged) {
            (*m_pSig_onChanged)(this);
        }
    }

    CL_Vec3f &GetVector3() {
        if (m_type == TYPE_UNUSED) {
            // Set a default
            Set(CL_Vec3f(0,0,0));
        }

        assert(m_type == TYPE_VECTOR3);
        return *((CL_Vec3f*)m_var);
    }

    [[nodiscard]] const CL_Vec3f &GetVector3() const {
        assert(m_type == TYPE_VECTOR3);
        return *((CL_Vec3f*)m_var);
    }

    void Set(float x, float y, float z) {
        Set(CL_Vec3f(x,y,z));
    }

    void Set(CL_Rectf const &var) {
        assert(m_type == TYPE_UNUSED || m_type == TYPE_RECT);

        m_type = TYPE_RECT;
        *((CL_Rectf*)m_var) = var;

        if (m_pSig_onChanged) {
            (*m_pSig_onChanged)(this);
        }
    }

    CL_Rectf &GetRect() {
        if (m_type == TYPE_UNUSED) {
            // Set a default
            Set(CL_Rectf(0,0,0,0));
        }

        assert(m_type == TYPE_RECT);
        return *((CL_Rectf*)m_var);
    }

    [[nodiscard]] const CL_Rectf &GetRect() const {
        assert(m_type == TYPE_RECT);
        return *((CL_Rectf*)m_var);
    }

    void Set(float x, float y, float w, float z) {
        Set(CL_Rectf(x, y, w, z));
    }

    /*void Set(const Entity *pEnt) {
        assert(m_type == TYPE_UNUSED || m_type == TYPE_ENTITY);
        m_type = TYPE_ENTITY;   m_pVoid = (void*)pEnt;
        if (m_pSig_onChanged) (*m_pSig_onChanged)(this);
    }

    Entity* GetEntity() {
        if (m_type == TYPE_UNUSED) Set((Entity*)NULL);
        assert(m_type == TYPE_ENTITY);
        return  ((Entity*)m_pVoid);
    }

    const Entity* GetEntity() const {
        assert(m_type == TYPE_ENTITY);
        return  ((Entity*)m_pVoid);
    }

    void Set(const EntityComponent *pEnt) {
        assert(m_type == TYPE_UNUSED || m_type == TYPE_COMPONENT);
        m_type = TYPE_COMPONENT;   m_pVoid = (void*)pEnt;
        if (m_pSig_onChanged) (*m_pSig_onChanged)(this);
    }

    EntityComponent * GetComponent() {
        if (m_type == TYPE_UNUSED) Set((Entity*)NULL);
        assert(m_type == TYPE_COMPONENT);
        return  ((EntityComponent*)m_pVoid);
    }

    const EntityComponent * GetComponent() const {
        assert(m_type == TYPE_COMPONENT);
        return  ((EntityComponent*)m_pVoid);
    }*/

    [[nodiscard]] eVariantType GetType() const { return m_type; }
    std::string Print();

    Variant &operator=(const Variant &rhs);

    /**
     * Add-and-assign operator.
     *
     * Adds the value of \a rhs to the value of this \c Variant and assigns the new value to this.
     * The type of this must match the type of \a rhs.
     *
     * The \c Variant type's specific addition operator is used. For example the
     * \c CL_Vec2f::operator+() is used when adding two vectors together.
     *
     * This operator has effect only for \c Variant types where adding two values makes sense
     * and is supported, such as the float and integer types. Adding for example two pointers
     * together makes no sense so this operator has no effect for the \c TYPE_ENTITY nor
     * \c TYPE_COMPONENT types among others.
     *
     * \note this operator is also supported for TYPE_STRING: it performs string
     * catenation as expected.
     * \return a reference to \c this.
     */
    Variant& operator+=(const Variant &rhs);

    /**
     * Subtract-and-assign operator.
     *
     * Subtracts the value of \a rhs from the value of this \c Variant and assigns the new value to this.
     * The type of this must match the type of \a rhs.
     *
     * The \c Variant type's specific subtraction operator is used. For example the
     * \c CL_Vec2f::operator-() is used when subtracting two vectors.
     *
     * This operator has effect only for \c Variant types where subtracting two values makes sense
     * and is supported, such as the float and integer types. Subtracting for example two pointers
     * from each other makes no sense so this operator has no effect for the \c TYPE_ENTITY nor
     * \c TYPE_COMPONENT types among others.
     *
     * \return a reference to \c this.
     */
    Variant& operator-=(const Variant &rhs);

    /**
     * Equality operator.
     *
     * \c Variants are considered equal when their types and values are equal.
     *
     * If the type is \c TYPE_UNUSED then it doesn't matter what the values
     * are, the \c Variants are considered equal.
     *
     * The values are compared for equality using the values' classes' own \c operator==().
     * For example if the type is \c TYPE_VECTOR2 then \c CL_Vec2f::operator==() is used for
     * comparing the values.
     */
    bool operator==(const Variant &rhs) const;

    /**
     * Inequality operator.
     *
     * The opposite of operator==().
     *
     * \see operator==() for a thorough explanation.
     */
    bool operator!=(const Variant &rhs) const;

    void Interpolate(Variant *pA, Variant *pB, float curPos, eInterpolateType type);

    bool Save(FILE *fp, const std::string &varName); // Assumes you've already fopen'ed something and pass the file pointer in

    void ClearConnections(); // Clear any boost connections that were connections to its onchanged signal.

private:
    void SetDefaults() {
        m_type = TYPE_UNUSED;
        m_pSig_onChanged = nullptr;
    }

private:
    eVariantType m_type;
    void *m_pVoid;
    union {
        uint8_t m_var[C_VAR_SPACE_BYTES]; // Large enough so we can use the space for all the datatypes we care about
        // Don't actually use these, these unions help me look at vars when debugging easier
        float m_as_floats[4];
        uint32_t m_as_uint32s[4];
        int32_t m_as_int32s[4];
    };
    std::string m_string;
    std::function<void(Variant *)> *m_pSig_onChanged; //not initialized unless used
};

/**
 * \relates Variant
 * Adds two \link Variant \c Variants \endlink together. Returns a new
 * \c Variant that contains the result of the operation.
 * \see Variant::operator+=()
 */
inline Variant operator+(Variant lhs, const Variant &rhs);

/**
 * \relates Variant
 * Subtracts \a rhs from \a lhs. Returns a new \c Variant that contains
 * the result of the operation.
 * \see Variant::operator-=()
 */
inline Variant operator-(Variant lhs, const Variant &rhs);

// A VariantList holds a group of variants, we pass these when we don't know in advance how many variants we want to use

#define C_MAX_VARIANT_LIST_PARMS 12

/*
    // Example of memory serialization of a VariantList
    VariantList v;
    v.Get(0).Set(uint32(42));
    v.Get(1).Set("Hey guys");
    // Save to mem
    byte *pData = v.SerializeToMem(&size, NULL);
    // Load from mem
    VariantList b;
    b.SerializeFromMem(pData, size);
    // Display
    LogMsg("%s, the answer to life is %d", b.Get(1).GetString().c_str(), b.Get(0).GetUINT32());
    // Clean up the mem we initted earlier
    SAFE_DELETE_ARRAY(pData);
*/

class VariantList {
public:
    VariantList() = default;

    explicit VariantList(const Variant &v0) { m_variant[0] = v0; }
    VariantList(const Variant &v0, const Variant &v1) { m_variant[0] = v0; m_variant[1] = v1; }
    VariantList(const Variant &v0, const Variant &v1, const Variant &v2) { m_variant[0] = v0; m_variant[1] = v1; m_variant[2] = v2; }
    VariantList(const Variant &v0, const Variant &v1, const Variant &v2, const Variant &v3) { m_variant[0] = v0; m_variant[1] = v1; m_variant[2] = v2; m_variant[3] = v3; }
    VariantList(const Variant &v0, const Variant &v1, const Variant &v2, const Variant &v3, const Variant &v4) { m_variant[0] = v0; m_variant[1] = v1; m_variant[2] = v2; m_variant[3] = v3; m_variant[4] = v4; }
    VariantList(const Variant &v0, const Variant &v1, const Variant &v2, const Variant &v3, const Variant &v4, const Variant &v5) { m_variant[0] = v0; m_variant[1] = v1; m_variant[2] = v2; m_variant[3] = v3; m_variant[4] = v4;  m_variant[5] = v5; }
    VariantList(const Variant &v0, const Variant &v1, const Variant &v2, const Variant &v3, const Variant &v4, const Variant &v5, const Variant &v6) { m_variant[0] = v0; m_variant[1] = v1; m_variant[2] = v2; m_variant[3] = v3; m_variant[4] = v4;  m_variant[5] = v5; m_variant[6] = v6; }

    void Reset() {
        for (auto &i : m_variant) {
            i.Reset();
        }
    }

    uint8_t *SerializeToMem(uint32_t *pSizeOut, uint8_t *pDest); // Pass in NULL for dest and it will new[] the memory itself
    bool SerializeFromMem(uint8_t *pSrc, int length, int *pBytesReadOut = nullptr); // Length seems to be unused..

    Variant &Get(int parmNum) { assert(parmNum < C_MAX_VARIANT_LIST_PARMS); return m_variant[parmNum]; }
    void GetVariantListStartingAt(VariantList *pOut, int startIndex); // Shift variables over to the right, deleting some.  0 based index

    std::string GetContentsAsDebugString();

public:
    Variant m_variant[C_MAX_VARIANT_LIST_PARMS]; // Non-dynamic for speed
};