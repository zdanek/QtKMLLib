#include "kmlrenderer.h"
#include "kmldocument.h"
#include "qtkml.h"
#include "qmlkml.h"
#include "kmlelement.h"

#include <QDebug>
#include <iostream>

//const StyleParams& KmlQmlRendererPrivate::style(QString id) const {
//    return m_styles.get(id);
//}

using namespace QtKml;

KmlQmlRendererPrivate::KmlQmlRendererPrivate(QObject* owner,
                                             const QSharedPointer<KmlDocumentPrivate>& doc/*,
                                             KmlDocument* parent*/):
    m_doc(doc)
{
    std::cout << "KmlQmlRendererPrivate::KmlQmlRendererPrivate() " << this << std::endl;
    m_list.clear();
    m_graphics.clear();
    // bzd in my version this was commented out
    doc->getPolygons(m_graphics, m_styles);
    m_centerPoint = doc->centerPoint();
    for (const auto &g : m_graphics) {
//        std::cout << " Style id " << g->styleId().toStdString() << std::endl;
//        std::cout << " owner element " << owner << std::endl;

        auto style_params = resolveStyleParams(doc, g);
        auto p = new KmlQmlElementPrivate(style_params, g);
        m_list.append(new KmlQmlElement(owner, p));
    }
}

KmlQmlRendererPrivate::~KmlQmlRendererPrivate(){
    qDebug() << "~KmlQmlRendererPrivate" << this;
  //  for(auto g: m_graphics)
  //      delete g;
}

int KmlQmlRendererPrivate::polygonCount() const {
    return m_graphics.count();
}

const Graphics* KmlQmlRendererPrivate::graphics(int index) const{
    if(index < polygonCount()){
        return (m_graphics.at(index).get());
    }
    return NULL;
}

KmlQmlRenderer::KmlQmlRenderer(const QSharedPointer<KmlDocumentPrivate>& doc, KmlDocument* parent) : QObject(parent),
    d_ptr(new KmlQmlRendererPrivate(this, doc/*, parent*/)){
    std::cout << "KmlQmlRenderer::KmlQmlRenderer() " << this << std::endl;
}

KmlQmlRenderer::~KmlQmlRenderer(){
    qDebug() << "~KmlQmlRenderer" << this;
}

QGeoRectangle KmlQmlRenderer::bounds() const{
    Q_D(const KmlQmlRenderer);
    return d->bounds();
}

QGeoCoordinate KmlQmlRenderer::center() const{
    Q_D(const KmlQmlRenderer);
    return QGeoCoordinate(d->centerPoint().lat(), d->centerPoint().lon());
}

QString KmlQmlRenderer::identifier() const{
    Q_D(const KmlQmlRenderer);
    return d->doc()->identifier();
}

QQmlListProperty<QObject> KmlQmlRenderer::elements(){
    // bzd in my version this was commented out
    const QQmlListProperty<KmlQmlElement>::CountFunction cf = [](QQmlListProperty<KmlQmlElement>* r)->int{
        return qobject_cast<KmlQmlRenderer*>(r->object)->d_ptr->elements().count();
    };
    // bzd in my version this was commented out
    const QQmlListProperty<KmlQmlElement>::AtFunction af = [](QQmlListProperty<KmlQmlElement>* r, int index)->KmlQmlElement*{
        return qobject_cast<KmlQmlRenderer*>(r->object)->d_ptr->elements().at(index);
    };

    return QQmlListProperty<QObject>(this, nullptr,
                                           reinterpret_cast<QQmlListProperty<QObject>::CountFunction>(cf),
                                           reinterpret_cast<QQmlListProperty<QObject>::AtFunction>(af));
}
// bzd in my version below was block commented out
void KmlQmlRenderer::setStyles(const QString& name, const QVariantMap& style){
    Q_D(KmlQmlRenderer);
    // bzd in my version this was commented out
    d->doc()->setStyles(name, style);
    emit documentChanged();
}

QVariantMap KmlQmlRenderer::styles(const QString& name) const{
    Q_D(const KmlQmlRenderer);
    // bzd in my version this was commented out
    return d->doc()->styles(name);
}

QStringList KmlQmlRenderer::styleNames() const{
    Q_D(const KmlQmlRenderer);
    return d->doc()->styleNames();
}

StyleParams KmlQmlRendererPrivate::resolveStyleParams(const QSharedPointer<KmlDocumentPrivate> &doc, const std::shared_ptr<Graphics> &g)
{
    auto style_id = g->styleId();
    if (style_id == DEFAULT_STYLE_ID) {
        return StyleVisitor::defaultStyleParams(g->domObject());
    }

    return doc->customStyle(m_styles[g->styleId()], g->styleId());
}

