#include <QtQml>
#include <QPainter>

#include "kmlgraphics.h"
#include "kmlrenderer.h"
#include "mercatorprojection.h"
#include "qmlkml.h"
#include "qtkml.h"
#include <iostream>

using namespace QtKml;

KmlQmlGraphicsPrivate::~KmlQmlGraphicsPrivate(){
    for(const auto& r : qAsConst(m_renderers)){
        // bzd in my version this was commented out
        r->d_ptr->freeMonitor(this);
    }
}

void KmlQmlGraphicsPrivate::documentDeleted(const QString& id){
    Q_Q(KmlQmlGraphics);
    const int count = m_renderers.remove(id);
    if(count >= 0){
        emit q->documentRemoved(id);
        }
    }


KmlQmlRenderer* KmlQmlGraphicsPrivate::at(int index) const{
    auto k = m_renderers.keyBegin();
    std::advance(k, index);
    return m_renderers[*k];
}

int KmlQmlGraphicsPrivate::append(const QString& id, KmlQmlRenderer* renderer){
    Q_ASSERT(renderer->parent());
    // log
    std::cout << "KmlQmlGraphicsPrivate::append " << id.toStdString() << " self " << this << " renderer " << renderer << std::endl;
    m_renderers.insert(id, renderer);
    return m_renderers.count();
}

int KmlQmlGraphicsPrivate::remove(const QString& id){
    return m_renderers.remove(id);
}

void KmlQmlGraphicsPrivate::map(std::function<void (const KmlQmlRenderer &)> mapper) const{
    for(auto p = m_renderers.constBegin(); p != m_renderers.constEnd(); p++)
       mapper(*(*p));
}


inline QGeoCoordinate operator +(const QGeoCoordinate& a, const QGeoCoordinate& b){
    return QGeoCoordinate(a.latitude() + b.latitude(), a.longitude() + b.longitude());
}


void KmlQmlGraphicsPrivate::renderAll(QPainter& painter,  const QRect& rect, qreal zoom, const QPointF& centerPoint, std::function<bool (const QString& id)> filter){

    if(!painter.isActive()){
        qWarning() << "Painter failed";
        return;
    }
    painter.setBackgroundMode(Qt::TransparentMode);
  //  painter.fillRect(rect, Qt::transparent);
    for(const auto& r : qAsConst(m_renderers) ){
        if(filter == nullptr || filter(r->identifier())){
            // bzd in my version this was commented out
            r->d_ptr->doc()->renderAll(painter, rect, zoom, centerPoint);
        }
    }
}

void KmlQmlGraphicsPrivate::renderAll(const QSize& size, qreal zoom, const QPointF& centerPoint, std::function<bool (const QString& id)> filter){
    if(size != m_image.size()){
        m_image = QPixmap(size);
    }

    m_image.fill(Qt::transparent);

    QPainter painter(&m_image);

    renderAll(painter, QRect({0, 0}, size), zoom, centerPoint, filter);
}

KmlQmlGraphics::KmlQmlGraphics(QObject* parent) : QObject(parent), d_ptr(new KmlQmlGraphicsPrivate(this)){
    std::cout << "KmlQmlGraphics::KmlQmlGraphics() " << this << std::endl;
    qmlRegisterUncreatableType<KmlQmlRenderer>("QtKML", 1, 0, "KmlRenderer", "QtKML");
    qmlRegisterUncreatableType<KmlQmlElement>("QtKML", 1, 0, "KmlElement", "QtKML");
    qmlRegisterType<KmlItem>("QtKML", 1, 0, "KmlItem");
    qRegisterMetaType<QGeoCoordinate>();
    qRegisterMetaType<QGeoRectangle>();
}

bool KmlQmlGraphics::append(KmlDocument* document, const QString& identifier){
    Q_D(KmlQmlGraphics);
    if(document == nullptr)
        return false; //null is not ok
    if(!identifier.isEmpty()){ //id can be empty
        if(!document->d_ptr->identifier().isEmpty()) {

            return false; //but then there shall not be id
        }
        // bzd in my version this was commented out
        document->d_ptr->setIdentifier(identifier); //set id
    } else {
        std::cout << "KmlQmlGraphics::append: id is empty ";
    }

    if(!document->parent()){
        document->setParent(this);
    }
    std::cout << "KmlQmlGraphics::append " << identifier.toStdString() << " self " << this << " document " << document << std::endl;
    auto renderer = new KmlQmlRenderer(document->d_ptr, document);
    QObject::connect(document, &KmlDocument::imageChanged, this, &KmlQmlGraphics::renderersChanged);
    QObject::connect(renderer, &KmlQmlRenderer::documentChanged, this, &KmlQmlGraphics::renderersChanged);
    remove(nullptr, identifier);
    int count = d->append(identifier, renderer);
    std::cout << "KmlQmlGraphics::append count " << count << std::endl;
    // bzd in my version this was commented out
    document->d_ptr->setMonitor(d);
    emit documentAdded(identifier);
    emit renderersChanged();
    return true;
}

void KmlQmlGraphics::remove(KmlDocument* document, const QString& identifier){
    Q_D(KmlQmlGraphics);
    int changes = 0;
    if(document != nullptr)
        changes = d->remove(document->d_ptr->identifier());
    if(!identifier.isEmpty())
        changes = d->remove(identifier);
    if(changes > 0){
        emit documentRemoved(identifier);
        emit renderersChanged();
    }
}

KmlDocument* KmlQmlGraphics::document(const QString& id) const{
    Q_D(const KmlQmlGraphics);
    if(d->contains(id)){
        return qobject_cast<KmlDocument*>(d->get(id)->parent());
    }
    return nullptr;
}

QStringList KmlQmlGraphics::documents() const{
    Q_D(const KmlQmlGraphics);
    return d->ids();
}

QQmlListProperty<QObject> KmlQmlGraphics::renderers(){
    // bzd in my version this was commented out ===  static_cast<KmlQmlGraphics*>(r->object)
    const QQmlListProperty<KmlQmlRenderer>::CountFunction cf = [](QQmlListProperty<KmlQmlRenderer>* r)->int{
        std::cout << "KmlQmlGraphics::renderers count " << std::endl;
        return qobject_cast<KmlQmlGraphics*>(r->object)->d_ptr->count();
    };
    // bzd in my version this was commented out
    const QQmlListProperty<KmlQmlRenderer>::AtFunction af = [](QQmlListProperty<KmlQmlRenderer>* r, int index)->KmlQmlRenderer*{
        return qobject_cast<KmlQmlGraphics*>(r->object)->d_ptr->at(index);
    };

    return QQmlListProperty<QObject>(this, this, (QQmlListProperty<QObject>::CountFunction) cf, (QQmlListProperty<QObject>::AtFunction) af);
}

QGeoRectangle KmlQmlGraphics::bounds() const{
    Q_D(const KmlQmlGraphics);
    if(d->count() == 0)
        return QGeoRectangle();
    QGeoRectangle rect = d->at(0)->bounds();
    d->map([&rect](const KmlQmlRenderer& renderer) mutable{
        rect = rect.united(renderer.bounds());
    });
    return rect;
}

KmlQmlGraphics::~KmlQmlGraphics(){
}


KmlQmlImage KmlQmlGraphics::qmlImage(){
    Q_D(KmlQmlGraphics);
    return KmlQmlImage(d);
}


QVariant KmlQmlGraphics::renderer(const QString& id) const {
    Q_D(const KmlQmlGraphics);
    return QVariant::fromValue<KmlQmlRenderer*>(d->get(id));
}

