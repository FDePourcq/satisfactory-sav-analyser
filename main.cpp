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

#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/encodings.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/encodedstream.h"

#include <regex>


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


#define pt(x)  "\"" << #x << "\" = " << x << ";\t"

#define assertss(expr, value)\
   if (!(expr)){\
       std::cerr << "BOEM!" << std::endl << value << std::endl; \
        __assert_fail (__STRING(expr), __FILE__, __LINE__, __ASSERT_FUNCTION); abort(); \
   }
#define assertssexec(expr, value, __e__)\
   if (!(expr)){\
       std::cerr << "BOEM!" << std::endl << value << std::endl; __e__; \
        __assert_fail (__STRING(expr), __FILE__, __LINE__, __ASSERT_FUNCTION); abort(); \
   }

void writeToFile(const std::string file, const char *s) {
    FILE *f = fopen(file.c_str(), "w");
    if (f != NULL) {
        fputs(s, f);
        fclose(f);
    }
}


#include <functional>

// this might be used instead of setcomparison too...
template<typename containertype>
void sortedContainerComparison(const containertype &seta,
                               const containertype &setb,
                               std::function<void(const typename containertype::const_iterator &)> handleSetAOnly,
                               std::function<void(const typename containertype::const_iterator &)> handleSetBOnly,
                               std::function<void(const typename containertype::const_iterator &, const typename containertype::const_iterator &)> handleCommon) {

    auto ita = seta.begin();
    auto enda = seta.end();
    auto itb = setb.begin();
    auto endb = setb.end();

    while (ita != enda && itb != endb) {
        if (*ita < *itb) {
            handleSetAOnly(ita);//, itb);
            ++ita;
        } else if (*itb < *ita) {
            handleSetBOnly(itb);//ita, itb);
            ++itb;
        } else {
            handleCommon(ita, itb);
            ++ita;
            ++itb;
        }
    }
    while (ita != enda) {
        handleSetAOnly(ita);
        ++ita;
    }
    while (itb != endb) {
        handleSetBOnly(itb);
        ++itb;
    }
}


// this might be used instead of map-comparison too...
template<typename containertype>
void sortedPairContainerComparison(containertype &seta,
                                   containertype &setb,
                                   std::function<bool(const typename containertype::const_iterator &)> handleSetAOnly,
                                   std::function<bool(const typename containertype::const_iterator &)> handleSetBOnly,
                                   std::function<bool(const typename containertype::const_iterator &, const typename containertype::const_iterator &)> handleCommon) {

    auto ita = seta.begin();
    auto enda = seta.end();
    auto itb = setb.begin();
    auto endb = setb.end();

    while (ita != enda && itb != endb) {
        if (ita->first < itb->first) {
            if (!handleSetAOnly(ita)) {
                return;
            }
            ++ita;
        } else if (itb->first < ita->first) {
            if (!handleSetBOnly(itb)) {
                return;
            }
            ++itb;
        } else {
            if (!handleCommon(ita, itb)) {
                return;
            }
            ++ita;
            ++itb;
        }
    }
    while (ita != enda) {
        if (!handleSetAOnly(ita)) {
            return;
        }
        ++ita;
    }
    while (itb != endb) {
        if (!handleSetBOnly(itb)) {
            return;
        }
        ++itb;
    }
}

std::string string_format(const std::string fmt, ...) {
    int size = 100;
    std::string str;
    va_list ap;
    while (1) {
        str.resize(size);
        va_start(ap, fmt);
        int n = vsnprintf((char *) str.c_str(), size, fmt.c_str(), ap);
        va_end(ap);
        if (n > -1 && n < size) {
            str.resize(n);
            return str;
        }
        if (n > -1) {
            size = n + 1;
        } else {
            size *= 2;
        }
    }
    return str;
}


struct Color {
    double r = 0, g = 0, b = 0;
};

#include <cmath>
Color hsv2rgb(double h, double s, double v) {
    double i;
    Color c;
    double f, p, q, t;
    while (h > 1)
        h -= 1.0;
    while (h < 0)
        h += 1.0;

    if (s == 0) {
        c.r = v;
        c.g = v;
        c.b = v;
    } else {
        h *= 6.0;
        i = floor(h);
        f = h - (double) i;
        p = v * (1 - s);
        q = v * (1 - s * f);
        t = v * (1 - s * (1 - f));
        switch ((int) i) {
            case 0:
                c.r = v;
                c.g = t;
                c.b = p;
                break;
            case 1:
                c.r = q;
                c.g = v;
                c.b = p;
                break;
            case 2:
                c.r = p;
                c.g = v;
                c.b = t;
                break;
            case 3:
                c.r = p;
                c.g = q;
                c.b = v;
                break;
            case 4:
                c.r = t;
                c.g = p;
                c.b = v;
                break;
            default:
                c.r = v;
                c.g = p;
                c.b = q;
                break;
        }
    }
    return c;
};


struct World {
    typedef std::size_t resourceindex;
    typedef std::size_t recipeindex;

    struct Recipe {
        std::string ClassName;
        recipeindex index;

        std::string mDisplayName;
        std::map<resourceindex, double> input, output, neteffect;

        std::string factorytype;
        double mManufactoringDuration; // in seconds.

        void calculateNet() {
            Recipe *this2 = this;
            sortedPairContainerComparison(input,
                                          output,
                                          [this2](const std::map<const resourceindex, double>::const_iterator &input_only) -> bool {
                                              this2->neteffect[input_only->first] = -input_only->second;
                                              return true;
                                          },
                                          [this2](const std::map<const resourceindex, double>::const_iterator &output_only) -> bool {
                                              this2->neteffect[output_only->first] = output_only->second;
                                              return true;
                                          },
                                          [this2](const std::map<const resourceindex, double>::const_iterator &input,
                                                  const std::map<const resourceindex, double>::const_iterator &output) -> bool {
                                              this2->neteffect[input->first] = output->second - input->second;
                                              return true;
                                          });
        }
    };

    struct Resource {
        std::string name;
        resourceindex index;
        std::string mDisplayName;
        bool isliquid;
        double energyMJ;
        bool isresource; // ores, oil, water are resources
        std::set<recipeindex> allprecursors, producedby, usedby; // not the resources but the recipes that directly or indirectly are involved with creating this resource.

    };

    std::map<std::string, resourceindex> resourcename_to_resourceindex;
    std::vector<Resource> resources;

    std::map<std::string, recipeindex> recipe_to_recipeindex;
    std::vector<Recipe> recipes;

    std::map<std::string, double> powerconsumptionmw; // for the buildings...

    resourceindex getResource(const std::string &n) {
        auto i = resourcename_to_resourceindex.find(n);
        if (i != resourcename_to_resourceindex.end()) {
            return i->second;
        }
        resourcename_to_resourceindex.insert(std::make_pair(n, resources.size()));
        resources.push_back(Resource{n, resources.size(), "", false, 0, false});
        return resources.size() - 1;
    }

    resourceindex getResourceNC(const std::string &n) const {
        auto i = resourcename_to_resourceindex.find(n);
        assertss(i != resourcename_to_resourceindex.end(), n);
        return i->second;
    }

    recipeindex getRecipe(const std::string &n) { // TODO: how can a string_view be used here?
        const auto i = recipe_to_recipeindex.find(n);
        if (i != recipe_to_recipeindex.end()) {
            return i->second;
        }
        recipe_to_recipeindex.insert(std::make_pair(n, recipes.size()));
        recipes.push_back(Recipe{n, recipes.size()});
        return recipes.size() - 1;
    }

    recipeindex getRecipeNC(const std::string &n) const {
        const auto i = recipe_to_recipeindex.find(n);
        assertss(i != recipe_to_recipeindex.end(), n);
        return i->second;
    }

    std::string getResourceDisplayName(std::string resourceclass) const {
        resourceclass = shortenItemName(resourceclass);
        const auto it = resourcename_to_resourceindex.find(resourceclass);
        if (it == resourcename_to_resourceindex.end() || resources[it->second].mDisplayName.empty()) {
            return resourceclass;
        } else {
            return resources[it->second].mDisplayName;
        }
    }

    std::string convertBracesFormatToJsonish(std::string in) const { // i am just not patient or good enough to get a decent parser+transform in only a few lines. Maybe one day. But not today.
        std::replace(in.begin(), in.end(), '"', '_');
        std::replace(in.begin(), in.end(), '\'', '_');
        std::replace(in.begin(), in.end(), '(', '{');
        std::replace(in.begin(), in.end(), ')', '}');
        std::replace(in.begin(), in.end(), '=', ':');

        std::regex e("([{}:,])([^\"{}:,]{1,})([{}:,])");
        std::string result, result2, result3;
        std::regex_replace(std::back_inserter(result), in.begin(), in.end(), e, "$1\"$2\"$3");
        std::regex_replace(std::back_inserter(result2), result.begin(), result.end(), e, "$1\"$2\"$3");
        std::regex e2("\"[^\"]*\\.([^\"\\.]*)__\"");
        std::regex_replace(std::back_inserter(result3), result2.begin(), result2.end(), e2, "\"$1\"");
        //result3 = result2;
        result3[0] = '[';
        result3[result3.size() - 1] = ']';

        //prt(in);
        //    prt(result2);
        prt(result3);
        //assert(false);
        return result3;
    }

    std::string shortenItemName(const std::string in) const {
        return in;
        std::regex e("Desc_(.*)_C");
        std::string shortinput;
        std::regex_replace(std::back_inserter(shortinput), in.begin(), in.end(), e, "$1");
        return shortinput;
    }

    void loadFromCommunityResourcesDocsJson(const std::string &jsonfilename) {
        FILE *fp = fopen(jsonfilename.c_str(), "rb");
        char readBuffer[65536];
        rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
        rapidjson::EncodedInputStream<rapidjson::UTF16LE<>, rapidjson::FileReadStream> eis(is);
        rapidjson::Document configdump;
        configdump.ParseStream<0, rapidjson::UTF16LE<> >(eis);
        assertss(configdump.IsArray(), " input is expected to be the <satisfactory-install>/communityresources/docs.json-file");


        for (const auto &partialconfig : configdump.GetArray()) {
            std::string nc(partialconfig["NativeClass"].GetString());
            if (nc == "Class'/Script/FactoryGame.FGItemDescriptor'" || nc == "Class'/Script/FactoryGame.FGResourceDescriptor'" || nc == "Class'/Script/FactoryGame.FGConsumableDescriptor'" || nc == "Class'/Script/FactoryGame.FGItemDescriptorNuclearFuel'" || nc == "Class'/Script/FactoryGame.FGItemDescriptorBiomass'") {
                bool isresource = nc == "Class'/Script/FactoryGame.FGResourceDescriptor'";
                for (const auto &someitemjson :  partialconfig["Classes"].GetArray()) {
                    Resource &r = resources[getResource(shortenItemName(someitemjson["ClassName"].GetString()))];
                    r.mDisplayName = someitemjson["mDisplayName"].GetString();
                    r.energyMJ = std::stod(someitemjson["mEnergyValue"].GetString());
                    r.isliquid = std::string(someitemjson["mForm"].GetString()) == "RF_LIQUID" || std::string(someitemjson["mForm"].GetString()) == "RF_GAS";
                    r.isresource = isresource;
                    prt4(r.name, r.isliquid, r.energyMJ, r.isresource);
                }
            }
        }


        for (const auto &partialconfig : configdump.GetArray()) {
            if (std::string(partialconfig["NativeClass"].GetString()) != "Class'/Script/FactoryGame.FGRecipe'") {
                continue;
            }
            for (const auto &jsonrecipe :  partialconfig["Classes"].GetArray()) {
//                const double durationminutes = std::stod(jsonrecipe["mManufactoringDuration"].GetString()) / 60.0;

                Recipe &recipe = recipes[getRecipe(jsonrecipe["ClassName"].GetString())];
                prt(jsonrecipe["ClassName"].GetString());
                recipe.mDisplayName = jsonrecipe["mDisplayName"].GetString();
                recipe.mManufactoringDuration = std::stod(jsonrecipe["mManufactoringDuration"].GetString());
                const std::string mingredients = convertBracesFormatToJsonish(jsonrecipe["mIngredients"].GetString());
                rapidjson::Document ingredientsjson;
                ingredientsjson.Parse(mingredients.c_str());
                assertss(ingredientsjson.IsArray(), mingredients);
                for (const auto &jsonrecipeinput : ingredientsjson.GetArray()) {
                    Resource &input = resources[getResource(jsonrecipeinput["ItemClass"].GetString())];
                    recipe.input[input.index] = std::stod(jsonrecipeinput["Amount"].GetString());
                    input.usedby.insert(recipe.index);
                }
                const std::string mproducts = convertBracesFormatToJsonish(jsonrecipe["mProduct"].GetString());
                rapidjson::Document productsjson;
                productsjson.Parse(mproducts.c_str());
                assertss(productsjson.IsArray(), mproducts);
                for (const auto &jsonrecipeoutput : productsjson.GetArray()) {
                    Resource &output = resources[getResource(jsonrecipeoutput["ItemClass"].GetString())];
                    prt(jsonrecipeoutput["ItemClass"].GetString());

                    recipe.output[output.index] = std::stod(jsonrecipeoutput["Amount"].GetString());
                    output.producedby.insert(recipe.index);
                }

//                World::resourceindex zucht = getResource(recipe.ClassName);
//
                std::string building = jsonrecipe["mProducedIn"].GetString();
                recipe.factorytype = building;
//                for (const auto &producer : powerconsumptionmw) {
//                    if (building.find(producer.first) != std::string::npos) {
//                        resources[zucht].isresource = true;
//                        recipe.input[zucht] += 1;
//
//                        //TODO: ALSO ADD THE COOLING WATER, not sure how to get it from the json though.
//                        recipe.input[getResourceNC("energyMJ")] = producer.second * 60.0; // watt is joule per second ... 60 seconds in a minute ... we are normalizing everything to minutes...
//                        //	      assert(false);// please trigger.
//                        break;
//                    }
//                }

                recipe.calculateNet();
                prt(recipe.mDisplayName);
            }

        }

        //    print();

        fclose(fp);

    }
//
//    void loadFromDerivedJson(const std::string &jsonfilename) {
//        FILE *fp = fopen(jsonfilename.c_str(), "rb");
//        char readBuffer[65536];
//        rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
//        rapidjson::EncodedInputStream<rapidjson::UTF16LE<>, rapidjson::FileReadStream> eis(is);
//        rapidjson::Document configdump;
//        configdump.ParseStream<0, rapidjson::UTF16LE<> >(eis);
//        assertss(configdump.IsArray(), " input is expected to be the <satisfactory-install>/communityresources/docs.json-file");
//
//
//        rapidjson::Document sfclasses;
//        sfclasses.ParseStream(is);
//        assert(sfclasses.IsArray());
//
//        for (const auto &currentclass : sfclasses.GetArray()) {
////            "NativeClass": "Class'/Script/FactoryGame.FGRecipe'",
//            prt(currentclass["NativeClass"].GetString());
//            if (strcmp(currentclass["NativeClass"].GetString() , "Class'/Script/FactoryGame.FGRecipe'")){
//                continue;
//            }
//
//            for (const auto& jsonrecipe : currentclass["Classes"].GetArray()){
//
//                Recipe &recipe = recipes[getRecipe(jsonrecipe["ClassName"].GetString())];
//                recipe.mDisplayName = jsonrecipe["mDisplayName"].GetString();
//                prt(recipe.mDisplayName);
//                const double durationminutes = std::stod(jsonrecipe["mManufactoringDuration"].GetString()) / 60.0;
//                for (const auto &jsonrecipeinput : jsonrecipe["mIngredients"].GetArray()) {
//                    Resource &input = resources[getResource(jsonrecipeinput["ItemClass"].GetString())];
//                    recipe.input[input.index] = std::stod(jsonrecipeinput["Amount"].GetString()) / durationminutes;
//                    input.usedby.insert(recipe.index);
//                }
//                for (const auto &jsonrecipeoutput : jsonrecipe["mProduct"].GetArray()) {
//                    Resource &output = resources[getResource(jsonrecipeoutput["ItemClass"].GetString())];
//                    recipe.output[output.index] = std::stod(jsonrecipeoutput["Amount"].GetString()) / durationminutes;
//                    output.producedby.insert(recipe.index);
//                }
//
//                std::string building = jsonrecipe["mProducedIn"].GetString();
//
//                recipe.factorytype = building;
//
//
//                recipe.calculateNet();
//            }
//
//        }
//        fclose(fp);
//    }
};

#include <iomanip>

void printMemory(char *c, const char *end) {
    std::cout << std::endl;
    std::cout << "MEMORY DUMP: " << std::endl;
    std::size_t counter = 0;
    bool currently_doing_readablechars = false;

    const std::string ascii = "0123456789ABCDEF";


    for (; c < end; ++c) {
        if (*c > 32 && *c < 127) {
            if (!currently_doing_readablechars) {
                std::cout << "              <<< " << counter << " non-readable chars>>>" << std::endl;
                counter = 0;
            }
            currently_doing_readablechars = true;
            std::cout << (char) *c;

        } else {
            if (currently_doing_readablechars) {
                std::cout << "|" << std::endl;
            }
            currently_doing_readablechars = false;
            counter++;
//            std::cout << std::hex << std::setfill('0') << std::setw(2)  << (uint8_t) *c << std::dec;

            if (((std::size_t) c % 4) == 0) {
                std::cout << " ";
            }

            std::cout << ascii[((uint8_t) *c) >> 4] << ascii[((uint8_t) *c) % 4];


        }

    }
    if (currently_doing_readablechars) {
        std::cout << "|" << std::endl;
    } else {
        std::cout << "              <<< " << counter << " non-readable chars>>>" << std::endl;
    }

    std::cout << std::endl;
    std::cout.flush();
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
struct MemSize<int8_t> {
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
    int32_t buildVersion;

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


    int32_t readBuildVersion() {
        int32_t ret = inc<int32_t>();
        buildVersion = ret;
        return ret;
    }

    template<typename T>
    Builder &skip() {
        offset += MemSize<T>::v;//sizeof(T);
        return *this;
    }

    Builder &skip(const std::size_t n) {
        offset += n;
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

const int CultureInvariantChangeBuild = 140822;

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

    RepeatedPrimitiveType(int32_t n_, T *v_) : n(n_), v(v_) {

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


auto eq = [](const std::string &a, const RepeatedPrimitiveType<char> &b) -> bool {
    return a.size() + 1 == b.size() && memcmp(a.c_str(), b.v, a.size()) == 0;
};

auto contains = [](const std::string &a, const RepeatedPrimitiveType<char> &b) -> bool {
    const std::string grr(b.v, b.n);
    return grr.find(a) != std::string::npos;
};


template<typename T>
struct RepeatedType : public std::vector<T> {
    RepeatedType(Builder &b) {
        //printMemory(b.ptr+ b.offset, b.ptr + b.offset + 4);
        int32_t n = b.inc<int32_t>();
        assert(n < 1000000);
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
    int32_t flags;
    uint8_t history_type;
    int32_t HasCultureInvariantString;

    std::optional<RepeatedPrimitiveType<char> > CultureInvariantString, s1, value;

    struct NameData {
        RepeatedPrimitiveType<char> name;
        //std::array<uint8_t, 14> mysterystuff
        RepeatedPrimitiveType<char> key;
        RepeatedPrimitiveType<char> data;
    };

    std::vector<NameData> namedatas;


    enum ETextHistoryType {
        None = 255,
        Base = 0,
        NamedFormat,
        OrderedFormat,
        ArgumentFormat,
        AsNumber,
        AsPercent,
        AsCurrency,
        AsDate,
        AsTime,
        AsDateTime,
        Transform,
        StringTableEntry,
        TextGenerator,
    };


    TextProperty(Builder &b, bool inarray = false) {
        // logic from Goz3rr_SatisfactorySaveEditor  TextProperty.cs
        debugloc.push_back("TextProperty");
        printDebugLoc();

//        if (!inarray){
//            b.assertNullByte();
//        }

        flags = b.inc<int32_t>();
        history_type = b.inc<uint8_t>();
        prt((int) history_type);
        switch (history_type) {
            case ETextHistoryType::Base: // 0
                namedatas.emplace_back(NameData{RepeatedPrimitiveType<char>(b),
//                        b.inc<std::array<uint8_t, 14> >(),
                                                RepeatedPrimitiveType<char>(b),
                                                RepeatedPrimitiveType<char>(b)});
                break;
            case ETextHistoryType::ArgumentFormat: // 3
            {
                printMemory(b.ptr + b.offset, b.ptr + b.offset + 1000);
                TextProperty sourceFmt(b);
                const int32_t argumentsCount = b.inc<int32_t>();
                assertss(argumentsCount < 10000, pt(argumentsCount));
                for (int32_t i = 0; i < argumentsCount; ++i) {

                    RepeatedPrimitiveType<char> name(b);
                    const int8_t valueType = b.inc<int8_t>();
                    switch (valueType) {
                        case 4: {
                            TextProperty argumentValue(b);
                            break;
                        }
                        default:
                            assert(false);
                    }

                }

//                assert(false);
                break;
            }
            case ETextHistoryType::None:
                // nothing to be done.
                if (b.buildVersion >= CultureInvariantChangeBuild) {
                    HasCultureInvariantString = b.inc<int32_t>();
                    if (HasCultureInvariantString == 1) {
                        CultureInvariantString.emplace(b);
                    }
                }

                break;
            default:
                assert(false);
        }
//        if (mystery_choice == 0x3) {
//            b.skip<int32_t>();
//            b.skip<uint8_t>();
//        } else if (mystery_choice == 0x255) {
//            debugloc.pop_back();
//            return;
//        }
//        b.assertNullInt();
//        s1.emplace(b);
//        value.emplace(b);
//        prt2(*s1, *value);

        //if (mystery_choice == 0x3)
        /*{
            int32_t count = b.inc<int32_t>();
            for (uint32_t i = 0; i < count; ++i) {
                namedatas.emplace_back(NameData{RepeatedPrimitiveType<char>(b), b.inc<std::array<uint8_t, 14> >(), RepeatedPrimitiveType<char>(b)});
                const auto &nd = *namedatas.rbegin();
                prt2(nd.name, nd.data);
            }
        }*/
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

struct SerializedFields;

struct SerializedFields {

    std::vector<std::unique_ptr<PropertyType> > props;

    SerializedFields(Builder &b, int32_t &len);

    SerializedFields(Builder &b);
};

struct MoreInventoryStuff {
// for (let a = 0; a < t; a++) this.saveParser.objects[e].extra.items.push({length: this.readInt(), name: this.readString(), levelName: this.readString(), pathName: this.readString(), position: this.readFloat()})
    int32_t length;
    RepeatedPrimitiveType<char> name;
    RepeatedPrimitiveType<char> levelName;
    RepeatedPrimitiveType<char> pathName;
    float position;


    MoreInventoryStuff(Builder &b) : length(b.inc<int32_t>()),
                                     name(b),
                                     levelName(b),
                                     pathName(b),
                                     position(b.inc<float>()) {

    }
};


struct SaveEntity /*: public SaveObject*/ {


    int32_t NeedTransform;// = reader.ReadInt32() == 1;
    std::array<float, 4> Rotation;// = reader.ReadVector4();
    std::array<float, 3> Position;// = reader.ReadVector3();
    std::array<float, 3> Scale; //= reader.ReadVector3();
    int32_t WasPlacedInLevel; //= reader.ReadInt32() == 1;

    std::optional<RepeatedPrimitiveType<char> > ParentObjectRoot; // "this.saveParser.objects[e].entityLevelName"
    std::optional<RepeatedPrimitiveType<char> > ParentObjectName; // "this.saveParser.objects[e].entityPathName"
    std::optional<RepeatedType<ObjectReference> > components; //this.saveParser.objects[e].children

    std::optional<SerializedFields> DataFields;//,DataFields2,DataFields3,DataFields4; // "this.saveParser.objects[e].properties"
//    std::optional<RepeatedPrimitiveType<PropertyType> > properties;
    std::int32_t extra_count;
    std::optional<RepeatedType<MoreInventoryStuff> > extra;

    SaveEntity(Builder &b) : //SaveObject(b),   // readActorV5, type == 1
            NeedTransform(b.inc<int32_t>()),
            Rotation(b.inc<std::array<float, 4>>()),
            Position(b.inc<std::array<float, 3>>()),
            Scale(b.inc<std::array<float, 3> >()),
            WasPlacedInLevel(b.inc<int32_t>()) {
    }

    void parseAdditionalData(Builder &b, const int32_t end_of_struct, const RepeatedPrimitiveType<char> &className);

};


struct Properties : std::vector<std::unique_ptr<PropertyType> > {
    Properties(Builder &b);
};

struct SaveComponent {//: public SaveObject {
    RepeatedPrimitiveType<char> ParentEntityName;

    std::optional<SerializedFields> DataFields; // "this.saveParser.objects[e].properties"

    SaveComponent(Builder &b) : // SaveObject(b),
            ParentEntityName(b) {
//        prt(ParentEntityName);
    }

    void parseAdditionalData(Builder &b, int32_t &remainingbytes) {
//        assert(false);
        //b.offset += len;
        int a = b.offset;
        DataFields.emplace(b, remainingbytes);


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
//        prt4(unknown, itemtype, unknown2, unknown3);
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
//        assert(false);
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

//struct StructTrainSimulationData {
//    static const std::string tname;
//
//    StructTrainSimulationData(Builder &b) {
//        assert(false);
//    }
//};

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

struct StructDroneDockingStateInfo {
    static const std::string tname;

    /*   EnumProperty "          EDroneDockingState     EDroneDockingState::DS_DOCKED    None    mDockedStation    ObjectProperty Z           Persistent_Level A   Persistent_Level:PersistentLevel.Build_DroneStation_C_2147034464
   mHomeStation    ObjectProperty Z           Persistent_Level A   Persistent_Level:PersistentLevel.Build_DroneStation_C_2147034464    mHealthComponent    ObjectProperty i           Persistent_Level P   Persistent_Level:PersistentLevel.BP_DroneTransport_C_2147006949.HealthComponent    mBuiltWithRecipe    ObjectProperty Z            R   /Game/FactoryGame/Recipes/Buildings/Recipe_DroneTransport.Recipe_DroneTransport_C    mPrimaryColor    StructProperty           LinearColor                  ��t?���>6s�=  �?   mSecondaryColor    StructProperty           LinearColor                  � �=�-�=��>  �?
   mIsSimulated */

    StructDroneDockingStateInfo(Builder &b) {

        std::cerr << std::string(b.ptr + b.offset, b.ptr + b.offset + 10000) << std::endl;
        assert(false);

    }

};


struct StructProperty {
    static const std::string tname;
    RepeatedPrimitiveType<char> structtype;

    std::array<char, 17> unknown;
//    int32_t unk1, unk2, unk3, unk4;
//    char unk5;
    std::variant<std::monostate, StructBox, StructColor, StructCompletedResearch, StructFluidBox, StructGuid, StructHotbar, StructInventoryItem, StructInventoryStack, StructItemAmount, StructItemFoundData, StructLinearColor, StructMessageData, StructPhaseCost, StructProjectileData, StructQuat, StructRailroadTrackPosition, StructRecipeAmountStruct, StructRemovedInstanceArray, StructResearchCost, StructResearchData, StructResearchRecipeReward, StructRotator, StructSlateBrush, StructSplinePointData, StructTimerHandle, StructTransform, StructVector, SerializedFields> structcontent;


    StructProperty(Builder &b);
};

const std::string StructProperty::tname = "StructProperty";


struct MapProperty {
    static const std::string tname;
    RepeatedPrimitiveType<char> keyType;
    RepeatedPrimitiveType<char> valueType;
    int32_t count;

    struct KeyValue {
        std::variant<std::monostate, int32_t, uint8_t, ObjectReference, EnumProperty, RepeatedPrimitiveType<char> > key; //,  RepeatedPrimitiveType<int32_t>, RepeatedPrimitiveType<uint8_t>
        std::variant<std::monostate, int32_t, std::vector<PropertyType>, uint8_t> value;
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
        std::variant<std::monostate, std::vector<StructLinearColor>, std::vector<SerializedFields>, std::vector<StructVector> > properties; //std::vector<std::unique_ptr<PropertyType> > ,

        StructBS(Builder &b);
    };

    std::variant<std::monostate,
            RepeatedType<ObjectReference>,
            StructBS, // this is a list of keyvaluepairs it seems?
            RepeatedPrimitiveType<int32_t>,
            RepeatedPrimitiveType<uint8_t>,
            RepeatedType<InterfaceProperty>,
            RepeatedType<RepeatedPrimitiveType<char> >
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
//const std::string StructTrainSimulationData::tname = "TrainSimulationData";
const std::string StructTransform::tname = "Transform";
const std::string StructVector::tname = "Vector";
const std::string StructDroneDockingStateInfo::tname = "DroneDockingStateInfo";

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
            int8_t,
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


void SaveEntity::parseAdditionalData(Builder &b, const int32_t end_of_struct, const RepeatedPrimitiveType<char> &className) {

//        this.saveParser.objects[e].entityLevelName = this.readString(), this.saveParser.objects[e].entityPathName = this.readString();
//        let t = this.readInt();
//        if (t > 0) {
//            this.saveParser.objects[e].children = [];
//            for (let a = 0; a < t; a++) this.saveParser.objects[e].children.push({levelName: this.readString(), pathName: this.readString()})
//        }




//        std::size_t start_offset = b.offset;
//        remainingbytes -= 12;
    ParentObjectRoot.emplace(b);
//        if (ParentObjectRoot.value().n > 0) {
//            remainingbytes -= ParentObjectRoot.value().n + 1;
//        }
    assert(ParentObjectRoot.has_value());

    ParentObjectName.emplace(b);
//        if (ParentObjectName.value().n > 0) {
//            remainingbytes -= ParentObjectName.value().n + 1;
//        }

    prt2(*ParentObjectRoot, *ParentObjectName);
    components.emplace(b);  // "children"
    prt2(components->size(), components.value().size()); // -> 2 children/components per conveyor i guess.


    for (const auto &i : *components) {
        prt2(i.pathName, i.levelName);
//            remainingbytes -= 10 + i.pathName.n + i.levelName.n;
    }
    if (b.offset + 5 > end_of_struct) {
        return;
    }

//    for (this.saveParser.objects[e].properties = []; ;) {
//        let t = this.readPropertyV5();
//        if (null === t) break;
//        this.saveParser.objects[e].properties.push(t)
//    }

    DataFields.emplace(b);
    if (contains("/Build_ConveyorBeltMk", className)) {

        // "properties", but what type is this exactly ...
        extra_count = 0;
        if (b.offset + 5 > end_of_struct) {
            return;
        }

        prt2(end_of_struct - b.offset, className);
        //
        //properties.emplace(b);
//        PropertyT ype wtf(b);
        //DataFields2.emplace(b);
        //DataFields3.emplace(b);
        //DataFields4.emplace(b);
        prt(DataFields.value().props.size());
        //prt4(DataFields.value().props.size(),DataFields2.value().props.size(),DataFields3.value().props.size(),DataFields4.value().props.size()  );

//        std::cerr << "df1: ";
//        for (auto &i : DataFields.value().props) {
//            std::cerr << i->name << " ";
//        }
//        std::cerr << std::endl;
//        std::cerr << "df2: ";
//        for (auto& i : DataFields2.value().props ){
//            std::cerr << i->name << " ";
//        }
//        std::cerr << std::endl;
//        std::cerr << "df3: ";
//        for (auto& i : DataFields3.value().props ){
//            std::cerr << i->name << " ";
//        }
//        std::cerr << std::endl;
//        std::cerr << "df4: ";
//        for (auto& i : DataFields4.value().props ){
//            std::cerr << i->name << " ";
//        }


        // 15 bytes before string mmmm... mmm???
        //b.skip(3);

        if (b.offset + 5 > end_of_struct) {
            return;
        }

        // extra_count itemcount firstitemlen  namelen  -> 4 x 4 = 16 ...

        extra_count = b.inc<int32_t>();
        extra.emplace(b);

        //printMemory(b.ptr + b.offset, b.ptr + end_of_struct);
    }

}

SerializedFields::SerializedFields(Builder &b, int32_t &len) {
    std::size_t start = b.offset;

    while (props.empty() || !eq("None", (*props.rbegin())->name)) {

        props.emplace_back(std::make_unique<PropertyType>(b));
    }
    if (!props.empty()) {
        assert(eq("None", (*props.rbegin())->name));
        props.pop_back();
    }

//    std::cerr << "SERIALIZED FIELDS CONSTRUCTED, SIZE " << props.size() << std::endl;
    //b.skip<int32_t>();
    //b.assertNullInt();
//    std::size_t remaining = start + len - b.offset;
//    if (remaining && remaining < 18446744073709000000) {
//       prt(remaining);
//       //assert(remaining < 18446744073709000000);
//       b.skip(remaining);
//    }


// ???
}

SerializedFields::SerializedFields(Builder &b) {
    std::size_t start = b.offset;

    while (props.empty() || !eq("None", (*props.rbegin())->name)) {

        props.emplace_back(std::make_unique<PropertyType>(b));
    }
    if (!props.empty()) {
        assert(eq("None", (*props.rbegin())->name));
        props.pop_back();
    }
//    std::cerr << "SERIALIZED FIELDS CONSTRUCTED, SIZE " << props.size() << std::endl;


// ???
}

//readPropertyV5::ArrayProperty
ArrayProperty::StructBS::StructBS(Builder &b) : count(b.inc<int32_t>()),  // let r = this.readInt();
                                                structName(b), //e.value = {type: this.readString(), values: []}, this.skipBytes();
                                                structType(b), // e.structureName = this.readString(), e.structureType = this.readString(), this.readInt(), this.readInt(), e.structureSubType = this.readString();
                                                structSize(b.inc<int32_t>()),
                                                structInnerType(b.assertNullInt()),
                                                guid(b.inc<std::array<char, 16>>()) { //let t = this.readInt(), a = this.readInt(), i = this.readInt(), o = this.readInt();
    b.assertNullByte(); // this.skipBytes(1);
    prt2(structName, structInnerType);

    //assert(eq("StructProperty", structName));

//    int32_t t = b.inc<int32_t>();
//    int32_t a = b.inc<int32_t>();
//    int32_t i = b.inc<int32_t>();
//    int32_t o = b.inc<int32_t>();


    const std::vector<std::string> sfnames = {"SpawnData",
                                              "Transform",
                                              "PhaseCost",
                                              "ItemAmount",
                                              "TimeTableStop",
                                              "RemovedInstance",
                                              "SplinePointData",
                                              "InventoryStack",
                                              "ResearchData",
                                              "SchematicCost",
                                              "ItemFoundData",
                                              "ScannableResourcePair",
                                              "Hotbar",
                                              "PresetHotbar",
                                              "MessageData",
                                              "SplitterSortRule",
                                              "FeetOffset",
                                              "RecipeAmountStruct",
                                              "DroneTripInformation",
                                              "ResearchTime",
                                              "ResearchCost",
                                              "CompletedResearch",
                                              "SubCategoryMaterialDefault",
                                              "FactoryCustomizationColorSlot",
                                              "PrefabTextElementSaveData",
                                              "PrefabIconElementSaveData",
                                              "GlobalColorPreset",
                                              "LampGroup",
                                              "STRUCT_ProgElevator_Floor",
                                              "InserterBuildingProfile",
                                              "BRN_Base_FrackingSatelliteInfo",
                                              "FINCommandLabelReferences",
                                              "FINCommandLabelData",
                                              "MiniGameResult"};
//structName = 'mTetrominoLeaderBoard'	structInnerType = 'MiniGameResult'

    for (const auto &sfname : sfnames) {
        if (eq(sfname, structInnerType)) {
            auto &v = properties.emplace<std::vector<SerializedFields> >();
            for (int i = 0; i < count; ++i) {
                v.emplace_back(b);
            }
            return;
        }
    }

    if (eq(StructLinearColor::tname, structInnerType)) {
        auto &v = properties.emplace<std::vector<StructLinearColor>>();
        for (int32_t i = 0; i < count; ++i) {
            v.emplace_back(b);
        }
    }else if (eq(StructVector::tname, structInnerType)) {
        auto &v = properties.emplace<std::vector<StructVector>>();
        for (int32_t i = 0; i < count; ++i) {
            v.emplace_back(b);
        }

    } else {
        printMemory(b.ptr + b.offset, b.ptr + b.offset + 900);
        assert(false);

    }


}


ArrayProperty::ArrayProperty(Builder &b) :
        itemType(b) {
    b.assertNullByte();
    //prt(itemType);

    if (eq(ObjectReference::tname, itemType)) {
        content.emplace<RepeatedType<ObjectReference> >(b);
    } else if (eq(StructProperty::tname, itemType)) {
//        std::cerr << "AHA ArrayProperty WITH STRUCTBS" << std::endl;

        content.emplace<StructBS>(b);
//        std::get<std::vector<StructBS>>(content).emplace_back(b);
//        while ( !eq("None", std::get<std::vector<StructBS>>(content).rbegin()->structName )   ){
//            std::get<std::vector<StructBS>>(content).emplace_back(b);
//        }

//        printMemory(b.ptr + b.offset, b.ptr + b.offset + 900);
//        std::cerr << "STRUCTBS done, count " << std::get<StructBS>(content).count << std::endl;

    } else if (eq("IntProperty", itemType)) {
        content.emplace<RepeatedPrimitiveType<int32_t> >(b);
    } else if (eq("ByteProperty", itemType)) {
        content.emplace<RepeatedPrimitiveType<uint8_t> >(b);
    } else if (eq(InterfaceProperty::tname, itemType)) {
        content.emplace<RepeatedType<InterfaceProperty> >(b);
    } else if (eq(EnumProperty::tname, itemType)) {
        content.emplace<RepeatedType<RepeatedPrimitiveType<char>>>(b);

    } else {
        prt(itemType);
        assert(false);
    }

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
//    prt(structtype);
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
//    } else if (eq(StructTrainSimulationData::tname, structtype)) {
//        structcontent.emplace<StructTrainSimulationData>(b);
    } else if (eq(StructTransform::tname, structtype)) {
        structcontent.emplace<StructTransform>(b);
    } else if (eq(StructVector::tname, structtype)) {
        structcontent.emplace<StructVector>(b);
//    } else if (eq(StructDroneDockingStateInfo::tname, structtype)) {
//        structcontent.emplace<StructDroneDockingStateInfo>(b);


        /*                                    case"SpawnData":
                                    case"Transform":
                                    case"PhaseCost":
                                    case"ItemAmount":
                                    case"TimeTableStop":
                                    case"RemovedInstance":
                                    case"SplinePointData":
                                    case"InventoryStack":
                                    case"ResearchData":
                                    case"SchematicCost":
                                    case"ItemFoundData":
                                    case"ScannableResourcePair":
                                    case"Hotbar":
                                    case"PresetHotbar":
                                    case"MessageData":
                                    case"SplitterSortRule":
                                    case"FeetOffset":
                                    case"RecipeAmountStruct":
                                    case"DroneTripInformation":
                                    case"ResearchTime":
                                    case"ResearchCost":
                                    case"CompletedResearch":
                                    case"SubCategoryMaterialDefault":
                                    case"FactoryCustomizationColorSlot":
                                    case"PrefabTextElementSaveData":
                                    case"PrefabIconElementSaveData":
                                    case"GlobalColorPreset":
                                    case"LampGroup":
                                    case"STRUCT_ProgElevator_Floor":
                                    case"InserterBuildingProfile":
                                    case"BRN_Base_FrackingSatelliteInfo":
                                    case"FINCommandLabelReferences":
                                    case"FINCommandLabelData":*/



    } else {

        structcontent.emplace<SerializedFields>(b);


        prt(structtype);
        std::cout.flush();
        //assert(false); // unknown structtype
    }
    debugloc.pop_back();
}


PropertyType::PropertyType(Builder &b) :  //readPropertyV5
        name(b) {
    const std::string typename_None = "None";
    debugloc.push_back("PropertyType_" + std::string(name.v));
    printDebugLoc();
// mRemovedInstances
//
//    auto eq = [](const std::string &a, const RepeatedPrimitiveType<char> &b) -> bool {
//        return a.size() + 1 == b.size() && memcmp(a.c_str(), b.v, a.size()) == 0;
//    };
//    prt(name);
    if (eq(typename_None, name)) {
        printDebugLoc();
        debugloc.pop_back();
        return;
    }
    header.emplace(b);
//    prt(header->fieldType);
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
    } else if (eq("Int8Property", header->fieldType)) {
        content.emplace<int8_t>(b.assertNullByte().inc<int8_t>());

    } else if (eq("ByteProperty", header->fieldType)) {
        content.emplace<ByteProperty>(b);



//    } else if (header->fieldType.n > 40 && header->fieldType < 160) {
//        // just assume
    } else {
        prt4(name, header->fieldType, header->size, header->index);
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
    RepeatedPrimitiveType<char> typePath; // className
    RepeatedPrimitiveType<char> rootObject;
    RepeatedPrimitiveType<char> instanceName;
    int32_t skippedBytes;

//    union {
//        SaveEntity entity;
//        SaveComponent component;
//    } payload;
    std::variant<std::monostate, SaveEntity, SaveComponent> v;

//    std::optional<SerializedFields> sf;
//    std::vector<PropertyType> serializedFields;

    WorldObject(Builder &b) :
            worldobjecttype(b.inc<int32_t>()),
            typePath(b),
            rootObject(b),
            instanceName(b) {
        switch (worldobjecttype) {
            case 1:
                v.emplace<SaveEntity>(b); //readActorV5
                break;
            case 0:
                v.emplace<SaveComponent>(b);  // readObjectV5
                break;
            default:
                prt(worldobjecttype);
                std::cout.flush();
                assert(false);
        }
    }

    void parseAdditionalData(Builder &b) { //readEntityV5
        int32_t len = b.inc<int32_t>();
        std::size_t start_offset = b.offset;  // "before"
        prt((int) worldobjecttype);

        int32_t remainingbytes = len;
        int32_t end_of_struct = start_offset + len;

        switch (worldobjecttype) {
            case 1:
                std::get<SaveEntity>(v).parseAdditionalData(b, end_of_struct, typePath);

                break;
            case 0:
                std::get<SaveComponent>(v).parseAdditionalData(b, remainingbytes);

                break;
            default:
                prt(worldobjecttype);
                std::cout.flush();
                assert(false);
        }



        //assert(remainingbytes == 0);
        if (start_offset + len > b.offset) {
            skippedBytes = start_offset + len - b.offset;
            b.skip(start_offset + len - b.offset);
        }




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
//        while (true) {
//            remainingbytes = (start_offset + len - b.offset);
//            //prt(remainingbytes);
//            assert(remainingbytes > 0);
//            auto &keyvalue = serializedFields.emplace_back(b);
//            /*std::cout << "***************************\n";
//            printDebugLoc();*/
//            if (!serializedFields.rbegin()->header.has_value()) {
////                std::cout << "NO HEADER -> BREAK (parseAdditionalData)\n";
//                serializedFields.pop_back();
//                break;
//            }
//        }
////
//        skippedBytes = start_offset + len - b.offset;
//        if (skippedBytes != 4) {
//            std::cout << "setting offset to " << (start_offset + len) << " while it currently is " << b.offset << " skippedbytes: " << skippedBytes << '\n';
//        }
////        assert(start_offset + len - b.offset == 4);
////assert(b.offset == start_offset + len);
//        b.offset = start_offset + len;
//        SerializedFields.emplace(b);

    }
};


MapProperty::KeyValue::KeyValue(Builder &b, const RepeatedPrimitiveType<char> &keytype, const RepeatedPrimitiveType<char> &valuetype) {

//    b.skip<char>();
//    b.skip<char>();
//    if (eq("IntProperty", keytype)) {
//        key.emplace<int32_t>(b/*.assertNullByte()*/.inc<int32_t>());
//    } else {
//        prt(keytype);
//        assert(false);
//    }

//    auto switchtypes = [ ](auto & keytype, auto& destination  ) {
//
//
//    };

    if (eq(ObjectReference::tname, keytype)) {
        key.emplace<ObjectReference>(b);
//    } else if (eq(StructProperty::tname, keytype)) {
//
//        key.emplace<StructBS>(b);
    } else if (eq("IntProperty", keytype)) {
        key.emplace<int32_t>(b.inc<int32_t>());
    } else if (eq("ByteProperty", keytype)) {
        key.emplace<uint8_t>(b.inc<uint8_t>());
//    } else if (eq(InterfaceProperty::tname, keytype)) {
//        key.emplace<RepeatedType<InterfaceProperty> >(b);
    } else if (eq("EnumProperty", keytype)) {
        printMemory(b.ptr + b.offset, b.ptr + b.offset + 100);
        key.emplace<RepeatedPrimitiveType<char>>(b);
    } else {

        prt(keytype);
        printMemory(b.ptr + b.offset, b.ptr + b.offset + 100);
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
    } else if (eq("IntProperty", valuetype)) {
        value.emplace<int32_t>(b.inc<int32_t>());

    } else if (eq("ByteProperty", valuetype)) {
        value.emplace<uint8_t>(b.inc<uint8_t>());
    } else {
        prt(valuetype);
        assert(false);
    }

}

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
        assert(currentChunkUncompressedLength == currentChunkUncompressedLength2);
        prt4(packageFileTag, maximumChunkSize, currentChunkCompressedLength, currentChunkUncompressedLength2);
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


bool mergeEqClasses(std::vector<std::size_t> &eqclasses, std::size_t a_i, std::size_t b_i) {

    while (eqclasses[a_i] > a_i) {
        a_i = eqclasses[a_i];
    }

    while (eqclasses[b_i] > b_i) {
        b_i = eqclasses[b_i];
    }

    std::size_t a_begin = eqclasses[a_i];
    std::size_t b_begin = eqclasses[b_i];
    if (a_begin == b_begin) {
        // they are linked already!
        return false;
    }

    std::size_t combined_begin;
    std::size_t combined_i;

    auto setReturnRef = [&combined_i, &eqclasses, &combined_begin]() {
        while (combined_i < eqclasses[combined_i]) {
            combined_i = eqclasses[combined_i];
        }
        eqclasses[combined_i] = combined_begin;
    };

    if (a_begin < b_begin) {
        combined_begin = a_begin;
        combined_i = combined_begin;
        a_i = eqclasses[a_begin];
        b_i = b_begin;
        if (a_i == a_begin) {
            eqclasses[combined_begin] = b_i;
            setReturnRef();
            return true;
        }
    } else {
        combined_begin = b_begin;
        combined_i = combined_begin;
        a_i = a_begin;
        b_i = eqclasses[b_begin];
        if (b_i == b_begin) {
            eqclasses[combined_begin] = a_i;
            setReturnRef();
            return true;
        }
    }


// somehow at the end of a previous merge the other_mrss_member_partindex was not set to combined_begin
    while (true) {
        if (a_i < b_i) {
            eqclasses[combined_i] = a_i;
            combined_i = a_i;

            a_i = eqclasses[a_i];

            if (a_i == a_begin) {
                eqclasses[combined_i] = b_i;
                //return;
                break;
            }
        } else {
            eqclasses[combined_i] = b_i;
            combined_i = b_i;
            b_i = eqclasses[b_i];

            if (b_i == b_begin) {
                eqclasses[combined_i] = a_i;
                //return;
                break;
            }
        }
    }
    setReturnRef();
    return true;
}


void mergeEqClasses(std::vector<std::size_t> &eqclasses, const std::vector<std::size_t> &tobemerged) {
    for (std::size_t i = 1; i < tobemerged.size(); ++i) {
        mergeEqClasses(eqclasses, tobemerged[0], tobemerged[i]);
    }

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
            wo.parseAdditionalData(b); //readEntityV5
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


    void studyTopology(const World &w) {

        // the nodes of our graph will be elements in worldObjects
        // the way to find a node by name will be through findWorldObject() which uses lookup table ordered_indexes

        // the choice of the inputs is usually irrelevant ... but the choice of the factory outputs is important (refineries...)

//        std::vector<std::pair<std::size_t,std::size_t> >  edges;


        std::map<std::size_t, std::size_t> circuitid_to_index;
        std::map<std::string, std::size_t> instance_to_circuitid;

        for (std::size_t i = 0; i < worldObjects.size(); ++i) {
            const WorldObject &wo = worldObjects[i];
            if (!eq("/Script/FactoryGame.FGPowerCircuit", wo.typePath)) {
                continue;
            }
            std::size_t circuitid = 0;
            std::vector<std::string> components;
            const SaveComponent &sc = std::get<SaveComponent>(wo.v);
            if (!sc.DataFields.has_value()) {
                continue;
            }
            for (const std::unique_ptr<PropertyType> &d : sc.DataFields.value().props) {
                if (!d) {
                    continue;
                }
                if (eq("mCircuitID", d->name)) {
                    circuitid = std::get<int32_t>(d->content);
                    circuitid_to_index[circuitid] = i;
                }
                if (eq("mComponents", d->name)) {
                    const ArrayProperty &ap = std::get<ArrayProperty>(d->content);
                    const auto &ors = std::get<RepeatedType<ObjectReference> >(ap.content);
                    for (std::size_t j = 0; j < ors.size(); ++j) {
                        components.push_back(std::string(ors[j].pathName.v));
                    }

                }
            }
            for (const auto &c : components) {
                instance_to_circuitid[c] = circuitid;
            }
        }


        std::vector<std::size_t> eqclasses(worldObjects.size()); // the intent is to ... merge what belongs together.
        for (std::size_t i = 0; i < worldObjects.size(); ++i) {
            eqclasses[i] = i; // to begin with each object is its own eqclass
        }

        for (std::size_t i = 0; i < worldObjects.size(); ++i) {

            if (worldObjects[i].worldobjecttype != 0) {
                continue;
            }
            const SaveComponent &sc = std::get<SaveComponent>(worldObjects[i].v);
            if (!sc.DataFields.has_value()) {
                continue;
            }

            for (const auto &d : sc.DataFields.value().props) {
                if (!d || !eq("mConnectedComponent", d->name)) {
                    continue;
                }
                const ObjectReference &obr = std::get<ObjectReference>(d->content);

                const std::size_t other = findWorldObject(obr.pathName);
                assert(other < worldObjects.size());

                mergeEqClasses(eqclasses, i, other);
                if (contains("Conveyor", sc.ParentEntityName) || contains("StorageContainer", sc.ParentEntityName) || contains("Pipeline", sc.ParentEntityName) || contains("Valve", sc.ParentEntityName) || contains("PipeStorageTank", sc.ParentEntityName)) {
                    const std::size_t parent = findWorldObject(sc.ParentEntityName);
                    assert(parent < worldObjects.size());
                    mergeEqClasses(eqclasses, i, parent);
                }
                break;
            }


        }
        // iron screws for ... rotors?
        //mergeEqClasses(eqclasses, {273554, 256492, 273634, 256724, 273432, 273458, 273520, 273540 });


        // plutonium collider outputs
        //mergeEqClasses(eqclasses, {282564, 280102, 263244, 282542,  282562, 280096, 263350, 265794, 282442, 282558, 282458, 263282, 280052, 282560, 263254, 282540, 282454, 279638 , 279476 , 282440, 263356, 282456, 263352, 282444  });

        // todo: invent algo that tries to merge eqclasses when they have identical behaviour
        //      probably a iterative attemptMerge is possible, whereby we start from a single node, create the set of candidate-eqclass-members, but we postpone the merge till the members have lengthened enough so that they meet at the other end...
        //


        auto isRelevant = [&](std::size_t i) -> bool {
            const auto &wo = worldObjects[i];

            if (contains("Build_FrackingExtractor", worldObjects[i].typePath) || contains("Build_MinerMk", worldObjects[i].typePath) || contains("OilPump", worldObjects[i].typePath) || contains("WaterPump", worldObjects[i].typePath)) {
                return true;
            }

            return !(contains("Conveyor", wo.instanceName) || contains("StorageContainer", wo.instanceName) || contains("Pipeline", wo.instanceName) || contains("Valve", wo.instanceName) || contains("PipeStorageTank", wo.instanceName) || contains("IndustrialTank", wo.instanceName));
        };


        struct AggregatingId {
            // we want to group together all buildings that use the same recipe and are attached in the same way to the same set of eqclasses. (assuming that they will be attached ... the same way?? ... that is ... not the case ...)
            std::string recipe; // recipe is building-specific, no need to compare buildingtypes
            std::size_t circuitid = 0;
            //std::string connection_postfix; // "output0",... but only for outputs, it does not matter for inputs
            std::map<std::string, std::size_t> outputs_eqclasses;
            std::set<std::size_t> inputeqclasses;


            bool operator<(const AggregatingId &o) const {
                if (recipe != o.recipe) {
                    return recipe < o.recipe;
                }
                if (circuitid != o.circuitid) {
                    return circuitid < o.circuitid;
                }
                if (outputs_eqclasses != o.outputs_eqclasses) {
                    return outputs_eqclasses < o.outputs_eqclasses;
                }
                return inputeqclasses < o.inputeqclasses;
//                return sorted_eqclasses < o.sorted_eqclasses;
            }

        };
        struct GroupedBuildings {
            std::set<std::size_t> buildingids;
            bool worth_mentioning = false;
            std::array<float, 3> examplePosition;
            double total_power = 0;
            // total production statistics here...
        };

        struct EqClassRoughDescription {
            std::set<std::size_t> sources, sinks;

        };


        //std::map<std::size_t,EqClassRoughDescription > relevant_eqclasses;



        auto iterateEqClass = [&](const std::size_t i, const auto &fct) {

            if (!fct(eqclasses[i])) {
                return;
            }
            for (std::size_t j = eqclasses[i]; j != i; j = eqclasses[j]) {
                if (!fct(eqclasses[j])) {
                    return;
                }
            }
        };

        auto getLastIdOfEqClass = [&](std::size_t i) -> std::size_t {
            while (eqclasses[i] > i) {
                i = eqclasses[i];
            }
            return i;
        };

        const auto eqClassHasInputs = [&](const std::size_t eqclassid) -> bool {
            bool gotinputs = false;
            iterateEqClass(eqclassid, [&](const std::size_t woid) {
                const auto &wo = worldObjects[woid];
                bool is_resource = contains("Build_FrackingExtractor", wo.instanceName) || contains("Build_OilPump", wo.instanceName) || contains("Build_MinerMk", wo.instanceName) || contains("OilPump", wo.instanceName) || contains("WaterPump", wo.instanceName);

                if (is_resource || contains("Output0", wo.instanceName) || contains("Output1", wo.instanceName) || contains("Output2", wo.instanceName) || contains("PipeOutputFactory", wo.instanceName)) {
                    gotinputs = true;
                    return false; // stop searching
                }


                return true; // continue searching

            });
            return gotinputs;
        };

        const auto guessEqClassPayload = [&](const std::size_t eqclassi) -> std::map<std::string, std::size_t> {

            std::map<std::string, std::size_t> ret;

            const auto cb = [&](const std::size_t j) {

                const auto &wo = worldObjects[j];

                std::string mmm = std::string(wo.instanceName.v, wo.instanceName.n - 1);
                mmm += ".StorageInventory";
                std::size_t inventoryid = findWorldObject(RepeatedPrimitiveType<char>(mmm.size() + 1, (char *) mmm.c_str()));
                if (inventoryid >= worldObjects.size()) {
                    return true;
                }
                //std::cout << "AHA ! this exists: " << mmm << "   " << worldObjects[inventoryid].worldobjecttype << std::endl;

                if (worldObjects[inventoryid].worldobjecttype != 0) {
                    return true;
                }
//                    const SaveComponent &sc = std::get<SaveComponent>(wo.v);
//                    if (contains("Conveyor", wo.instanceName) || contains("StorageContainer", wo.instanceName) || contains("Pipeline", wo.instanceName) || contains("Valve", wo.instanceName) || contains("PipeStorageTank", wo.instanceName) || contains("IndustrialTank", wo.instanceName)) {


                const SaveComponent &inventorysc = std::get<SaveComponent>(worldObjects[inventoryid].v);
                for (const auto &df : inventorysc.DataFields.value().props) {
                    if (!eq("mInventoryStacks", df->name)) {
                        continue;
                    }
                    const ArrayProperty &inventoryarray = std::get<ArrayProperty>(df->content);
                    //prt(inventoryarray.tname);

                    const ArrayProperty::StructBS &sbs = std::get<ArrayProperty::StructBS>(inventoryarray.content);
                    const std::vector<SerializedFields> &vsfs = std::get<std::vector<SerializedFields>>(sbs.properties);
                    for (const SerializedFields &sfs: vsfs) {
                        for (const std::unique_ptr<PropertyType> &sf: sfs.props) {
                            //std::cerr << sf->header.value().fieldType << std::endl;
                            if (!eq("StructProperty", sf->header.value().fieldType)) {
                                continue;
                            }
                            const StructProperty &sp = std::get<StructProperty>(sf->content);
                            assert(eq("InventoryItem", sp.structtype));
                            const StructInventoryItem &sii = std::get<StructInventoryItem>(sp.structcontent);
                            if (sii.itemtype.size()) {
                                ret[std::string(sii.itemtype.v)]++;
                            }
                        }
                    }
                }


                return true;

            };

            iterateEqClass(eqclassi, cb);


            return ret;

        };

//        for (const std::size_t i : relevant_eqclasses) {
//            guessEqClassPayload(i);
//        }

        std::map<std::size_t, EqClassRoughDescription> relevant_eqclasses;
        std::map<AggregatingId, GroupedBuildings> groups;


        const auto classifyObject = [&](const std::size_t i) -> std::optional<AggregatingId> {

//            assert (isRelevant(i));


            const auto &wo = worldObjects[i];
            if (wo.worldobjecttype != 1 || !contains("/Game/FactoryGame/Buildable/Factory", wo.typePath)) {
                return std::optional<AggregatingId>();
            }
            AggregatingId aid;
            bool is_resource = contains("Build_FrackingExtractor", wo.typePath) || contains("Build_MinerMk", wo.typePath) || contains("WaterPump", wo.typePath) || contains("OilPump", wo.typePath);
            bool is_generator = contains("Build_Generator", wo.typePath);
//            if (contains("WaterPump", wo.typePath)) {
//                aid.recipe = "Water";
//            }else
                if (contains("OilPump", wo.typePath)) {
                aid.recipe = "Recipe_CrudeOil_C";
            }

            const SaveEntity &se = std::get<SaveEntity>(wo.v);
            if (!se.DataFields.has_value()) {
                return std::optional<AggregatingId>();
            }
            if (is_resource || is_generator) {
                aid.recipe = std::string(wo.typePath.v);
                aid.recipe = aid.recipe.substr(aid.recipe.rfind('.') + 1);

            } else {

                for (const auto &d : se.DataFields.value().props) {
                    if (!d || !eq("mCurrentRecipe", d->name)) {
                        continue;
                    }

                    const ObjectReference &obr = std::get<ObjectReference>(d->content);
                    std::string recipe(obr.pathName.v, obr.pathName.n - 1);
                    if (*recipe.rbegin() == '\0') {
                        recipe.pop_back();
                    }
                    aid.recipe = recipe.substr(recipe.rfind('.') + 1);
                }


                if (aid.recipe.empty()) {
                    return std::optional<AggregatingId>();
                }
            }

            std::set<std::string> valid_postfixes = {"Output0", "Input0", "Input1", "Input2", "Input3", "InPut3", "InPut2", "Output1", "Output2", "PipeInputFactory", "PipeInputFactory1", "PipeOutputFactory", "FGPipeConnectionFactory"};
            if (contains("OilPump", wo.typePath)) {
                valid_postfixes = {"FGPipeConnectionFactory"};
            }
            bool isoilrefinery = contains("OilRefinery", wo.typePath);
            for (const std::string postfix : valid_postfixes) {
                const std::string tbs = std::string(wo.instanceName.v) + "." + postfix;
                const std::size_t inventoryid = findWorldObject(RepeatedPrimitiveType<char>(tbs.size() + 1, (char *) tbs.c_str()));
                if (inventoryid > worldObjects.size()) {
                    continue;
                }
                std::size_t normalizedinventoryid = getLastIdOfEqClass(inventoryid);
                if (relevant_eqclasses.find(normalizedinventoryid) == relevant_eqclasses.end()) {
                    continue;
                }
                if (is_resource || postfix == "Output0" || postfix == "Output1" || postfix == "Output2" || postfix == "PipeOutputFactory") {
                    if (!isoilrefinery || eqclasses[inventoryid] != inventoryid) { // me thinks that the unused outputs from oilrefineries create their own eqclasses... confusing everything.
                        aid.outputs_eqclasses[postfix] = normalizedinventoryid;
                    } else {

                    }
                } else if (eqClassHasInputs(inventoryid)) {
                    aid.inputeqclasses.insert(normalizedinventoryid);
                }
            }
            {
                const std::string powerinput = std::string(wo.instanceName.v) + ".PowerInput";
                const auto circuitit = instance_to_circuitid.find(powerinput);
                if (circuitit != instance_to_circuitid.end()) {
                    aid.circuitid = circuitit->second;
                }
            }
            {
                const std::string powerinput = std::string(wo.instanceName.v) + ".PowerConnection";
                const auto circuitit = instance_to_circuitid.find(powerinput);
                if (circuitit != instance_to_circuitid.end()) {
                    aid.circuitid = circuitit->second;
                }
            }


            return std::optional<AggregatingId>(aid);
        };

        std::set<std::pair<std::size_t, std::size_t> > tobemerged;
        do {
            for (const auto &i : tobemerged) {
                mergeEqClasses(eqclasses, i.first, i.second);
            }
            tobemerged.clear();
            relevant_eqclasses.clear();
            groups.clear();

            for (std::size_t i = 0; i < eqclasses.size(); ++i) {
                if (eqclasses[i] >= i) { // do each list only once, therefore only look at their tails.
                    continue;
                }
                bool relevant = false;
                {
                    relevant = relevant || isRelevant(i);
                    std::size_t j = eqclasses[i];
                    for (; j != i && !relevant; j = eqclasses[j]) {
                        relevant = relevant || isRelevant(j);
                    }

                }
                if (relevant) {
                    relevant_eqclasses[i];
                }
            }
            prt(relevant_eqclasses.size());

            for (std::size_t i = 0; i < worldObjects.size(); ++i) {
                auto aid = classifyObject(i);
                if (!aid.has_value()) {
                    continue;
                }
                GroupedBuildings &gb = groups[aid.value()];

                gb.buildingids.insert(i); //examplePosition
                gb.examplePosition = std::get<SaveEntity>(worldObjects[i].v).Position;

                for (const auto &mmm : aid.value().outputs_eqclasses) {
                    relevant_eqclasses[mmm.second].sources.insert(i);
                }
                for (const auto &mmm : aid.value().inputeqclasses) {
                    relevant_eqclasses[mmm].sinks.insert(i);
                }

//                std::string powerinfoname(worldObjects[i].instanceName.v);
//                powerinfoname += ".powerInfo";
//                std::size_t powerindex = findWorldObject(powerinfoname);
//                assert(powerindex < worldObjects.size());
                bool foundCurrentPotential = false;
                for (const std::unique_ptr<PropertyType> &powerproperty : std::get<SaveEntity>(worldObjects[i].v).DataFields.value().props) {
                    if (eq("mCurrentPotential", powerproperty->name)) {  // this is the target percentage, the underclocking/overclocking amount
                        float mCurrentPotential = std::get<float>(powerproperty->content);
                        //prt(mCurrentPotential);
                        gb.total_power += mCurrentPotential;
                        foundCurrentPotential = true;
                        break;
                    }
                }
                if (!foundCurrentPotential) {
                    gb.total_power += 1.0;
                }
            }
            std::set<std::pair<std::size_t, std::size_t> > checked;

            for (const auto &startclass_desc : relevant_eqclasses) {
                // can i by inspecting the sources ... find eqclasses similar enough that they deserve to be merged...
                if (startclass_desc.second.sources.size() < 2) {
                    continue;
                }

                std::vector<AggregatingId> aids;
                for (const std::size_t s : startclass_desc.second.sources) {
                    auto optaid = classifyObject(s);
                    assert(optaid.has_value());
                    aids.push_back(optaid.value());
                }

                for (std::size_t i = 0; i < aids.size(); ++i) {
                    for (std::size_t j = i + 1; j < aids.size(); ++j) {
                        if (aids[i].recipe != aids[j].recipe || aids[i].outputs_eqclasses != aids[j].outputs_eqclasses) {
                            continue;
                        }
                        std::size_t num_difference_i = 0;
                        std::size_t num_difference_j = 0;
                        std::vector<std::size_t> differinginputeqclasses;
                        sortedContainerComparison(
                                aids[i].inputeqclasses,
                                aids[j].inputeqclasses,
                                [&](const auto &onlya) {
                                    num_difference_i++;
                                    differinginputeqclasses.push_back(*onlya);
                                },
                                [&](const auto &onlyb) {
                                    num_difference_j++;
                                    differinginputeqclasses.push_back(*onlyb);
                                },
                                [](const auto &, const auto &) {});
                        if (num_difference_i != 1 || num_difference_j != 1) {
                            continue;
                        }
                        auto key = std::make_pair(differinginputeqclasses[0], differinginputeqclasses[1]);
                        if (key.first > key.second) {
                            std::swap(key.first, key.second);
                        }
                        if (checked.find(key) != checked.end()) {
                            continue;
                        }
                        checked.insert(key);

                        // how different are these 2 eqclasses?
                        auto iti = relevant_eqclasses.find(differinginputeqclasses[0]);
                        auto itj = relevant_eqclasses.find(differinginputeqclasses[1]);
                        assert(iti != relevant_eqclasses.end() && itj != relevant_eqclasses.end());
                        std::set<AggregatingId> sinkaids_i, sinkaids_j;
                        for (auto &k : iti->second.sinks) {
                            const auto &optaid = classifyObject(k);
                            assert(optaid.has_value());
                            sinkaids_i.insert(optaid.value());
                        }
                        for (auto &k : itj->second.sinks) {
                            const auto &optaid = classifyObject(k);
                            assert(optaid.has_value());
                            sinkaids_j.insert(optaid.value());
                        }
                        std::cerr << "judging eqclasses " << differinginputeqclasses[0] << " and " << differinginputeqclasses[1] << ". sinkaidsizes: " << sinkaids_i.size() << " " << sinkaids_j.size() << std::endl;
                        if (sinkaids_i.size() != 1 || sinkaids_j.size() != 1) {
                            continue;
                        }
                        tobemerged.insert(std::make_pair(differinginputeqclasses[0], differinginputeqclasses[1]));

                        std::cerr << "to be merged, detected by starting from sink " << startclass_desc.first << ": " << differinginputeqclasses[0] << " and " << differinginputeqclasses[1] << " recipe " << aids[0].recipe << std::endl;
                    }
                }
            }


        } while (!tobemerged.empty());


        const auto printRelevantObject = [&](std::size_t id, std::size_t i) {

            assert (isRelevant(i));


            const auto &wo = worldObjects[i];
            if (wo.worldobjecttype != 0) {
                return;
            }

            std::cerr << "  " << id << "   " << wo.instanceName;
            assert(wo.worldobjecttype == 0);
            const SaveComponent &sc = std::get<SaveComponent>(wo.v);
            const std::size_t parentid = findWorldObject(sc.ParentEntityName);
            assert(parentid < worldObjects.size());
            const auto &parent = worldObjects[parentid];
            assert(parent.worldobjecttype == 1);
            const SaveEntity &scparent = std::get<SaveEntity>(parent.v);
            if (scparent.DataFields.has_value()) {
                for (const auto &d : scparent.DataFields.value().props) {
                    if (!d || !eq("mCurrentRecipe", d->name)) {
                        continue;
                    }
                    const ObjectReference &obr = std::get<ObjectReference>(d->content);
                    std::string recipe(obr.pathName.v, obr.pathName.n - 1);
                    if (*recipe.rbegin() == '\0') {
                        recipe.pop_back();
                    }
                    recipe = recipe.substr(recipe.rfind('.') + 1);
                    std::cerr << " " << recipe;

                }
            }
            std::cerr << " " << scparent.Position[0] << " " << scparent.Position[1] << " " << scparent.Position[2];

            std::cerr << std::endl;
//            std::cout.flush();
        };


        for (const auto &i_desc : relevant_eqclasses) {
            const std::size_t i = i_desc.first;
            std::cout.flush();
            std::cerr << "-----------------------------------" << std::endl;

            iterateEqClass(i, [&](const std::size_t j) -> bool {
                if (isRelevant(j)) {
                    printRelevantObject(i, j);
                }
                return true;
            });
//            for (std::size_t j = i;; j = eqclasses[j]) {
//                if (isRelevant(j)) {
//                    printRelevantObject(i, j);
//                }
//
//                if (eqclasses[j] < j) {
//                    break;
//                }
//            }

            for (const auto &s : guessEqClassPayload(i)) {
                std::cerr << s.second << " X " << s.first << std::endl;
            }


        }

//        auto& oss = std::cerr;
        std::ostringstream oss;
        {
            oss << "digraph 123 { node [shape=box,fontname=\"Bitstream Vera Sans Mono\", penwidth=5];  \n";
            std::size_t counter = 0;
            std::map<std::size_t, std::size_t> encountered_eqclasses; // + count

            for (auto &aid_group : groups) {
//                if (aid_group.second.buildingids.size() < 2){
//                    continue;
//                }

                int mentions = 0;
                for (const auto &cxn_eqclassid : aid_group.first.outputs_eqclasses) {
                    if (eqclasses[cxn_eqclassid.second] == cxn_eqclassid.second) {
                        continue;
                    }
                    encountered_eqclasses[cxn_eqclassid.second]++;
                    mentions++;
                }

                for (const auto &eqclassid : aid_group.first.inputeqclasses) {
                    if (eqclasses[eqclassid] == eqclassid) {
                        continue;
                    }
                    encountered_eqclasses[eqclassid]++;
                    mentions++;
                }

                aid_group.second.worth_mentioning = (mentions > 1 || aid_group.second.buildingids.size() > 1);

                if (aid_group.second.worth_mentioning) {
                    continue;
                }
                for (auto &i : aid_group.second.buildingids) {
                    const auto &wo2 = worldObjects[i];
                    if (contains("Build_FrackingExtractor", wo2.typePath) || contains("Build_MinerMk", wo2.typePath) || contains("OilPump", wo2.typePath) || contains("WaterPump", wo2.typePath)) {
                        aid_group.second.worth_mentioning = true;
                        continue;
                    }
                }


            }
            std::size_t MAX_EQCLASS_USE = 2000;

            for (const auto &aid_group : groups) {
                if (!aid_group.second.worth_mentioning) {
                    continue;
                }
//                if (aid_group.second.buildingids.size() < 2){
//                    continue;
//                }
//                if (false && counter++ > 4000) {
//                    break;
//                }
                // todo: make a nice "record"
                // bool worth_mentioning = false;
                for (const auto &cxn_eqclassid : aid_group.first.outputs_eqclasses) {
                    if (eqclasses[cxn_eqclassid.second] == cxn_eqclassid.second) {
                        continue;
                    }
                    if (encountered_eqclasses[cxn_eqclassid.second] > 1 && encountered_eqclasses[cxn_eqclassid.second] < MAX_EQCLASS_USE) {
//                    oss << cxn_eqclassid.second << " [shape=none,label=\"" << cxn_eqclassid.second  << "\"];\n " << std::endl;
                        oss << ((std::size_t) &aid_group) << " -> " << cxn_eqclassid.second << ";\n" << std::endl;
                        // worth_mentioning = true;
                    }

                }

                for (const auto &eqclassid : aid_group.first.inputeqclasses) {
                    if (eqclasses[eqclassid] == eqclassid) {
                        continue;
                    }
                    if (encountered_eqclasses[eqclassid] > 1 && encountered_eqclasses[eqclassid] < MAX_EQCLASS_USE) {
//                    oss << eqclassid << " [shape=none,label=\"" << eqclassid  << "\"];\n " << std::endl;
                        oss << eqclassid << " -> " << ((std::size_t) &aid_group) << ";\n" << std::endl;
                        //worth_mentioning = true;
                    }
                }
//                const auto& examplebuilding = worldObjects[*aid_group.second.buildingids.begin()];


//                Color color = colorify(aid_group.first.circuitid);

                double colorseed1 = double((aid_group.first.circuitid * 777) %  circuitid_to_index.size())  / circuitid_to_index.size();
                double colorseed2 = double((aid_group.first.circuitid * 7777) %  circuitid_to_index.size())  / circuitid_to_index.size();
                double colorseed3 = double((aid_group.first.circuitid * 77777) %  circuitid_to_index.size())  / circuitid_to_index.size();
                oss << ((std::size_t) &aid_group) << " [color=\"" << colorseed1 << " " << (0.3 + 0.4 * colorseed2) << " " << (0.8 + 0.2 * colorseed3) << "\", label=\"";
                if (aid_group.first.recipe.starts_with("Recipe_")) {

                    const World::Recipe &r = w.recipes[w.getRecipeNC(aid_group.first.recipe)];
                    oss << aid_group.second.buildingids.size() << " X " << r.mDisplayName << " (" << (aid_group.second.total_power * 100.0) << "%)" << "\\l" << "circuit " << aid_group.first.circuitid << "\\l";
                    std::cerr << "{w.getRecipeNC(\"" << r.mDisplayName << "\")," << aid_group.second.total_power << "}," << std::endl;


                    for (const auto i : r.input) {
                        double base = w.resources[i.first].isliquid ? i.second / 1000.0 : i.second;
                        double perminute = 60.0 / r.mManufactoringDuration;
                        double intotal = aid_group.second.total_power;
                        oss << string_format("- %3d %6.2f %8.2f %s\\l", (int) base, (base * perminute), (base * perminute * intotal), w.resources[i.first].mDisplayName.c_str());

//                        oss << i.second << " " << (i.second * 60.0 / r.mManufactoringDuration) << " " << (i.second * 60.0 / r.mManufactoringDuration * double(aid_group.second.buildingids.size())  ) << " " << w.resources[i.first].mDisplayName << "\\l";
                    }
                    for (const auto i : r.output) {
                        double base = w.resources[i.first].isliquid ? i.second / 1000.0 : i.second;
                        double perminute = 60.0 / r.mManufactoringDuration;
                        double intotal = aid_group.second.total_power;
                        oss << string_format("+ %3d %6.2f %8.2f %s\\l", (int) base, (base * perminute), (base * perminute * intotal), w.resources[i.first].mDisplayName.c_str());
                    }
                } else {
                    oss << aid_group.second.buildingids.size() << " X " << aid_group.first.recipe << "\\l" << "circuit " << aid_group.first.circuitid << "\\l" ;
                }
//                oss << "\\n(" << aid_group.second.buildingids.size() << ", " << aid_group.first.inputeqclasses.size() << ", " << aid_group.first.outputs_eqclasses.size() << ")\\n";
                oss << "\\l";
                oss << "Pos: " << aid_group.second.examplePosition[0] << " " << aid_group.second.examplePosition[1] << " " << aid_group.second.examplePosition[2] << "\\l";
                oss << "Inputs:";
                for (auto &i : aid_group.first.inputeqclasses) {
                    oss << i << " ";
                }
                oss << "\\l";
                for (auto &i : aid_group.first.outputs_eqclasses) {
                    oss << i.first << "->" << i.second << "\\l";
                }
                oss << "\" ];" << std::endl;
            }

            for (const auto &eqclassid : encountered_eqclasses) {
                if (eqclassid.second > 1 && eqclassid.second < MAX_EQCLASS_USE) {
                    oss << eqclassid.first << " [shape=none,label=\"" << eqclassid.first;
                    for (const auto &payload_amount  : guessEqClassPayload(eqclassid.first)) {
                        const std::string resourceclass = payload_amount.first.substr(payload_amount.first.rfind('.') + 1);
                        oss << "\\l" << w.getResourceDisplayName(resourceclass);
                    }
                    oss << "\\l\"];\n " << std::endl;
                }
            }

            oss << "}\n";
        }


        std::cerr << "writing topology.dot" << std::endl;
        writeToFile("/tmp/topology.dot", oss.str().c_str());
        prt(groups.size());

    }
};

struct SatisfactorySav {
    int32_t headerVersion;
    int32_t saveVersion;
    int32_t buildVersion;
    RepeatedPrimitiveType<char> worldType; // MapName
    RepeatedPrimitiveType<char> worldProperties; // MapOptions
    RepeatedPrimitiveType<char> sessionName;
    int32_t playTimeSeconds;
    int64_t saveDateMsTicks;
    bool sessionVisibility;
    int32_t editorObjectVersion;
    RepeatedPrimitiveType<char> modMetaData;
    int32_t isModdedSave;

//    RepeatedType<CompressedBlock> chunks;
//    CompressedBlock chunk;
    std::vector<CompressedBlock> chunks;
    std::vector<char> uncompressedData;

    std::optional<SatisfactorySavPayload> payload;

    SatisfactorySav(Builder &b) :
            headerVersion(b.inc<int32_t>()),
            saveVersion(b.inc<int32_t>()),
            buildVersion(b.readBuildVersion()),
            worldType(b),
            worldProperties(b),
            sessionName(b),
            playTimeSeconds(b.inc<int32_t>()),
            saveDateMsTicks(b.inc<int64_t>()),
            sessionVisibility(b.inc<bool>()),
            editorObjectVersion(b.inc<int32_t>()),
            modMetaData(b),
            isModdedSave(b.inc<int32_t>())
//            , worldObjects(ptr, offset)
    {
        std::size_t totalsize_uncompressed = 0;
        while (b.offset < b.maxoffset) {
            totalsize_uncompressed += chunks.emplace_back(b).currentChunkUncompressedLength;
        }
        prt2(chunks.size(), totalsize_uncompressed);
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
        Builder b2{uncompressedData.data(), 0, uncompressedData.size(), b.buildVersion};

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

template<typename W>
void printJson(W &w, const SerializedFields &t) noexcept {
    w.StartArray();
    //prt(t.props.size());
    for (const auto &c : t.props) {
        printJson(w, *c);
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
void printJson(W &w, const MoreInventoryStuff &t) noexcept {
    w.StartObject();
    JSONFIELD(length);
    JSONFIELD(name);
    JSONFIELD(levelName);
    JSONFIELD(pathName);
    JSONFIELD(position);
    w.EndObject();
}


template<typename W>
void printJson(W &w, const WorldObject &t) noexcept {
    w.StartObject();
    if (contains("FGFoliageRemoval", t.typePath) || contains("Benefit", t.typePath) || contains("Spawnable", t.typePath) || contains("reatureSpawn", t.typePath)) {
        w.EndObject();
        return;
    }

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

//    JSONFIELD(serializedFields);

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
    JSONFIELD(DataFields);
    JSONFIELD(extra_count);
    JSONFIELD(extra);
//    w.EndObject();
}

template<typename W>
void printJson(W &w, const SaveComponent &t) noexcept {
//    w.StartObject();
    JSONFIELD(ParentEntityName);
    JSONFIELD(DataFields);
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
        } else if (eq("Int8Property", ft)) {
            printJson(w, std::get<int8_t>(t.content));
        } else {
            prt(ft);
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
//    } else if (eq(StructTrainSimulationData::tname, t.structtype)) {
//        printJson(w, std::get<StructTrainSimulationData>(t.structcontent));
    } else if (eq(StructTransform::tname, t.structtype)) {
        printJson(w, std::get<StructTransform>(t.structcontent));
    } else if (eq(StructVector::tname, t.structtype)) {
        printJson(w, std::get<StructVector>(t.structcontent));
    } else {
        printJson(w, std::get<SerializedFields>(t.structcontent));
        //assert(false);
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
    } else if (eq("EnumProperty", t.itemType)) {
        printJson(w, std::get<RepeatedType<RepeatedPrimitiveType<char>>>(t.content));

    } else {
        prt(t.itemType);
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
    }else     if (eq(StructVector::tname, t.structInnerType)) {
        printJson(w, std::get<std::vector<StructVector> >(t.properties));
    } else { // if (eq("RemovedInstance", structInnerType)) {
        printJson(w, std::get<std::vector<SerializedFields> >(t.properties));
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
    JSONFIELD(key);
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

//template<typename W>
//void printJson(W &w, const StructTrainSimulationData &t) noexcept {
//    assert(false);
//}

#undef JSONFIELD


int main() {


    World world;
    world.loadFromCommunityResourcesDocsJson("/home/donf/fsrc/satisfactory/satisfactorysimulator/DocsV0.5.0.11.json");

    //    MemoryMappedFile mmf("/home/donf/fsrc/satisfactory/satisfactory-sav-analyser/mmm2_2004-181613.sav");
//    MemoryMappedFile mmf("/home/donf/fsrc/satisfactory/satisfactory-sav-analyser/mmm2_autosave_0.sav");
//    MemoryMappedFile mmf("/home/donf/fsrc/satisfactory/satisfactory-sav-analyser/mmm2_111121-204114.sav");
//    MemoryMappedFile mmf("/home/donf/fsrc/satisfactory/satisfactory-sav-analyser/mmm2_171121-175958.sav"); mmm2_181121-132253.sav
//    MemoryMappedFile mmf("/home/donf/fsrc/satisfactory/satisfactory-sav-analyser/mmm2_181121-132253.sav");
//    MemoryMappedFile mmf("/home/donf/fsrc/satisfactory/satisfactory-sav-analyser/mmm2_281121-205216.sav");


    MemoryMappedFile mmf("/home/donf/fsrc/satisfactory/satisfactory-sav-analyser/therapy_autosave_0.sav");
    mmf.initialise();
    prt(mmf.size);
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
//    writeToFile("/tmp/satisfactory_save2.json", s.GetString());

    std::cerr << "building component-graph.";
    sav.payload->discoverComponentRelations();
    std::cerr << "component-graph written." << std::endl;


    sav.payload->studyTopology(world);


    // consider ... a boolean set operations-tool ...
    // given 2 files create ... intersection, union, difference
    // also specify how to recognize identical elements: same name, same location, same rotation, same type ...
    //  also specify on what to do with the names of the new file: for the intersection: the names of which side should be kept? should one side be completely renamed?   is one dominant?

    // or ... we add a new objecttype ... one that gives a named reference to the inputs/outputs/stuff from the "blueprint" factory
    // so ... when we do "templates" ... then we must be able to rename everything ...


    std::cerr << "neato -Noverlap=false -Eoverlap=false -Goverlap=false -LT103 /tmp/satisfactory_sav_structure.dot  -Tpng > /tmp/zucht.png  && feh /tmp/zucht.png" << std::endl;
    return 0;
}
