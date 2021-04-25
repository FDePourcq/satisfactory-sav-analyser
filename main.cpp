#include <iostream>
#include <vector>
#include <cassert>
#include <stack>
#include <stdint.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string_view>
#include <array>
#include <cstring>
#include "zlib.h"
#include <memory>
#include <cstdio>
#include <array>
#include <variant>
#include <optional>
#include <sstream>
#include <string.h>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#include <map>
#include <set>
#include <utility>
//#include "rapidjson/"


#define prt(x)  std::cout << #x " = '" << x << "'" << '\n';
#define prt2(x, y) std::cout << #x " = '" << x << "'\t" << #y " = '" << y << "'" << '\n';
#define prt3(x, y, z)  std::cout << #x " = '" << x << "'\t" << #y " = '" << y << "'\t" << #z " = '" << z << "'" << '\n';
#define prt4(x, y, z, u)  std::cout << #x " = '" << x << "'\t" << #y " = '" << y << "'\t" << #z " = '" << z << "'\t" << #u " = '" << u << "'" << '\n';
#define prt5(x, y, z, u, v) std::cout << #x " = '" << x \
    << "'\t" << #y " = '" << y \
    << "'\t" << #z " = '" << z \
    << "'\t" << #u " = '" << u \
    << "'\t" << #v " = '" << v \
    << "'" << '\n';
#define prt6(x, y, z, u, v, w) std::cout << #x " = '" << x \
    << "'\t" << #y " = '" << y \
    << "'\t" << #z " = '" << z \
    << "'\t" << #u " = '" << u \
    << "'\t" << #v " = '" << v \
    << "'\t" << #w " = '" << w \
    << "'" << '\n';

void writeToFile(const std::string file, const char *s) {
    FILE *f = fopen(file.c_str(), "w");
    if (f != NULL) {
        fputs(s, f);
        fclose(f);
    }
}

std::vector<std::string> debugloc;

void printDebugLoc() {
    return;
    std::cout << "DEBUGLOC: ";
    for (const auto &i : debugloc) {
        std::cout << i << ' ';
    }
    std::cout << '\n';
}

struct MemoryMappedFile;

struct MemoryMappedFile {
    int mmap_fd;
    int pagesize;
    char *d; //points directly to first address of the memory-mapped file!
    std::string filename;
    std::size_t size;

    MemoryMappedFile(const std::string filename);

    ~MemoryMappedFile();

    bool initialise();
};

MemoryMappedFile::MemoryMappedFile(const std::string a_filename) :
        mmap_fd(-1),
        pagesize(-1),
        d(0),
        filename(a_filename),
        size(0) {
}

bool MemoryMappedFile::initialise() {
    struct stat filestatus;
    stat(filename.c_str(), &filestatus);
    size = filestatus.st_size;

    mmap_fd = open(filename.c_str(), O_RDONLY);
    if (mmap_fd < 0) {
        std::cout << "open error (file not found) " << filename << '\n';
        return false;
    }
    pagesize = getpagesize();
    d = (char *) mmap((caddr_t) 0, size, PROT_READ /*| PROT_WRITE*/, MAP_SHARED, mmap_fd, 0);
    if (!d) {
        std::cout << "mmap error" << '\n';
        return false;
    }
    return true;
}

MemoryMappedFile::~MemoryMappedFile() {
    munmap(d, pagesize);
    close(mmap_fd);
}

/*
int32_t changeEndianness32(int32_t val) {
    return (val << 24) |
           ((val << 8) & 0x00ff0000) |
           ((val >> 8) & 0x0000ff00) |
           ((val >> 24) & 0x000000ff);
}*/


template<typename T>
struct MemSize {
    static const size_t v = T::memorysize;
};
template<typename T, std::size_t C>
struct MemSize<std::array<T, C> > {
    static const size_t v = C * MemSize<T>::v;
};

template<>
struct MemSize<int32_t> {
    static const size_t v = 4;
};
template<>
struct MemSize<int64_t> {
    static const size_t v = 8;
};
template<>
struct MemSize<bool> {
    static const size_t v = 1;
};
template<>
struct MemSize<char> {
    static const size_t v = 1;
};
template<>
struct MemSize<uint8_t> {
    static const size_t v = 1;
};
template<>
struct MemSize<float> {
    static const size_t v = 4;
};



//template <> class MemSize { static const size_t v = sizeof(T); };
//template <> class MemSize { static const size_t v = sizeof(T); };

struct Builder {
    char *ptr;
    std::size_t offset;
    std::size_t maxoffset;

    template<typename T>
    T inc() {
        //std::cout << "inc: ";
        std::array<char, MemSize<T>::v> alignedmem;
        memcpy(alignedmem.data(), ptr + offset, MemSize<T>::v);

        T ret = *reinterpret_cast< T const * >(alignedmem.data());
        /*      for (unsigned char c : alignedmem) {
                  fprintf(stdout, " %02x ", c);
      //        std::cout << std::hex  << (int)c << " ";
              }
              std::cout << '\n';
              std::cout << "offset: " << offset << " interpreted as " << ret << "  should be " << typeid(T).name() << '\n';
      */
        offset += MemSize<T>::v;// sizeof(T);
        return ret;
    }

    template<typename T>
    Builder &skip() {
        offset += MemSize<T>::v;//sizeof(T);
        return *this;
    }

    Builder &assertNullByte() {
        assert(inc<uint8_t>() == 0x0);
        return *this;
    }

    Builder &assertNullInt() {
        assert(inc<int32_t>() == 0x0);
        return *this;
    }

};


//
//template<>
//bool inc<bool>(char const *ptr, std::size_t &offset) {
//
//
//    bool ret = ptr[offset];
//    std::cout << "offset: " << offset << " interpreted as " << ret << "  should be  bool " << (int) ptr[offset] << '\n';
//    ++offset;
//    return ret;
//}

//template<>
//int32_t inc<int32_t>(char const *ptr, std::size_t &offset) {
//    std::array<char, sizeof(int32_t)> alignedmem;
//    memcpy(alignedmem.data(), ptr + offset, sizeof(int32_t));
//    std::cout << offset << '\n';
//    int32_t ret = *reinterpret_cast< int32_t const * >(alignedmem.data());
//
//    offset += sizeof(int32_t);
//    return ret;
//}


template<typename T>
struct RepeatedPrimitiveType {
    int32_t n;
    T *v;

    RepeatedPrimitiveType(Builder &b) :
            n(b.inc<int32_t>()),
            v(reinterpret_cast<T *>(b.ptr + b.offset)) {
//        prt2(n, size());

        assert(size() < 1048577);
        b.offset += size() * sizeof(T); // (not the string array)
    }

    std::size_t size() const { // as in number of items, not number of bytes
        return n;//std::abs(n);
    }

    bool operator<(const RepeatedPrimitiveType<T> &o) const {
        if (n == o.n) {
            return std::lexicographical_compare(v, v + n, o.v, o.v + n);
        } else {
            return n < o.n;
        }
    }

    bool operator==(const RepeatedPrimitiveType<char> &o) const {
        return n == o.n && memcmp(v, o.v, n) == 0;
    };

};


template<typename T>
struct RepeatedType : public std::vector<T> {
    RepeatedType(Builder &b) {
        int32_t n = b.inc<int32_t>();
//        std::cout << "num objects: " << n << '\n';
//        for (int32_t   i = 0; i < n; ++i){
//
//        }
//        n = std::abs(n);
//        if (n > 1000) {
//            n = 1000;
//        }
//
//        prt(n);
////        assert(n < 1000 && n > 0);
//        std::vector<T>::reserve(n);
        for (int32_t i = 0; i < n; ++i) {
            std::vector<T>::emplace_back(b);
        }
    }

};

/*
template<>
std::size_t RepeatedPrimitiveType<char>::size() const {
    return std::abs(n);
}

template<>
RepeatedPrimitiveType<char>::RepeatedPrimitiveType(char *const ptr, std::size_t &offset) :
        n(inc<int32_t>(ptr, offset)),
        v(ptr + offset) {
    prt2(n, size());

    assert(size() < 1000);

    offset += (size() + 0) * sizeof(char); // end-of-string-char

}
*/

template<typename T>
std::ostream &operator<<(std::ostream &os, const RepeatedPrimitiveType<T> &o) noexcept {
    auto s = o.size();
    for (int32_t i = 0; i < s; ++i) {
        os << o.v[i] << '\n';
    }
    return os;
}

template<>
std::ostream &operator<<(std::ostream &os, const RepeatedPrimitiveType<char> &o) noexcept {
    auto s = o.size();
    for (int32_t i = 0; i + 1 < s; ++i) {
        os << o.v[i];
    }
    if (o.size() > 0 && o.v[o.size() - 1] != '\0') {
        os << o.v[o.size() - 1];
    }
    return os;
}

template<typename T, std::size_t C>
std::ostream &operator<<(std::ostream &os, const std::array<T, C> &o) noexcept {
    for (int32_t i = 0; i < C; ++i) {
        os << o[i] << ' ';
    }
    return os;
}

struct StructProperty;
struct PropertyType;

struct ObjectReference {
    static const std::string tname;
    RepeatedPrimitiveType<char> levelName;
    RepeatedPrimitiveType<char> pathName;

    ObjectReference(Builder &b) :
            levelName(b),
            pathName(b) {
        prt2(levelName, pathName);
    }

};

const std::string ObjectReference::tname = "ObjectProperty";


struct TextProperty { // omfg
    static const std::string tname;
    //uint8_t choice1, unknown5, unknown6, unknown7;

//    RepeatedPrimitiveType<char> levelName;
//    RepeatedPrimitiveType<char> pathName;
    std::optional<RepeatedPrimitiveType<char> > s1, value;

    struct NameData {
        RepeatedPrimitiveType<char> name;
        std::array<uint8_t, 14> mysterystuff;
        RepeatedPrimitiveType<char> data;
    };

    std::vector<NameData> namedatas;

    TextProperty(Builder &b) {
        // logic from Goz3rr_SatisfactorySaveEditor  TextProperty.cs
        debugloc.push_back("TextProperty");
        printDebugLoc();
        const uint8_t mystery_choice = b.skip<int32_t>().inc<uint8_t>();prt((int) mystery_choice);
        if (mystery_choice == 0x3) {
            b.skip<int32_t>();
            b.skip<uint8_t>();
        } else if (mystery_choice == 0x255) {
            debugloc.pop_back();
            return;
        }
        b.assertNullInt();
        s1.emplace(b);
        value.emplace(b);prt2(*s1, *value);

        if (mystery_choice == 0x3) {
            int32_t count = b.inc<int32_t>();
            for (uint32_t i = 0; i < count; ++i) {
                namedatas.emplace_back(NameData{RepeatedPrimitiveType<char>(b), b.inc<std::array<uint8_t, 14> >(), RepeatedPrimitiveType<char>(b)});
                const auto &nd = *namedatas.rbegin();prt2(nd.name, nd.data);
            }
        }
        debugloc.pop_back();

    }

};

const std::string TextProperty::tname = "TextProperty";


struct EnumProperty {
    static const std::string tname;
    RepeatedPrimitiveType<char> enumName;
    RepeatedPrimitiveType<char> enumValue;

    EnumProperty(Builder &b) :
            enumName(b),
            enumValue(b.assertNullByte()) {

    }
};

const std::string EnumProperty::tname = "EnumProperty";

/*
struct SaveObject {
    RepeatedPrimitiveType<char> TypePath, RootObject, InstanceName;

    SaveObject(Builder &b) : TypePath(b),
                             RootObject(b),
                             InstanceName(b) {

    }

};*/

struct SaveEntity /*: public SaveObject*/ {


    int32_t NeedTransform;// = reader.ReadInt32() == 1;
    std::array<float, 4> Rotation;// = reader.ReadVector4();
    std::array<float, 3> Position;// = reader.ReadVector3();
    std::array<float, 3> Scale; //= reader.ReadVector3();
    int32_t WasPlacedInLevel; //= reader.ReadInt32() == 1;

    std::optional<RepeatedPrimitiveType<char> > ParentObjectRoot;
    std::optional<RepeatedPrimitiveType<char> > ParentObjectName;
    std::optional<RepeatedType<ObjectReference> > components;

    SaveEntity(Builder &b) : //SaveObject(b),
            NeedTransform(b.inc<int32_t>()),
            Rotation(b.inc<std::array<float, 4>>()),
            Position(b.inc<std::array<float, 3>>()),
            Scale(b.inc<std::array<float, 3> >()),
            WasPlacedInLevel(b.inc<int32_t>()) {
    }

    void parseAdditionalData(Builder &b) {

//        std::size_t start_offset = b.offset;

        ParentObjectRoot.emplace(b);
        ParentObjectName.emplace(b);prt2(*ParentObjectRoot, *ParentObjectName);
        components.emplace(b);prt(components->size());
        for (const auto &i : *components) { prt2(i.pathName, i.levelName);
        }
//        prt();
//        b.offset += len;
    }
};


struct Properties : std::vector<std::unique_ptr<PropertyType> > {
    Properties(Builder &b);
};

struct SaveComponent {//: public SaveObject {
    RepeatedPrimitiveType<char> ParentEntityName;

    SaveComponent(Builder &b) : // SaveObject(b),
            ParentEntityName(b) {
        prt(ParentEntityName);
    }

    void parseAdditionalData(Builder &b) {
//        assert(false);
        //b.offset += len;
    }
};

struct SerializedFields {
    SerializedFields(Builder &b, std::size_t len) {
        // ???
    }
};


struct StructBox {
    static const std::string tname;

    float minX, minY, minZ, maxX, maxY, maxZ;
    char isValid;

    StructBox(Builder &b) : minX(b.inc<float>()),
                            minY(b.inc<float>()),
                            minZ(b.inc<float>()),
                            maxX(b.inc<float>()),
                            maxY(b.inc<float>()),
                            maxZ(b.inc<float>()),
                            isValid(b.inc<char>()) {
        debugloc.push_back("StructBox");
        printDebugLoc();
        debugloc.pop_back();
//        assert(false);
    }
};

auto eq = [](const std::string &a, const RepeatedPrimitiveType<char> &b) -> bool {
    return a.size() + 1 == b.size() && memcmp(a.c_str(), b.v, a.size()) == 0;
};

struct StructColor {
    static const std::string tname;

    uint8_t r, g, b, a;

    StructColor(Builder &b_) : r(b_.inc<uint8_t>()),
                               g(b_.inc<uint8_t>()),
                               b(b_.inc<uint8_t>()),
                               a(b_.inc<uint8_t>()) {
//        assert(false);
    }
};

struct StructCompletedResearch {
    static const std::string tname;

    StructCompletedResearch(Builder &b) {
        assert(false);
    }
};

struct StructFluidBox {
    static const std::string tname;
    float v;

    StructFluidBox(Builder &b) : v(b.inc<float>()) {
//        assert(false);
    }
};

struct StructGuid {
    static const std::string tname;

    StructGuid(Builder &b) {
        assert(false);
    }
};

struct StructHotbar {
    static const std::string tname;

    StructHotbar(Builder &b) {
        assert(false);
    }
};

struct StructInventoryItem {
    static const std::string tname;
    int32_t unknown;
    RepeatedPrimitiveType<char> itemtype, unknown2, unknown3;


    StructInventoryItem(Builder &b) : unknown(b.inc<int32_t>()),
                                      itemtype(b),
                                      unknown2(b),
                                      unknown3(b) {
        prt4(unknown, itemtype, unknown2, unknown3);
    }
};

struct StructInventoryStack : public Properties {
    static const std::string tname;

    StructInventoryStack(Builder &b) : Properties(b) {

    }
};

struct StructItemAmount {
    static const std::string tname;

    StructItemAmount(Builder &b) {
        assert(false);
    }
};

struct StructItemFoundData {
    static const std::string tname;

    StructItemFoundData(Builder &b) {
        assert(false);
    }
};

struct StructLinearColor {
    static const std::string tname;


    float b, g, r, a;

    StructLinearColor(Builder &b_) : b(b_.inc<float>()),
                                     g(b_.inc<float>()),
                                     r(b_.inc<float>()),
                                     a(b_.inc<float>()) {
//        assert(false);
    }


};

struct StructMessageData {
    static const std::string tname;

    StructMessageData(Builder &b) {
        assert(false);
    }
};

struct StructPhaseCost {
    static const std::string tname;

    StructPhaseCost(Builder &b) {
        assert(false);
    }
};

struct StructProjectileData {
    static const std::string tname;

    StructProjectileData(Builder &b) {
        assert(false);
    }
};

struct StructQuat : std::array<float, 4> {
    static const std::string tname;

//    float a, b, c, d;

    StructQuat(Builder &b_) :
            std::array<float, 4>({b_.inc<float>(),
                                  b_.inc<float>(),
                                  b_.inc<float>(),
                                  b_.inc<float>()}) {
//        assert(false);
    }
};

struct StructRailroadTrackPosition {
    static const std::string tname;

    RepeatedPrimitiveType<char> levelName;
    RepeatedPrimitiveType<char> pathName;
    float offset;
    float forward;

    StructRailroadTrackPosition(Builder &b) : levelName(b),
                                              pathName(b),
                                              offset(b.inc<float>()),
                                              forward(b.inc<float>()) {
        assert(false);
    }
};

struct StructRecipeAmountStruct {
    static const std::string tname;

    StructRecipeAmountStruct(Builder &b) {
        assert(false);
    }
};

struct StructRemovedInstanceArray : public Properties {
    static const std::string tname;

    StructRemovedInstanceArray(Builder &b) : Properties(b) {

    }
};

struct StructResearchCost {
    static const std::string tname;

    StructResearchCost(Builder &b) {
        assert(false);
    }
};

struct StructResearchData {
    static const std::string tname;

    StructResearchData(Builder &b) {
        assert(false);
    }
};

struct StructResearchRecipeReward {
    static const std::string tname;

    StructResearchRecipeReward(Builder &b) {
        assert(false);
    }
};

struct StructRotator {
    static const std::string tname;

    float x, y, z;

    StructRotator(Builder &b) : x(b.inc<float>()),
                                y(b.inc<float>()),
                                z(b.inc<float>()) {
//        assert(false);
    }
};

struct StructSlateBrush {
    static const std::string tname;

    StructSlateBrush(Builder &b) {
        assert(false);
    }
};

struct StructSplinePointData {
    static const std::string tname;

    StructSplinePointData(Builder &b) {}
};

struct StructTimerHandle : RepeatedPrimitiveType<char> {
    static const std::string tname;

    StructTimerHandle(Builder &b) : RepeatedPrimitiveType<char>(b) {
//        assert(false);
    }
};

struct StructTrainSimulationData {
    static const std::string tname;

    StructTrainSimulationData(Builder &b) {
        assert(false);
    }
};

struct StructTransform : public Properties {
    static const std::string tname;

    StructTransform(Builder &b) : Properties(b) {
//        assert(false);
    }
};

struct StructVector : std::array<float, 3> {
    static const std::string tname;
//    float x, y, z;

    StructVector(Builder &b) : std::array<float, 3>({b.inc<float>(),
                                                     b.inc<float>(),
                                                     b.inc<float>()}) {
//        assert(false);
    }

//    StructVector(Builder &b) : x(b.inc<float>()),
//                               y(b.inc<float>()),
//                               z(b.inc<float>()) {
////        assert(false);
//    }
};

struct StructProperty {
    static const std::string tname;
    RepeatedPrimitiveType<char> structtype;

    std::array<char, 17> unknown;
//    int32_t unk1, unk2, unk3, unk4;
//    char unk5;
    std::variant<std::monostate, StructBox, StructColor, StructCompletedResearch, StructFluidBox, StructGuid, StructHotbar, StructInventoryItem, StructInventoryStack, StructItemAmount, StructItemFoundData, StructLinearColor, StructMessageData, StructPhaseCost, StructProjectileData, StructQuat, StructRailroadTrackPosition, StructRecipeAmountStruct, StructRemovedInstanceArray, StructResearchCost, StructResearchData, StructResearchRecipeReward, StructRotator, StructSlateBrush, StructSplinePointData, StructTimerHandle, StructTrainSimulationData, StructTransform, StructVector> structcontent;


    StructProperty(Builder &b);
};

const std::string StructProperty::tname = "StructProperty";


struct MapProperty {
    static const std::string tname;
    RepeatedPrimitiveType<char> keyType;
    RepeatedPrimitiveType<char> valueType;
    int32_t count;

    struct KeyValue {
        std::variant<std::monostate, int32_t> key;
        std::variant<std::monostate, std::vector<PropertyType> > value;
//        std::vector<std::unique_ptr<PropertyType> > props;

        KeyValue(Builder &b, const RepeatedPrimitiveType<char> &keytype, const RepeatedPrimitiveType<char> &valuetype);

    };

    std::vector<MapProperty::KeyValue> content;

    MapProperty(Builder &b);
};

const std::string MapProperty::tname = "MapProperty";

struct InterfaceProperty : public ObjectReference {
    static const std::string tname;

    InterfaceProperty(Builder &b) : ObjectReference(b) {}
};

const std::string InterfaceProperty::tname = "InterfaceProperty";

struct NameProperty : RepeatedPrimitiveType<char> {

    static const std::string tname;

    NameProperty(Builder &b) : RepeatedPrimitiveType<char>(b.assertNullByte()) {
        prt(*this);
    }
};

const std::string NameProperty::tname = "NameProperty";


struct ArrayProperty {

    static const std::string tname;
    RepeatedPrimitiveType<char> itemType;


    struct StructBS {
        int32_t count;
        RepeatedPrimitiveType<char> structName;
        RepeatedPrimitiveType<char> structType;
        int32_t structSize;
        RepeatedPrimitiveType<char> structInnerType;
        std::array<char, 16> guid;
        std::variant<std::monostate, std::vector<StructLinearColor>, std::vector<std::unique_ptr<PropertyType> > > properties;

        StructBS(Builder &b);
    };

    std::variant<std::monostate,
            RepeatedType<ObjectReference>,
            StructBS,
            RepeatedPrimitiveType<int32_t>,
            RepeatedPrimitiveType<uint8_t>,
            RepeatedType<InterfaceProperty>
    > content;


    ArrayProperty(Builder &b);
};


struct ByteProperty {
    static const std::string tname;
    RepeatedPrimitiveType<char> name;
    std::variant<std::monostate, uint8_t, RepeatedPrimitiveType<char>> content;

    ByteProperty(Builder &b) : name(b) {
        b.assertNullByte();
        if (eq("None", name)) {
            content.emplace<uint8_t>(b.inc<uint8_t>());
        } else {
            content.emplace<RepeatedPrimitiveType<char> >(b);
        }
    }
};

const std::string ByteProperty::tname = "ByteProperty";


const std::string ArrayProperty::tname = "ArrayProperty";

const std::string StructBox::tname = "Box";
const std::string StructColor::tname = "Color";
const std::string StructCompletedResearch::tname = "CompletedResearch";
const std::string StructFluidBox::tname = "FluidBox";
const std::string StructGuid::tname = "Guid";
const std::string StructHotbar::tname = "Hotbar";
const std::string StructInventoryItem::tname = "InventoryItem";
const std::string StructInventoryStack::tname = "InventoryStack";
const std::string StructItemAmount::tname = "ItemAmount";
const std::string StructItemFoundData::tname = "ItemFoundData";
const std::string StructLinearColor::tname = "LinearColor";
const std::string StructMessageData::tname = "MessageData";
const std::string StructPhaseCost::tname = "PhaseCost";
const std::string StructProjectileData::tname = "ProjectileData";
const std::string StructQuat::tname = "Quat";
const std::string StructRailroadTrackPosition::tname = "RailroadTrackPosition";
const std::string StructRecipeAmountStruct::tname = "RecipeAmountStruct";
const std::string StructRemovedInstanceArray::tname = "RemovedInstanceArray";
const std::string StructResearchCost::tname = "ResearchCost";
const std::string StructResearchData::tname = "ResearchData";
const std::string StructResearchRecipeReward::tname = "ResearchRecipeReward";
const std::string StructRotator::tname = "Rotator";
const std::string StructSlateBrush::tname = "SlateBrush";
const std::string StructSplinePointData::tname = "SplinePointData";
const std::string StructTimerHandle::tname = "TimerHandle";
const std::string StructTrainSimulationData::tname = "TrainSimulationData";
const std::string StructTransform::tname = "Transform";
const std::string StructVector::tname = "Vector";

struct PropertyType {
    RepeatedPrimitiveType<char> name;

    struct Header {
        RepeatedPrimitiveType<char> fieldType;
        int32_t size;
        int32_t index;

        Header(Builder &b) :
                fieldType(b),
                size(b.inc<int32_t>()),
                index(b.inc<int32_t>()) {

        }
    };

    std::optional<Header> header;
    std::variant<std::monostate,
            StructProperty,
            NameProperty,
            ArrayProperty,
            int32_t,
            int64_t,
            uint8_t,
            float,
            RepeatedPrimitiveType<char>,
            ObjectReference,
            EnumProperty,
            MapProperty,
            TextProperty,
            ByteProperty,
            bool> content;

    PropertyType(Builder &b);
};


ArrayProperty::StructBS::StructBS(Builder &b) : count(b.inc<int32_t>()),
                                                structName(b),
                                                structType(b),
                                                structSize(b.inc<int32_t>()),
                                                structInnerType(b.assertNullInt()),
                                                guid(b.inc<std::array<char, 16>>()) {
    b.assertNullByte();
    if (eq(StructLinearColor::tname, structInnerType)) {
        auto &v = properties.emplace<std::vector<StructLinearColor>>();
        for (int32_t i = 0; i < count; ++i) {
            v.emplace_back(b);
        }
    } else { // if (eq("RemovedInstance", structInnerType)) {
        auto &v = properties.emplace<std::vector<std::unique_ptr<PropertyType> >>();
        for (int32_t i = 0; i < count; ++i) {
            v.emplace_back(std::make_unique<PropertyType>(b));
        }
    } /*else {
        prt2(structName, structType, structInnerType);
        assert(false);
    }*/
}


ArrayProperty::ArrayProperty(Builder &b) :
        itemType(b) {
    b.assertNullByte();
    //prt(itemType);

    if (eq(ObjectReference::tname, itemType)) {
        content.emplace<RepeatedType<ObjectReference> >(b);
    } else if (eq(StructProperty::tname, itemType)) {

        content.emplace<StructBS>(b);
    } else if (eq("IntProperty", itemType)) {
        content.emplace<RepeatedPrimitiveType<int32_t> >(b);
    } else if (eq("ByteProperty", itemType)) {
        content.emplace<RepeatedPrimitiveType<uint8_t> >(b);
    } else if (eq(InterfaceProperty::tname, itemType)) {
        content.emplace<RepeatedType<InterfaceProperty> >(b);
    } else { prt(itemType);
        assert(false);
    }

}

MapProperty::KeyValue::KeyValue(Builder &b, const RepeatedPrimitiveType<char> &keytype, const RepeatedPrimitiveType<char> &valuetype) {
//    b.skip<char>(); // do these 3 bytes belong to
//    b.skip<char>();
//    b.skip<char>();
    if (eq("IntProperty", keytype)) {
        key.emplace<int32_t>(b/*.assertNullByte()*/.inc<int32_t>());
    } else { prt(keytype);
        assert(false);
    }
//    b.assertNullInt();
    if (eq(StructProperty::tname, valuetype)) {

        //value.emplace<StructProperty>(b);
        std::vector<PropertyType> &v = value.emplace<std::vector<PropertyType>>();
        while (true) {
            v.emplace_back(b);
            if (!v.rbegin()->header.has_value()) {
                v.pop_back();
                break;
            }
        }

    } else { prt(valuetype);
        assert(false);
    }

////        std::cout << "IntProperty" << '\n';
//    if (eq())
//if (eq(StructProperty::tname, header->fieldType)) {
//        content.emplace<StructProperty>(b);
//    } else if (eq(NameProperty::tname, header->fieldType)) {
//        content.emplace<NameProperty>(b);
//    } else if (eq(ArrayProperty::tname, header->fieldType)) {
//        content.emplace<ArrayProperty>(b);
//    } else if (eq("IntProperty", header->fieldType)) {
////        std::cout << "IntProperty" << '\n';
//        content.emplace<int32_t>(b.assertNullByte().inc<int32_t>());
//    } else if (eq("FloatProperty", header->fieldType)) {
//        content.emplace<float>(b.assertNullByte().inc<float>());
//    } else if (eq("StrProperty", header->fieldType)) {
////        std::cout << "StrProperty" << '\n';
//        content.emplace<RepeatedPrimitiveType<char> >(b.assertNullByte());
//    } else if (eq("BoolProperty", header->fieldType)) {
////        std::cout << "BoolProperty" << '\n';
//        content.emplace<bool>(b.inc<bool>());
//        b.assertNullByte();
//
//    } else if (eq(EnumProperty::tname, header->fieldType)) {
////        std::cout << "EnumProperty" << '\n';
//        content.emplace<EnumProperty>(b);
//    } else if (eq(MapProperty::tname, header->fieldType)) {
//        content.emplace<MapProperty>(b);
//    } else if (eq(ObjectReference::tname, header->fieldType)) {
//        content.emplace<ObjectReference>(b.assertNullByte());
//    } else if (eq(TextProperty::tname, header->fieldType)) {
//        content.emplace<TextProperty>(b.assertNullByte());
////    } else if (header->fieldType.n > 40 && header->fieldType < 160) {
////        // just assume
//    } else {
//        prt4(name, header->fieldType, header->size, header->index);
//        assert(false);
//        b.offset = expected_end;
////            header.reset();
//    }
//    while (true) {
//        props.emplace_back(std::make_unique<PropertyType>(b));
//        if (!(*props.rbegin())->header.has_value()) {
//            break;
//        }
//    }
}

MapProperty::MapProperty(Builder &b) :
        keyType(b),
        valueType(b),
        count(b.assertNullByte().assertNullInt().inc<int32_t>()) {//,
//        values(b.assertNullByte().assertNullInt()) {
//   ;
    for (int32_t i = 0; i < count; ++i) {
//    while (true) {

        content.emplace_back(b, keyType, valueType);
        assert (!std::holds_alternative<std::monostate>(content.rbegin()->key) && !std::holds_alternative<std::monostate>(content.rbegin()->value));
//            break;
        //       }
    }

//content


}


StructProperty::StructProperty(Builder &b) :
        structtype(b),
        unknown(b.inc<std::array<char, 17> >()) {
    prt(structtype);
    debugloc.push_back("StructProperty_" + std::string(structtype.v));
    printDebugLoc();
    //const std::string structtype_StructProperty = "Vector";

//    auto eq = [](const std::string &a, const RepeatedPrimitiveType<char> &b) -> bool {
//        return a.size() + 1 == b.size() && memcmp(a.c_str(), b.v, a.size()) == 0;
//    };

    if (eq(StructBox::tname, structtype)) {
        structcontent.emplace<StructBox>(b);
    } else if (eq(StructColor::tname, structtype)) {
        structcontent.emplace<StructColor>(b);
    } else if (eq(StructCompletedResearch::tname, structtype)) {
        structcontent.emplace<StructCompletedResearch>(b);
    } else if (eq(StructFluidBox::tname, structtype)) {
        structcontent.emplace<StructFluidBox>(b);
    } else if (eq(StructGuid::tname, structtype)) {
        structcontent.emplace<StructGuid>(b);
    } else if (eq(StructHotbar::tname, structtype)) {
        structcontent.emplace<StructHotbar>(b);
    } else if (eq(StructInventoryItem::tname, structtype)) {
        structcontent.emplace<StructInventoryItem>(b);
    } else if (eq(StructInventoryStack::tname, structtype)) {
        structcontent.emplace<StructInventoryStack>(b);
    } else if (eq(StructItemAmount::tname, structtype)) {
        structcontent.emplace<StructItemAmount>(b);
    } else if (eq(StructItemFoundData::tname, structtype)) {
        structcontent.emplace<StructItemFoundData>(b);
    } else if (eq(StructLinearColor::tname, structtype)) {
        structcontent.emplace<StructLinearColor>(b);
    } else if (eq(StructMessageData::tname, structtype)) {
        structcontent.emplace<StructMessageData>(b);
    } else if (eq(StructPhaseCost::tname, structtype)) {
        structcontent.emplace<StructPhaseCost>(b);
    } else if (eq(StructProjectileData::tname, structtype)) {
        structcontent.emplace<StructProjectileData>(b);
    } else if (eq(StructQuat::tname, structtype)) {
        structcontent.emplace<StructQuat>(b);
    } else if (eq(StructRailroadTrackPosition::tname, structtype)) {
        structcontent.emplace<StructRailroadTrackPosition>(b);
    } else if (eq(StructRecipeAmountStruct::tname, structtype)) {
        structcontent.emplace<StructRecipeAmountStruct>(b);
    } else if (eq(StructRemovedInstanceArray::tname, structtype)) {
        structcontent.emplace<StructRemovedInstanceArray>(b);
    } else if (eq(StructResearchCost::tname, structtype)) {
        structcontent.emplace<StructResearchCost>(b);
    } else if (eq(StructResearchData::tname, structtype)) {
        structcontent.emplace<StructResearchData>(b);
    } else if (eq(StructResearchRecipeReward::tname, structtype)) {
        structcontent.emplace<StructResearchRecipeReward>(b);
    } else if (eq(StructRotator::tname, structtype)) {
        structcontent.emplace<StructRotator>(b);
    } else if (eq(StructSlateBrush::tname, structtype)) {
        structcontent.emplace<StructSlateBrush>(b);
    } else if (eq(StructSplinePointData::tname, structtype)) {
        structcontent.emplace<StructSplinePointData>(b);
    } else if (eq(StructTimerHandle::tname, structtype)) {
        structcontent.emplace<StructTimerHandle>(b);
    } else if (eq(StructTrainSimulationData::tname, structtype)) {
        structcontent.emplace<StructTrainSimulationData>(b);
    } else if (eq(StructTransform::tname, structtype)) {
        structcontent.emplace<StructTransform>(b);
    } else if (eq(StructVector::tname, structtype)) {
        structcontent.emplace<StructVector>(b);
    } else { prt(structtype);
        std::cout.flush();
        assert(false); // unknown structtype
    }
    debugloc.pop_back();
}


PropertyType::PropertyType(Builder &b) :
        name(b) {
    const std::string typename_None = "None";
    debugloc.push_back("PropertyType_" + std::string(name.v));
    printDebugLoc();
// mRemovedInstances
//
//    auto eq = [](const std::string &a, const RepeatedPrimitiveType<char> &b) -> bool {
//        return a.size() + 1 == b.size() && memcmp(a.c_str(), b.v, a.size()) == 0;
//    };
    prt(name);
    if (eq(typename_None, name)) {
        printDebugLoc();
        debugloc.pop_back();
        return;
    }

    header.emplace(b);prt(header->fieldType);
    std::size_t offset_before_reading_value = b.offset;

    if (eq(StructProperty::tname, header->fieldType)) {
        content.emplace<StructProperty>(b);
        //assert(b.offset == offset_before_reading_value + header->size() +  );
    } else if (eq(NameProperty::tname, header->fieldType)) {
        content.emplace<NameProperty>(b);
    } else if (eq(ArrayProperty::tname, header->fieldType)) {
        content.emplace<ArrayProperty>(b);
    } else if (eq("IntProperty", header->fieldType)) {
//        std::cout << "IntProperty" << '\n';
        content.emplace<int32_t>(b.assertNullByte().inc<int32_t>());
    } else if (eq("FloatProperty", header->fieldType)) {
        content.emplace<float>(b.assertNullByte().inc<float>());
    } else if (eq("StrProperty", header->fieldType)) {
//        std::cout << "StrProperty" << '\n';
        content.emplace<RepeatedPrimitiveType<char> >(b.assertNullByte());
    } else if (eq("BoolProperty", header->fieldType)) {
//        std::cout << "BoolProperty" << '\n';
        content.emplace<bool>(b.inc<bool>());
        b.assertNullByte();

    } else if (eq(EnumProperty::tname, header->fieldType)) {
//        std::cout << "EnumProperty" << '\n';
        content.emplace<EnumProperty>(b);
    } else if (eq(MapProperty::tname, header->fieldType)) {
        content.emplace<MapProperty>(b);
    } else if (eq(ObjectReference::tname, header->fieldType)) {
        content.emplace<ObjectReference>(b.assertNullByte());
    } else if (eq(TextProperty::tname, header->fieldType)) {
        content.emplace<TextProperty>(b.assertNullByte());
    } else if (eq("Int64Property", header->fieldType)) {
        content.emplace<int64_t>(b.assertNullByte().inc<int64_t>());
    } else if (eq("ByteProperty", header->fieldType)) {
        content.emplace<ByteProperty>(b);

//    } else if (header->fieldType.n > 40 && header->fieldType < 160) {
//        // just assume
    } else { prt4(name, header->fieldType, header->size, header->index);
        std::cout.flush();
        assert(false);
        //  b.offset = expected_end;
//            header.reset();
    }


    debugloc.pop_back();

}


Properties::Properties(Builder &b) {

    while (true) {
        this->push_back(std::make_unique<PropertyType>(b));
        if (!(*this->rbegin())->header.has_value()) {
            //std::cout << "NO HEADER -> BREAK (Properties)\n";
            pop_back();
            break;
        }
    }
}


struct WorldObject { // depending on the type it can be multiple things ...
    //static const size_t memorysize = 8;
    int32_t worldobjecttype;
    RepeatedPrimitiveType<char> typePath;
    RepeatedPrimitiveType<char> rootObject;
    RepeatedPrimitiveType<char> instanceName;
    int32_t skippedBytes;

//    union {
//        SaveEntity entity;
//        SaveComponent component;
//    } payload;
    std::variant<std::monostate, SaveEntity, SaveComponent> v;

//    std::optional<SerializedFields> sf;
    std::vector<PropertyType> serializedFields;

    WorldObject(Builder &b) :
            worldobjecttype(b.inc<int32_t>()),
            typePath(b),
            rootObject(b),
            instanceName(b) {
        switch (worldobjecttype) {
            case 1:
                v.emplace<SaveEntity>(b);
                break;
            case 0:
                v.emplace<SaveComponent>(b);
                break;
            default:prt(worldobjecttype);
                std::cout.flush();
                assert(false);
        }
    }

    void parseAdditionalData(Builder &b) {
        int32_t len = b.inc<int32_t>();
        std::size_t start_offset = b.offset;prt((int) worldobjecttype);
        switch (worldobjecttype) {
            case 1:
                std::get<SaveEntity>(v).parseAdditionalData(b);
                break;
            case 0:
                std::get<SaveComponent>(v).parseAdditionalData(b);
                break;
            default:
                assert(false);
        }
        int32_t remainingbytes = (start_offset + len - b.offset);
        /*std::cout << "missing " << remainingbytes << " bytes\n";
        for (std::size_t l = b.offset; l < start_offset + len; l += 16) {
            std::cout << l << ' ' << ' ';
            for (std::size_t o = l; o < start_offset + len && o < l + 16; ++o) {
                if (o < start_offset + len) {
                    fprintf(stdout, "%02x ", (unsigned char) b.ptr[o]);
                } else {
                    fprintf(stdout, "   ");
                }
            }
            for (std::size_t o = l; o < start_offset + len && o < l + 16; ++o) {
                if (o < start_offset + len) {
                    fprintf(stdout, "%c", b.ptr[o]);
                } else {
                    fprintf(stdout, "   ");
                }
            }
            std::cout << '\n';
        }
        std::cout << '\n';
*/
//        for (std::size_t o = b.offset; o < start_offset + len; ++o) {
//            fprintf(stderr, "%c", b.ptr[o]);
//        }
//        std::cout << '\n';
        while (true) {
            remainingbytes = (start_offset + len - b.offset);
            //prt(remainingbytes);
            assert(remainingbytes > 0);
            auto &keyvalue = serializedFields.emplace_back(b);
            /*std::cout << "***************************\n";
            printDebugLoc();*/
            if (!serializedFields.rbegin()->header.has_value()) {
//                std::cout << "NO HEADER -> BREAK (parseAdditionalData)\n";
                serializedFields.pop_back();
                break;
            }
        }
//
        skippedBytes = start_offset + len - b.offset;
        if (skippedBytes != 4) {
            std::cout << "setting offset to " << (start_offset + len) << " while it currently is " << b.offset << " skippedbytes: " << skippedBytes << '\n';
        }
//        assert(start_offset + len - b.offset == 4);
//assert(b.offset == start_offset + len);
        b.offset = start_offset + len;
//        SerializedFields.emplace(b);

    }
};

struct CompressedBlock {
    int64_t packageFileTag;
    int64_t maximumChunkSize;
    int64_t currentChunkCompressedLength;
    int64_t currentChunkUncompressedLength;
    int64_t currentChunkCompressedLength2;
    int64_t currentChunkUncompressedLength2;
    char *compressedData;

    CompressedBlock(Builder &b) :
            packageFileTag(b.inc<int64_t>()),
            maximumChunkSize(b.inc<int64_t>()),
            currentChunkCompressedLength(b.inc<int64_t>()),
            currentChunkUncompressedLength(b.inc<int64_t>()),
            currentChunkCompressedLength2(b.inc<int64_t>()),
            currentChunkUncompressedLength2(b.inc<int64_t>()) {
        prt(packageFileTag);
        compressedData = b.ptr + b.offset;
        b.offset += currentChunkCompressedLength;
        assert(packageFileTag == 0x9E2A83C1);  //CompressedBlock::CompressedBlock
        assert(currentChunkCompressedLength == currentChunkCompressedLength2);
        assert(currentChunkUncompressedLength == currentChunkUncompressedLength2);prt4(packageFileTag, maximumChunkSize, currentChunkCompressedLength, currentChunkUncompressedLength2);
    }
};


// This implementation of lower_bound doesnt require a reference to the value.
// But it breaks the convention (?) that a comparator accepts 2 params.
template<typename _ForwardIterator, typename _Compare>
_ForwardIterator lower_bound2(_ForwardIterator __first,
                              const _ForwardIterator &__last,
                              const _Compare &__comp) {
    auto __len = std::distance(__first, __last);
    //prt(__len);
    while (__len > 0) {

        auto __half = __len >> 1;
        _ForwardIterator __middle = __first;
        std::advance(__middle, __half);
        if (__comp(*__middle)) {
            __first = __middle;
            ++__first;
            __len = __len - __half - 1;
        } else
            __len = __half;
//        prt(__len);
    }
    return __first;
}

struct SatisfactorySavPayload {
    int32_t bufferSize;//, numObjects, maybe3, maybe4, maybe5, maybe6;
    RepeatedType<WorldObject> worldObjects;
    int32_t totalSaveObjectData;
    //int32_t collectedObjectsCount;
    std::optional<RepeatedType<ObjectReference>> collectedObjects;


    // lookup structures for every instancetype
    std::vector<std::size_t> ordered_indexes;


    SatisfactorySavPayload(Builder &b) :
            bufferSize(b.inc<int32_t>()),
//            numObjects(b.inc<int32_t>()),
//            maybe3(b.inc<int32_t>()),


            worldObjects(b),
            totalSaveObjectData(b.inc<int32_t>()) {

        prt3(bufferSize, worldObjects.size(), totalSaveObjectData);
        assert(worldObjects.size() == totalSaveObjectData);

        for (auto &wo : worldObjects) {
            wo.parseAdditionalData(b);
        }
        collectedObjects.emplace(b);// = b.inc<int32_t>();

        assert(totalSaveObjectData == worldObjects.size());


        auto &worldObjects2 = worldObjects;
        for (int32_t i = 0; i < totalSaveObjectData; ++i) {
            auto &wo = worldObjects[i];
            //auto &v = typepath_to_ordered_indexes[wo.typePath];
            ordered_indexes.insert(lower_bound2(ordered_indexes.begin(), ordered_indexes.end(), [&wo, &worldObjects2](const std::size_t o) {
                return worldObjects2[o].instanceName < wo.instanceName;
            }), i);
        }

    }

    std::size_t findWorldObject(const RepeatedPrimitiveType<char> &path) const {
        auto &worldObjects2 = worldObjects;
        auto i = lower_bound2(ordered_indexes.begin(), ordered_indexes.end(), [&path, &worldObjects2](const std::size_t o) {
            return worldObjects2[o].instanceName < path;
        });
        if (i != ordered_indexes.end() && worldObjects2[*i].instanceName == path) {
            return *i;
        } else {
            return std::numeric_limits<std::size_t>::max();
        }
    }

    void discoverComponentRelations() const {
        std::ostringstream oss;


        struct Edge : public std::array<std::string, 3> {

            bool operator<(const Edge &e) const {
                return std::lexicographical_compare(this->begin(), this->end(), e.begin(), e.end());
            }

        };

        std::map<Edge, int> edges;

        for (int32_t i = 0; i < totalSaveObjectData; ++i) {
            auto &wo = worldObjects[i];
            if (wo.worldobjecttype == 1) {
                const auto &se = std::get<SaveEntity>(wo.v);
                if (se.components.has_value()) {
                    for (const auto &j : *se.components) {
                        std::size_t i = findWorldObject(j.pathName);
                        if (i == std::numeric_limits<std::size_t>::max()) {
                            continue;
                        }

                        std::string label(j.pathName.v);
                        for (auto i = label.find('.'); i != std::string::npos; i = label.find('.')) {
                            label.erase(0, i + 1);
                        }
                        auto dottify = [](const std::string &s) {
                            std::string ret;
                            for (auto &i : s) {
                                if (i >= '0' && i <= '9') {
                                    ret += i;
                                } else if (i >= 'a' && i <= 'z') {
                                    ret += i;
                                } else if (i >= 'A' && i <= 'Z') {
                                    ret += i;
                                } else if (!ret.empty()) {
                                    ret += "\\l";
                                }
                            }
                            ret += "\\l";
                            return ret;
                        };

                        edges[{dottify(wo.typePath.v),
                               label,
                               dottify(worldObjects[i].typePath.v)}]++;
                    }
                }
            }
        }
        oss << "digraph 123 { GRAPH [rankdir=LR,overlap=false]; node [shape=box]; \n";
        for (const auto &edge_count : edges) {
            const Edge &edge = edge_count.first;
            oss << '"' << edge[0] << "\" -> \"" << edge[2] << "\" [label=\"" << edge[1] << "\\l" << edge_count.second << "\"]" << std::endl;
        }
        oss << "\n}\n";
        writeToFile("/tmp/satisfactory_sav_structure.dot", oss.str().c_str());
        //fdp -Elen="10"  -Tpng /tmp/satisfactory_sav_structure.dot  > /tmp/zucht.png && feh /tmp/zucht.png

    }

};

struct SatisfactorySav {
    int32_t headerVersion;
    int32_t saveVersion;
    int32_t buildVersion;
    RepeatedPrimitiveType<char> worldType;
    RepeatedPrimitiveType<char> worldProperties;
    RepeatedPrimitiveType<char> sessionName;
    int32_t playTimeSeconds;
    int64_t saveDateMsTicks;
    bool sessionVisibility;
//    RepeatedType<CompressedBlock> chunks;
//    CompressedBlock chunk;
    std::vector<CompressedBlock> chunks;
    std::vector<char> uncompressedData;

    std::optional<SatisfactorySavPayload> payload;

    SatisfactorySav(Builder &b) :
            headerVersion(b.inc<int32_t>()),
            saveVersion(b.inc<int32_t>()),
            buildVersion(b.inc<int32_t>()),
            worldType(b),
            worldProperties(b),
            sessionName(b),
            playTimeSeconds(b.inc<int32_t>()),
            saveDateMsTicks(b.inc<int64_t>()),
            sessionVisibility(b.inc<bool>())
//            , worldObjects(ptr, offset)
    {
        std::size_t totalsize_uncompressed = 0;
        while (b.offset < b.maxoffset) {
            totalsize_uncompressed += chunks.emplace_back(b).currentChunkUncompressedLength;
        }prt2(chunks.size(), totalsize_uncompressed);
        uncompressedData.resize(totalsize_uncompressed);
        std::size_t deflateOffset = 0;
        for (const auto &chunk : chunks) {
            z_stream strm;
            strm.zalloc = 0;
            strm.zfree = 0;
            strm.next_in = reinterpret_cast<uint8_t *>(chunk.compressedData);
            strm.avail_in = chunk.currentChunkCompressedLength;
            strm.next_out = reinterpret_cast<uint8_t *>(uncompressedData.data()) + deflateOffset;
            strm.avail_out = chunk.currentChunkUncompressedLength;
            inflateInit(&strm);
            inflate(&strm, Z_NO_FLUSH);
            inflateEnd(&strm);
            deflateOffset += chunk.currentChunkUncompressedLength2;
        }
        //std::cout << "============================================== b2 ======================================\n";
        Builder b2{uncompressedData.data(), 0, uncompressedData.size()};

        payload.emplace(b2);

    }


//
//    int &headerVersion() const {
//        return ptr[0];
//    }
//
//    int &version() const {
//        return ptr[1];
//    }
//
//    int &buildVersion() const {
//        return ptr[2];
//    }
//
//    std::string_view worldType() const {
//        return std::string_view(reinterpret_cast<char *>(&ptr[4]), ptr[3]);
//    }
//
//    std::string_view worldProperties() const {
//        return std::string_view(reinterpret_cast<char *>(&ptr[4]), ptr[5]);
//    }

};

#define JSONFIELD(x)  w.String(#x); printJson(w, t.x);

/*template<typename W, typename T>
void printJson(W &w, const T &t) noexcept {
    assert(false);
}*/


template<typename W>
void printJson(W &w, const int32_t &t) noexcept {
    w.Int(t);
};

template<typename W>
void printJson(W &w, const float &t) noexcept {
    w.Double(t);
};

template<typename W>
void printJson(W &w, const int64_t &t) noexcept {
    w.Int64(t);
};

template<typename W>
void printJson(W &w, const bool &t) noexcept {
    w.Bool(t);
};


template<typename W, typename X>
void printJson(W &w, const std::vector<X> &t) noexcept {
    w.StartArray();
    for (const auto &i : t) {
        printJson(w, i);
    }
    w.EndArray();
};

template<typename W, typename X>
void printJson(W &w, const std::unique_ptr<X> &t) noexcept {
    if (t) {
        printJson(w, *t);
    } else {
        w.Null();
    }


};

//std::array<float, 4>
//, typename std::enable_if<2 < C, bool>::type = true
template<typename W, typename X, size_t C>
void printJson(W &w, const std::array<X, C> &t) noexcept {
    w.StartArray();
    std::cout.flush();
    for (size_t i = 0; i < C; ++i) {
        printJson(w, t[i]);
    }
    w.EndArray();
};

template<typename W, typename T>
void printJson(W &w, const RepeatedPrimitiveType<T> &t) noexcept {
    w.StartArray();
    std::cout.flush();
    for (size_t i = 0; i < t.n; ++i) {
        printJson(w, t.v[i]);
    }
    w.EndArray();
};

template<typename W>
void printJson(W &w, const RepeatedPrimitiveType<char> &t) noexcept {
    if (t.n > 0) {
        assert(t.v[t.n - 1] == '\0');
        w.String(t.v, t.n - 1);
    } else {
        w.Null();
    }
};


template<typename W, typename X>
void printJson(W &w, const std::optional<X> &t) noexcept {
    if (t.has_value()) {
        printJson(w, *t);
    } else {
        w.Null();
    }
};

template<typename W, typename X>
void printJson(W &w, const RepeatedType<X> &t) noexcept {
    w.StartArray();
    for (auto &i: t) {
        printJson(w, i);
    }
    w.EndArray();
};

template<typename W>
void printJson(W &w, const PropertyType::Header &t) noexcept {
    w.StartObject();
    JSONFIELD(fieldType);
    JSONFIELD(size);
    JSONFIELD(index);
    w.EndObject();
};


template<typename W>
void printJson(W &w, const SatisfactorySav &t) noexcept {
    w.StartObject();
    JSONFIELD(headerVersion);
    JSONFIELD(saveVersion);
    JSONFIELD(buildVersion);
    JSONFIELD(worldType);
    JSONFIELD(worldProperties);
    JSONFIELD(sessionName);
    JSONFIELD(saveDateMsTicks);
    JSONFIELD(sessionVisibility);
    JSONFIELD(payload);
    w.EndObject();
}

template<typename W>
void printJson(W &w, const SatisfactorySavPayload &t) noexcept {
    w.StartObject();
    JSONFIELD(bufferSize);
    JSONFIELD(worldObjects);
    JSONFIELD(totalSaveObjectData);
    JSONFIELD(collectedObjects);
    w.EndObject();
}


template<typename W>
void printJson(W &w, const WorldObject &t) noexcept {
    w.StartObject();
    JSONFIELD(worldobjecttype);
    JSONFIELD(typePath);
    JSONFIELD(rootObject);
    JSONFIELD(instanceName);

//    w.String("content");
    switch (t.worldobjecttype) {
        case 1:
            printJson(w, std::get<SaveEntity>(t.v));
            break;
        case 0:
            printJson(w, std::get<SaveComponent>(t.v));
            break;
        default:
            assert(false);
    }

    JSONFIELD(serializedFields);

    if (t.skippedBytes != 4) {
        JSONFIELD(skippedBytes);
    }

    w.EndObject();
}

template<typename W>
void printJson(W &w, const SaveEntity &t) noexcept {
//    w.StartObject();
    JSONFIELD(NeedTransform);
    JSONFIELD(Rotation);
    JSONFIELD(Position);
    JSONFIELD(Scale);
    JSONFIELD(WasPlacedInLevel);
    JSONFIELD(ParentObjectRoot);
    JSONFIELD(ParentObjectName);
    JSONFIELD(components);
//    w.EndObject();
}

template<typename W>
void printJson(W &w, const SaveComponent &t) noexcept {
//    w.StartObject();
    JSONFIELD(ParentEntityName);
//    w.EndObject();
}

template<typename W>
void printJson(W &w, const ObjectReference &t) noexcept {
    w.StartObject();
    JSONFIELD(levelName);
    JSONFIELD(pathName);
    w.EndObject();
}

template<typename W>
void printJson(W &w, const PropertyType &t) noexcept {
    w.StartObject();
    JSONFIELD(name);
    JSONFIELD(header);

    if (t.header.has_value()) {
        w.String("value");
        const RepeatedPrimitiveType<char> &ft = t.header->fieldType;

        if (eq(StructProperty::tname, ft)) {
            printJson(w, std::get<StructProperty>(t.content));
        } else if (eq(NameProperty::tname, ft)) {
            printJson(w, std::get<NameProperty>(t.content));
        } else if (eq(ArrayProperty::tname, ft)) {
            printJson(w, std::get<ArrayProperty>(t.content));
        } else if (eq("IntProperty", ft)) {
            printJson(w, std::get<int32_t>(t.content));
        } else if (eq("FloatProperty", ft)) {
            printJson(w, std::get<float>(t.content));
        } else if (eq("StrProperty", ft)) {
            printJson(w, std::get<RepeatedPrimitiveType<char>>(t.content));
        } else if (eq("BoolProperty", ft)) {
            printJson(w, std::get<bool>(t.content));
        } else if (eq(EnumProperty::tname, ft)) {
            printJson(w, std::get<EnumProperty>(t.content));
        } else if (eq(MapProperty::tname, ft)) {
            printJson(w, std::get<MapProperty>(t.content));
        } else if (eq(ObjectReference::tname, ft)) {
            printJson(w, std::get<ObjectReference>(t.content));
        } else if (eq(TextProperty::tname, ft)) {
            printJson(w, std::get<TextProperty>(t.content));
        } else if (eq("Int64Property", ft)) {
            printJson(w, std::get<int64_t>(t.content));
        } else if (eq("ByteProperty", ft)) {
            printJson(w, std::get<ByteProperty>(t.content));
        } else {
            assert(false);
        }
    }

    w.EndObject();
}

template<typename W>
void printJson(W &w, const StructProperty &t) noexcept {
    w.StartObject();
    JSONFIELD(structtype);
    w.String("content");
    if (eq(StructBox::tname, t.structtype)) {
        printJson(w, std::get<StructBox>(t.structcontent));
    } else if (eq(StructColor::tname, t.structtype)) {
        printJson(w, std::get<StructColor>(t.structcontent));
    } else if (eq(StructCompletedResearch::tname, t.structtype)) {
        printJson(w, std::get<StructCompletedResearch>(t.structcontent));
    } else if (eq(StructFluidBox::tname, t.structtype)) {
        printJson(w, std::get<StructFluidBox>(t.structcontent));
    } else if (eq(StructGuid::tname, t.structtype)) {
        printJson(w, std::get<StructGuid>(t.structcontent));
    } else if (eq(StructHotbar::tname, t.structtype)) {
        printJson(w, std::get<StructHotbar>(t.structcontent));
    } else if (eq(StructInventoryItem::tname, t.structtype)) {
        printJson(w, std::get<StructInventoryItem>(t.structcontent));
    } else if (eq(StructInventoryStack::tname, t.structtype)) {
        printJson(w, std::get<StructInventoryStack>(t.structcontent));
    } else if (eq(StructItemAmount::tname, t.structtype)) {
        printJson(w, std::get<StructItemAmount>(t.structcontent));
    } else if (eq(StructItemFoundData::tname, t.structtype)) {
        printJson(w, std::get<StructItemFoundData>(t.structcontent));
    } else if (eq(StructLinearColor::tname, t.structtype)) {
        printJson(w, std::get<StructLinearColor>(t.structcontent));
    } else if (eq(StructMessageData::tname, t.structtype)) {
        printJson(w, std::get<StructMessageData>(t.structcontent));
    } else if (eq(StructPhaseCost::tname, t.structtype)) {
        printJson(w, std::get<StructPhaseCost>(t.structcontent));
    } else if (eq(StructProjectileData::tname, t.structtype)) {
        printJson(w, std::get<StructProjectileData>(t.structcontent));
    } else if (eq(StructQuat::tname, t.structtype)) {
        printJson(w, std::get<StructQuat>(t.structcontent));
    } else if (eq(StructRailroadTrackPosition::tname, t.structtype)) {
        printJson(w, std::get<StructRailroadTrackPosition>(t.structcontent));
    } else if (eq(StructRecipeAmountStruct::tname, t.structtype)) {
        printJson(w, std::get<StructRecipeAmountStruct>(t.structcontent));
    } else if (eq(StructRemovedInstanceArray::tname, t.structtype)) {
        printJson(w, std::get<StructRemovedInstanceArray>(t.structcontent));
    } else if (eq(StructResearchCost::tname, t.structtype)) {
        printJson(w, std::get<StructResearchCost>(t.structcontent));
    } else if (eq(StructResearchData::tname, t.structtype)) {
        printJson(w, std::get<StructResearchData>(t.structcontent));
    } else if (eq(StructResearchRecipeReward::tname, t.structtype)) {
        printJson(w, std::get<StructResearchRecipeReward>(t.structcontent));
    } else if (eq(StructRotator::tname, t.structtype)) {
        printJson(w, std::get<StructRotator>(t.structcontent));
    } else if (eq(StructSlateBrush::tname, t.structtype)) {
        printJson(w, std::get<StructSlateBrush>(t.structcontent));
    } else if (eq(StructSplinePointData::tname, t.structtype)) {
        printJson(w, std::get<StructSplinePointData>(t.structcontent));
    } else if (eq(StructTimerHandle::tname, t.structtype)) {
        printJson(w, std::get<StructTimerHandle>(t.structcontent));
    } else if (eq(StructTrainSimulationData::tname, t.structtype)) {
        printJson(w, std::get<StructTrainSimulationData>(t.structcontent));
    } else if (eq(StructTransform::tname, t.structtype)) {
        printJson(w, std::get<StructTransform>(t.structcontent));
    } else if (eq(StructVector::tname, t.structtype)) {
        printJson(w, std::get<StructVector>(t.structcontent));
    } else {
        assert(false);
    }


    w.EndObject();
}

template<typename W>
void printJson(W &w, const EnumProperty &t) noexcept {
    w.StartObject();
    JSONFIELD(enumName);
    JSONFIELD(enumValue);
    w.EndObject();
}

template<typename W>
void printJson(W &w, const ByteProperty &t) noexcept {
    w.StartObject();
    JSONFIELD(name);
    w.String("content");
    if (eq("None", t.name)) {
        printJson(w, std::get<uint8_t>(t.content));
    } else {
        printJson(w, std::get<RepeatedPrimitiveType<char> >(t.content));
    }
    w.EndObject();
}

template<typename W>
void printJson(W &w, const ArrayProperty &t) noexcept {
    w.StartObject();
    JSONFIELD(itemType);

    w.String("content");
    if (eq(ObjectReference::tname, t.itemType)) {
        printJson(w, std::get<RepeatedType<ObjectReference>>(t.content));
    } else if (eq(StructProperty::tname, t.itemType)) {
        printJson(w, std::get<ArrayProperty::StructBS>(t.content));
    } else if (eq("IntProperty", t.itemType)) {
        printJson(w, std::get<RepeatedPrimitiveType<int32_t>>(t.content));
    } else if (eq("ByteProperty", t.itemType)) {
        printJson(w, std::get<RepeatedPrimitiveType<uint8_t> >(t.content));
    } else if (eq(InterfaceProperty::tname, t.itemType)) {
        printJson(w, std::get<RepeatedType<InterfaceProperty> >(t.content));
    } else {
        assert(false);
    }

    w.EndObject();
}


template<typename W>
void printJson(W &w, const ArrayProperty::StructBS &t) noexcept {
    w.StartObject();
    JSONFIELD(count);
    JSONFIELD(structName);
    JSONFIELD(structType);
    JSONFIELD(structSize);
    JSONFIELD(structInnerType);
    JSONFIELD(guid);
    //JSONFIELD(count);

    //TODO: properties
    w.String("properties");

    if (eq(StructLinearColor::tname, t.structInnerType)) {
        printJson(w, std::get<std::vector<StructLinearColor> >(t.properties));
    } else { // if (eq("RemovedInstance", structInnerType)) {
        printJson(w, std::get<std::vector<std::unique_ptr<PropertyType> > >(t.properties));
    }


    w.EndObject();
}

template<typename W>
void printJson(W &w, const MapProperty &t) noexcept {
    w.StartObject();
    JSONFIELD(keyType);
    JSONFIELD(valueType);
    JSONFIELD(count);
    //TODO: content
    w.EndObject();
}

template<typename W>
void printJson(W &w, const TextProperty &t) noexcept {
    w.StartObject();
    JSONFIELD(s1);
    JSONFIELD(value);
    JSONFIELD(namedatas);
    w.EndObject();
}

template<typename W>
void printJson(W &w, const TextProperty::NameData &t) noexcept {
    w.StartObject();
    JSONFIELD(name);
    JSONFIELD(mysterystuff);
    JSONFIELD(data);
    w.EndObject();
}

template<typename W>
void printJson(W &w, const StructLinearColor &t) noexcept {
    w.StartArray();
    w.Double(t.r);
    w.Double(t.g);
    w.Double(t.b);
    w.Double(t.a);
    w.EndArray();
}

template<typename W>
void printJson(W &w, const StructBox &t) noexcept {
    w.StartObject();
    JSONFIELD(minX);
    JSONFIELD(minY);
    JSONFIELD(minZ);
    JSONFIELD(maxX);
    JSONFIELD(maxY);
    JSONFIELD(maxZ);
    JSONFIELD(isValid);
    w.EndObject();
}

template<typename W>
void printJson(W &w, const StructColor &t) noexcept {
    w.StartArray();
    w.Double(t.r);
    w.Double(t.g);
    w.Double(t.b);
    w.Double(t.a);
    w.EndArray();
}

template<typename W>
void printJson(W &w, const StructCompletedResearch &t) noexcept {
    assert(false);
}


template<typename W>
void printJson(W &w, const StructFluidBox &t) noexcept {
    w.StartObject();
    JSONFIELD(v);
    w.EndObject();
}


template<typename W>
void printJson(W &w, const StructGuid &t) noexcept {
    assert(false);
}

template<typename W>
void printJson(W &w, const StructHotbar &t) noexcept {
    assert(false);
}

template<typename W>
void printJson(W &w, const StructInventoryItem &t) noexcept {
    w.StartObject();
    JSONFIELD(unknown);
    JSONFIELD(itemtype);
    JSONFIELD(unknown2);
    JSONFIELD(unknown3);
    w.EndObject();
}

//template<typename W>
//void printJson(W &w, const StructInventoryStack &t) noexcept {
//    assert(false);
//}

template<typename W>
void printJson(W &w, const StructItemAmount &t) noexcept {
    assert(false);
}

template<typename W>
void printJson(W &w, const StructItemFoundData &t) noexcept {
    assert(false);
}

template<typename W>
void printJson(W &w, const StructMessageData &t) noexcept {
    assert(false);
}

template<typename W>
void printJson(W &w, const StructPhaseCost &t) noexcept {
    assert(false);
}

template<typename W>
void printJson(W &w, const StructProjectileData &t) noexcept {
    assert(false);
}

//template<typename W>
//void printJson(W &w, const StructQuat &t) noexcept {
//    w.StartArray();
//    w.Double(t.a);
//    w.Double(t.b);
//    w.Double(t.c);
//    w.Double(t.d);
//    w.EndArray();
//}


template<typename W>
void printJson(W &w, const StructRailroadTrackPosition &t) noexcept {
    w.StartObject();
    JSONFIELD(levelName);
    JSONFIELD(pathName);
    JSONFIELD(offset);
    JSONFIELD(forward);
    w.EndObject();
}

template<typename W>
void printJson(W &w, const StructRecipeAmountStruct &t) noexcept {
    assert(false);
}

template<typename W>
void printJson(W &w, const StructResearchCost &t) noexcept {
    assert(false);
}

template<typename W>
void printJson(W &w, const StructResearchData &t) noexcept {
    assert(false);
}

template<typename W>
void printJson(W &w, const StructResearchRecipeReward &t) noexcept {
    assert(false);
}

template<typename W>
void printJson(W &w, const StructRotator &t) noexcept {
    w.StartArray();
    w.Double(t.x);
    w.Double(t.y);
    w.Double(t.z);
    w.EndArray();
}

template<typename W>
void printJson(W &w, const StructSlateBrush &t) noexcept {
    assert(false);
}

template<typename W>
void printJson(W &w, const StructSplinePointData &t) noexcept {
    assert(false);
}

template<typename W>
void printJson(W &w, const StructTrainSimulationData &t) noexcept {
    assert(false);
}

#undef JSONFIELD


int main() {

  //    MemoryMappedFile mmf("/home/donf/fsrc/satisfactory/satisfactorygenerator/mmm2_2004-181613.sav");
        MemoryMappedFile mmf("/home/donf/fsrc/satisfactory/satisfactorygenerator/mmm2_autosave_0.sav");
    mmf.initialise();prt(mmf.size);
    assert(mmf.d);
//    std::size_t current_offset = 0;// reinterpret_cast<int32_t *>(mmf.d);
    Builder b{mmf.d, 0, mmf.size};
    SatisfactorySav sav(b);
    prt3(sav.headerVersion, sav.saveVersion, sav.buildVersion);
    prt(sav.worldType);
    prt(sav.worldProperties);
    prt(sav.sessionName);
    prt(sav.playTimeSeconds);
    prt(sav.saveDateMsTicks);
    prt(sav.sessionVisibility);

    prt4(sav.payload->bufferSize, sav.payload->totalSaveObjectData, sav.payload->collectedObjects->size(), sav.payload->worldObjects.size());

    std::cerr << "parsing done.";
    std::cerr << "building json.";

    rapidjson::StringBuffer s;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> w(s);
    printJson(w, sav);
    std::cerr << "writing json.";
    writeToFile("/tmp/satisfactory_save.json", s.GetString());

    std::cerr << "building component-graph.";
    sav.payload->discoverComponentRelations();
    std::cerr << "component-graph written." << std::endl;


    // consider ... a boolean set operations-tool ...
    // given 2 files create ... intersection, union, difference
    // also specify how to recognize identical elements: same name, same location, same rotation, same type ...
    //  also specify on what to do with the names of the new file: for the intersection: the names of which side should be kept? should one side be completely renamed?   is one dominant?

    // or ... we add a new objecttype ... one that gives a named reference to the inputs/outputs/stuff from the "blueprint" factory
    // so ... when we do "templates" ... then we must be able to rename everything ...


    std::cerr << "neato -Noverlap=false -Eoverlap=false -Goverlap=false -LT103 /tmp/satisfactory_sav_structure.dot  -Tpng > /tmp/zucht.png  && feh /tmp/zucht.png" << std::endl;
    return 0;
}
