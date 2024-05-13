

#ifndef KML_DOM_EXTSYMBOLS_H__
#define KML_DOM_EXTSYMBOLS_H__

#include "element.h"
#include "feature.h"

#include <kml/base/vec3.h>

namespace kmldom {

class ExtStandard : public Element
{
public:
    ExtStandard();
    virtual bool IsA(KmlDomType type) const { return type == Type_ExtStandard; }

    bool SetString(string *) override;

    string getStandard() const;
};

class ExtCode : public Element
{
public:
    ExtCode();
    virtual bool IsA(KmlDomType type) const { return type == Type_ExtCode; }

    bool SetString(string *) override;

    string getCode();
};

class ExtCoordinates : public Element
{
public:
    ExtCoordinates();
    virtual bool IsA(KmlDomType type) const { return type == Type_ExtCoordinates; }

    // The char_data is everything between <coordinates> elements including
    // leading and trailing whitespace.
    void Parse(const string& char_data);
    // Coordinates essentially parses itself.
    void AddElement(const ElementPtr &element);
    kmlbase::Vec3 coordinates() const { return coordinates_; }

private:
    kmlbase::Vec3 coordinates_;

    LIBKML_DISALLOW_EVIL_CONSTRUCTORS(ExtCoordinates);
};

class ExtSymbolInfo : public Feature
{
public:
    ExtSymbolInfo();
    virtual KmlDomType Type() const { return Type_ExtSymbolInfo; }
    virtual bool IsA(KmlDomType type) const {
        return type == Type_ExtSymbolInfo || Feature::IsA(type);
    }

    void AddElement(const ElementPtr &element) override;
    virtual void Accept(Visitor* visitor);

    const string& get_standard() const { return standard_; }
    bool has_standard() const { return has_standard_; }

    const string& get_code() const { return code_; }
    bool has_code() const { return has_code_; }

    bool has_coordinates() const { return has_coordinates_; }
    const kmlbase::Vec3 get_coordinates() const { return coordinates_; }
    void set_coordinates(const ExtCoordinatesPtr& coordinates) {
      has_coordinates_ = coordinates != NULL;
        if (has_coordinates_) {
            coordinates_ = coordinates->coordinates();
        }
    }



private:
    string standard_;
    bool has_standard_;
    string code_;
    bool has_code_;
    kmlbase::Vec3 coordinates_;
    bool has_coordinates_;
};

} // namespace kmldom

#endif // KML_DOM_EXTSYMBOLS_H__