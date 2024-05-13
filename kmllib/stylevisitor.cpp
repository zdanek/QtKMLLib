#include "stylevisitor.h"

#ifdef ALLOW_QDEBUG_PRINT
#include <QDebug>
#endif

using namespace QtKml;


StyleParams &StyleParams::from(const kmldom::StylePtr &style)
{
    if (style->has_linestyle()) {
        if (style->get_linestyle()->has_color()) {
            setLineColor(style->get_linestyle()->get_color().get_color_argb());
        }
        if (style->get_linestyle()->has_width()) {
            setLineWidth(style->get_linestyle()->get_width());
        }
    }
    if (style->has_polystyle()) {
        if (style->get_polystyle()->has_color()) {
            setFillColor(style->get_polystyle()->get_color().get_color_argb());
        }
        if (style->get_polystyle()->has_fill()) {
            setIsFill(style->get_polystyle()->get_fill());
        }
    }
    if (style->has_iconstyle()) {
        if (style->get_iconstyle()->has_icon()) {
            setIcon(QString::fromStdString(style->get_iconstyle()->get_icon()->get_href()));
        }
    }
    return *this;
}

StyleVisitor::StyleVisitor(StyleList& styles) : m_styles(&styles){
}

StyleVisitor::~StyleVisitor(){
}

/*
 * const StyleParams& StyleVisitor::style() const {return m_styles.find(m_currentStyle).value();}
QString StyleVisitor::currentStyleId() const {
    return m_currentStyle;
}


void StyleVisitor::VisitFeature(const kmldom::FeaturePtr &element){
    if(element->has_styleurl()){
         const QString label = QString::fromStdString(element->get_styleurl());
         m_currentStyle = label.at(0) == '#' ? label.mid(1) : label;
    }
}
*/

QString StyleVisitor::currentStyleId(const kmldom::ElementPtr &element) const
{
    if (element == nullptr)
        return QString("");
    if (element->IsA(kmldom::Type_Feature)) {
        const kmldom::FeaturePtr feature = kmldom::AsFeature(element);
        if (feature->has_styleurl()) {
            const QString label = QString::fromStdString(feature->get_styleurl());
            const QString id = label.at(0) == '#' ? label.mid(1) : label;
#ifdef ALLOW_QDEBUG_PRINT
            qDebug() << "ID is" << id.length() << id << "!!";
#endif
            return id;
        }
        if (feature->has_styleselector()) {
            return DEFAULT_STYLE_ID;
        }
    }
    return currentStyleId(element->GetParent());
}

const StyleParams StyleVisitor::defaultStyleParams(const kmldom::ElementPtr &element)
{
    if (element == nullptr) {
        return StyleParams();
    }
    if (element->IsA(kmldom::Type_Feature)) {
        const kmldom::FeaturePtr feature = kmldom::AsFeature(element);
        if (feature->has_styleselector()) {
            auto style_selector = feature->get_styleselector();
            if (style_selector->IsA(kmldom::Type_Style)) {
                const kmldom::StylePtr style = kmldom::AsStyle(style_selector);
                if (style != nullptr) {
                    StyleParams params;
                    params.from(style);
                    return params;
                }
            }
        }
    }

    return defaultStyleParams(element->GetParent());
}

void StyleVisitor::VisitStyle(const kmldom::StylePtr &element){
    QString styleId;
    if(element->has_id()) {
        styleId = QString::fromStdString(element->get_id());
        // } else {
        //     styleId = DEFAULT_STYLE_ID;
        // }
        StyleParams style;

        if(element->has_linestyle()){
            kmldom::LineStylePtr lp = element->get_linestyle();
            if(lp->has_color()){
                style.setLineColor (lp->get_color().get_color_argb());
            }
            if(lp->has_width()){
                style.setLineWidth (lp->get_width());
            }
        }
        if(element->has_polystyle()){
            kmldom::PolyStylePtr ps = element->get_polystyle();
            if(ps->has_color()){
                style.setFillColor (ps->get_color().get_color_argb());
            }
            // if(ps->has_fill()){ has_fill is FALSE even it shall not be, bug in kmllib
            style.setIsFill(ps->get_fill());
            //  }
        }
        if(element->has_iconstyle()){
            kmldom::IconStylePtr is = element->get_iconstyle();
            if(is->has_icon()){
                style.setIcon (QString::fromStdString(is->get_icon()->get_href()));
            }
        }
        m_styles->insert(styleId, style);
        //   qDebug() << styleId << "fill:" << style.fill;
    }
}
