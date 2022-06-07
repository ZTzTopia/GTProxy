#define _CRT_SECURE_NO_WARNINGS
#include <sstream>
#include <map>
#include <fmt/format.h>

#include "Variant.h"

Variant::~Variant() {
    if (m_pSig_onChanged) {
        delete m_pSig_onChanged;
        m_pSig_onChanged = nullptr;
    }
}

std::function<void (Variant *)> *Variant::GetSigOnChanged() {
    if (!m_pSig_onChanged) {
        m_pSig_onChanged = new std::function<void (Variant*)>;
    }

    return m_pSig_onChanged;
}

void Variant::Set(const Variant &v) {
    // Update our data from another variant object
    switch (v.GetType()) {
        case TYPE_FLOAT:
            Set(v.GetFloat());
            break;
        case TYPE_STRING:
            Set(v.GetString());
            break;
        case TYPE_VECTOR2:
            Set(v.GetVector2());
            break;
        case TYPE_VECTOR3:
            Set(v.GetVector3());
            break;
        case TYPE_UINT32:
            Set(v.GetUINT32());
            break;
        case TYPE_INT32:
            Set(v.GetINT32());
            break;
        /*case TYPE_ENTITY:
            Set(v.GetEntity());
            break;
        case TYPE_COMPONENT:
            Set(v.GetComponent());
            break;*/
        case TYPE_RECT:
            Set(v.GetRect());
            break;
        default:
            assert(!"Add support for this var type?");
    }

    if (m_pSig_onChanged) {
        (*m_pSig_onChanged)(this);
    }
}

void Variant::SetVariant(Variant *pVar) { // Needed this because boost was confused...
    Set(*pVar);
}

// Helper to turn anything into a string, like ints/floats
template<class C>
std::string toString(C value) {
    std::ostringstream o;
    o << value;
    return o.str();
}

std::string PrintVector2(CL_Vec2f v) {
    char st[128];
    sprintf(st, "%.2f, %.2f", v.x, v.y);
    return std::string{ st };
}

std::string PrintVector3(CL_Vec3f v) {
    char st[128];
    sprintf(st, "%.3f, %.3f, %.3f", v.x, v.y, v.z);
    return std::string{ st };
}

std::string PrintRect(CL_Rectf v) {
    char st[128];
    sprintf(st, "%.3f, %.3f, %.3f, %.3f", v.x, v.width, v.y, v.height);
    return std::string{ st };
}

std::string Variant::Print() {
    switch(GetType()) {
        case TYPE_FLOAT:
            return toString(GetFloat());
        case TYPE_STRING:
            return GetString();
        case TYPE_VECTOR2:
            return PrintVector2(GetVector2());
        case TYPE_VECTOR3:
            return PrintVector3(GetVector3());
        case TYPE_UINT32:
            return toString(GetUINT32());
        case TYPE_INT32:
            return toString(GetINT32());
        /*case TYPE_ENTITY:
            // return GetEntity()->GetName();
            return "An entity"; // I don't want to include the entity.h here right now
            break;
        case TYPE_COMPONENT:
            // return GetEntity()->GetName();
            return "A component"; // I don't want to include the entity.h here right now
            break;*/
        case TYPE_RECT:
            return PrintRect(GetRect());
        case TYPE_UNUSED:
            return "Unknown";
        default:
            assert(!"Add support for this var type?");
            return "";
    }
}

Variant &Variant::operator=(const Variant &rhs) {
    if (this != &rhs) {
        m_type = rhs.m_type;
        m_pVoid = rhs.m_pVoid;
        memcpy(m_var, rhs.m_var, C_VAR_SPACE_BYTES);
        m_string = rhs.m_string;

        if (m_pSig_onChanged) {
            // If anyone was connected to us, let them know the valid change.  Up to them to figure out that
            // The type might have as well!
            if (m_pSig_onChanged) {
                (*m_pSig_onChanged)(this);
            }
        }
    }

    return *this;
}

inline Variant operator+(Variant lhs, const Variant &rhs) {
    lhs += rhs;
    return lhs;
}

Variant &Variant::operator+=(const Variant &rhs) {
    if (GetType() == rhs.GetType()) {
        switch (GetType()) {
            case TYPE_FLOAT:
                Set(GetFloat() + rhs.GetFloat());
                break;
            case TYPE_STRING:
                Set(GetString() + rhs.GetString());
                break;
            case TYPE_VECTOR2:
                Set(GetVector2() + rhs.GetVector2());
                break;
            case TYPE_VECTOR3:
                Set(GetVector3() + rhs.GetVector3());
                break;
            case TYPE_UINT32:
                Set(GetUINT32() + rhs.GetUINT32());
                break;
            case TYPE_INT32:
                Set(GetINT32() + rhs.GetINT32());
                break;
            default:
                break;
        }
    }

    return *this;
}

inline Variant operator-(Variant lhs, const Variant &rhs) {
    lhs -= rhs;
    return lhs;
}

Variant &Variant::operator-=(const Variant &rhs) {
    if (GetType() == rhs.GetType()) {
        switch (GetType()) {
            case TYPE_FLOAT:
                Set(GetFloat() - rhs.GetFloat());
                break;
            case TYPE_VECTOR2:
                Set(GetVector2() - rhs.GetVector2());
                break;
            case TYPE_VECTOR3:
                Set(GetVector3() - rhs.GetVector3());
                break;
            case TYPE_UINT32:
                Set(GetUINT32() - rhs.GetUINT32());
                break;
            case TYPE_INT32:
                Set(GetINT32() - rhs.GetINT32());
                break;
            default:
                break;
        }
    }

    return *this;
}

bool Variant::operator==(const Variant &rhs) const {
    if (GetType() != rhs.GetType()) {
        return false;
    }

    switch (GetType()) {
        case TYPE_UNUSED:
            return true;
        case TYPE_FLOAT:
            return GetFloat() == rhs.GetFloat();
        case TYPE_STRING:
            return GetString() == rhs.GetString();
        case TYPE_VECTOR2:
            return GetVector2() == rhs.GetVector2();
        case TYPE_VECTOR3:
            return GetVector3() == rhs.GetVector3();
        case TYPE_UINT32:
            return GetUINT32() == rhs.GetUINT32();
        /*case TYPE_ENTITY:
            return GetEntity() == rhs.GetEntity();
        case TYPE_COMPONENT:
            return GetComponent() == rhs.GetComponent();*/
        case TYPE_RECT:
            return GetRect() == rhs.GetRect();
        case TYPE_INT32:
            return GetINT32() == rhs.GetINT32();
        default:
            assert(!"No equals operator for this type yet implemented");
            return false;
    }
}

bool Variant::operator!=(const Variant &rhs) const {
    return !operator==(rhs);
}

#define GET_RED(p)              (((p) & 0x0000FF00) >>  8)
#define GET_GREEN(p)            (((p) & 0x00FF0000) >> 16)
#define GET_BLUE(p)             (((p) & 0x000000FF) << 16)
#define GET_ALPHA(p)            ((p) & 0x000000FF)
#define MAKE_RGB(r, g, b)       (((uint32_t)(r) << 8) + ((uint32_t)(g) << 16) + ((uint32_t)(b) << 24))
#define MAKE_RGBA(r, g, b, a)   (((uint32_t)(r) << 8) + ((uint32_t)(g) << 16) + ((uint32_t)(b) << 24) + ((uint32_t)(a) << 0))

// Mix between c1 and c2 based on progress, which should be 0 to 1
uint32_t ColorCombineMix(uint32_t c1, uint32_t c2, float progress) {
    // OPTIMIZE oh come on, optimize this - Seth
    // What optimize? - ZTz

    int r = int(float(GET_RED(c1)) + ((float(GET_RED(c2)) - float(GET_RED(c1))) * progress));
    int g = int(float(GET_GREEN(c1)) + ((float(GET_GREEN(c2)) - float(GET_GREEN(c1))) * progress));
    int b = int(float(GET_BLUE(c1)) + ((float(GET_BLUE(c2)) - float(GET_BLUE(c1))) * progress));
    int a = int(float(GET_ALPHA(c1)) + ((float(GET_ALPHA(c2)) - float(GET_ALPHA(c1))) * progress));

    // LogMsg(PrintColor(MAKE_RGBA(r, g, b, a)).c_str());
    return MAKE_RGBA(r, g, b, a);
}

#define SMOOTHSTEP(x) ((x) * (x) * (3 - 2 * (x))) // Thanks to sol_hsa at
#define EASE_TO(x) (1 - (1 - (x)) * (1 - (x)))
#define EASE_FROM(x) ((x)*(x))

// curPos is between 0 (value of A) and 1 (value of B)
void Variant::Interpolate(Variant *pA, Variant *pB, float curPos, eInterpolateType type) {
	assert(GetType() == pA->GetType() && GetType() == pB->GetType() && "these should all be of the same type");
	bool bAsColor = false;
	switch (type) {
        case INTERPOLATE_LINEAR_AS_COLOR:
            bAsColor = true;
            break; // As is
        case INTERPOLATE_SMOOTHSTEP:
            curPos = SMOOTHSTEP(curPos);
            break;
        case INTERPOLATE_SMOOTHSTEP_AS_COLOR:
            curPos = SMOOTHSTEP(curPos);
            bAsColor = true;
            break;
        case INTERPOLATE_EASE_TO:
            curPos = EASE_TO(curPos);
            break;
        case INTERPOLATE_EASE_FROM:
            curPos = EASE_FROM(curPos);
            break;
        case INTERPOLATE_EASE_TO_QUARTIC:
            curPos = EASE_TO(curPos);
            curPos = EASE_TO(curPos);
            curPos = EASE_TO(curPos);
            break;
        case INTERPOLATE_EASE_FROM_QUARTIC:
            curPos = curPos * curPos * curPos * curPos;
            break;
        case INTERPOLATE_BOUNCE_TO:
            if (curPos < 0.36363636f) {
                curPos = 7.5625f * curPos * curPos;
            }
            else if (curPos < 0.72727273f) {
                curPos -= 0.54545455f;
                curPos = 7.5625f * curPos * curPos + 0.75f;
            }
            else if (curPos < 0.90909091f) {
                curPos -= 0.81818182f;
                curPos = 7.5625f * curPos * curPos + 0.9375f;
            }
            else {
                curPos -= 0.95454545f;
                curPos = 7.5625f * curPos * curPos + 0.984375f;
            }
            break;
        case INTERPOLATE_LINEAR:
            break; // As is
        default:
            // LogError("Unknown interpolation type");
            assert(0);
    }

    switch (pA->GetType()) {
        case eVariantType::TYPE_FLOAT: {
            Set(pA->GetFloat() + ((pB->GetFloat() - pA->GetFloat()) * curPos));
        }
        break;
        case eVariantType::TYPE_VECTOR2: {
            Set(pA->GetVector2() + ((pB->GetVector2() - pA->GetVector2()) * curPos));
        }
        break;
        case eVariantType::TYPE_UINT32: {
            if (bAsColor) {
#ifdef _CONSOLE
                assert(!"Not supported in console builds");
                pA->Set(pB->GetUINT32());
#else
                Set(ColorCombineMix(pA->GetUINT32(), pB->GetUINT32(), curPos));
#endif
            }
            else {
                Set(uint32_t(float(pA->GetUINT32()) + ((float(pB->GetUINT32()) - float(pA->GetUINT32())) * curPos)));
            }
        }
        break;
        case eVariantType::TYPE_INT32: {
            Set(int32_t(float(pA->GetINT32()) + ((float(pB->GetINT32()) - float(pA->GetINT32())) * curPos)));
        }
        break;
        default:
            // LogError("Interpolate: Don't handle this combination yet");
            assert(0);
	}
}

bool Variant::Save(FILE *fp, const std::string &varName) {
    // Unused
    return false;
}

void Variant::ClearConnections() {
    if (m_pSig_onChanged) {
        delete m_pSig_onChanged;
        m_pSig_onChanged = nullptr;
    }
}

int GetSizeOfData(eVariantType type) {
    switch (type) {
    case eVariantType::TYPE_UNUSED:
    /*case eVariantType::TYPE_COMPONENT:
    case eVariantType::TYPE_ENTITY:*/
        return 0;
    case eVariantType::TYPE_UINT32:
    case eVariantType::TYPE_INT32:
    case eVariantType::TYPE_FLOAT:
        return 4;
    case eVariantType::TYPE_VECTOR2:
        return sizeof(CL_Vec2f);
    case eVariantType::TYPE_VECTOR3:
        return sizeof(CL_Vec3f);
    case eVariantType::TYPE_RECT:
        return sizeof(CL_Rectf);
    default:
        assert(!"Uh..");
    }
    return 0;
}

uint8_t *VariantList::SerializeToMem(uint32_t *pSizeOut, uint8_t *pDest) {
    int varsUsed = 0, memNeeded = 0, tempSize = 0;
    for (auto &i : m_variant) {
        if (i.GetType() == eVariantType::TYPE_STRING) {
            tempSize = (int)i.GetString().size() + 4; // The 4 is for an int showing how long the string will be when writing
        }
        else {
            tempSize = GetSizeOfData(i.GetType());
        }

        if (tempSize > 0) {
            varsUsed++;
            memNeeded += tempSize;
        }
    }

    int totalSize = memNeeded + 1 + (varsUsed * 2);
    if (!pDest) {
        pDest = new uint8_t[totalSize]; // 1 is to write how many are coming
    }

    // Write it
    uint8_t *pCur = pDest;
    pCur[0] = uint8_t(varsUsed);
    pCur++;

    uint8_t type;
    for (uint8_t i = 0; i < C_MAX_VARIANT_LIST_PARMS; i++) {
        if (m_variant[i].GetType() == eVariantType::TYPE_STRING) {
            type = i;
            memcpy(pCur, &type, 1); pCur += 1; // Index

            type = eVariantType::TYPE_STRING;
            memcpy(pCur, &type, 1); pCur += 1; // Type

            uint32_t s = (int)m_variant[i].GetString().size();
            memcpy(pCur, &s, 4); pCur += 4; // Length of string
            memcpy(pCur, m_variant[i].GetString().c_str(), s); pCur += s; //actual string data
        }
        else {
            tempSize = GetSizeOfData(m_variant[i].GetType());
            if (tempSize > 0) {
                type = i;
                memcpy(pCur, &type, 1); pCur += 1; // Index

                type = m_variant[i].GetType();
                memcpy(pCur, &type, 1); pCur += 1; // Type

                memcpy(pCur, m_variant[i].m_var, static_cast<size_t>(tempSize)); pCur += tempSize;
            }
        }
    }

    *pSizeOut = static_cast<uint32_t>(totalSize);
    return pDest;
}

bool VariantList::SerializeFromMem(uint8_t *pSrc, int bufferSize, int *pBytesReadOut) {
    uint8_t *pStartPos = pSrc;
    uint8_t count = pSrc[0];
    pSrc++;
    for (int i = 0; i < count; i++) {
        uint8_t index = pSrc[0];
        pSrc++;
        uint8_t type = pSrc[0];
        pSrc++;
        switch (type) {
            case eVariantType::TYPE_STRING: {
                uint32_t strLen;
                memcpy(&strLen, pSrc, 4);
                pSrc += 4;

                std::string v;
                v.resize(strLen);
                memcpy(&v[0], pSrc, strLen);
                pSrc += strLen;
                m_variant[index].Set(v);
                break;
            }
            case eVariantType::TYPE_UINT32: {
                uint32_t v;
                memcpy(&v, pSrc, sizeof(uint32_t));
                pSrc += sizeof(uint32_t);
                m_variant[index].Set(v);
                break;
            }
            case eVariantType::TYPE_INT32: {
                int32_t v;
                memcpy(&v, pSrc, sizeof(int32_t));
                pSrc += sizeof(int32_t);
                m_variant[index].Set(v);
                break;
            }
            case eVariantType::TYPE_VECTOR2: {
                CL_Vec2f v;
                memcpy(&v, pSrc, sizeof(CL_Vec2f));
                pSrc += sizeof(CL_Vec2f);
                m_variant[index].Set(v);
                break;
            }
            case eVariantType::TYPE_VECTOR3: {
                CL_Vec3f v;
                memcpy(&v, pSrc, sizeof(CL_Vec3f));
                pSrc += sizeof(CL_Vec3f);
                m_variant[index].Set(v);
                break;
            }
            case eVariantType::TYPE_FLOAT: {
                float v;
                memcpy(&v, pSrc, sizeof(float));
                pSrc += sizeof(float);
                m_variant[index].Set(v);
                break;
            }
            default: {
                // LogMsg("unknown var type");
                if (pBytesReadOut) {
                    *pBytesReadOut = 0;
                }

                assert(!"Unknown var type");
                return false;
            }
        }
    }

    if (pBytesReadOut) {
        *pBytesReadOut = (int) (pSrc - pStartPos);
    }

    return true; // Success
}

void VariantList::GetVariantListStartingAt(VariantList *pOut, int startIndex) {
    int cur = 0;
    for (int i = startIndex; i < C_MAX_VARIANT_LIST_PARMS; i++) {
        pOut->m_variant[cur++] = m_variant[i];
    }
}

std::vector<std::string>  VariantList::GetContentsAsArray() {
    std::vector<std::string>  ret{};
     for (auto & variant : m_variant) {
        if (variant.GetType() == eVariantType::TYPE_UNUSED) {
            continue;
        }

        ret.push_back(variant.Print());
     }

     return ret;
}
std::string VariantList::GetContentsAsDebugString() {
    const auto& data = GetContentsAsArray();
    if (data.empty()) {
        return "(None)";
    }

    std::string ret{};
    for (int i = 0; i < data.size(); i++) {
        ret.append(fmt::format("[{}] {}\r\n", i, data[i]));
    }

    return ret;
}
