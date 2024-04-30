#include "graphicsvisitor.h"
#include <QPointF>

#include <iostream>

#ifdef ALLOW_QDEBUG_PRINT
#include <QDebug>
#endif

using namespace QtKml;

Graphics::~Graphics(){
#ifdef ALLOW_QDEBUG_PRINT
    qDebug() << "Graphics deleted";
#endif
}

GraphicsVisitor::GraphicsVisitor(Graphics::GraphicsList& graphics, StyleVisitor::StyleList &styles) :
    StyleVisitor(styles),
       m_graphics(graphics){}

void GraphicsVisitor::addVertices(const kmldom::CoordinatesGeometryCommon* const element, Graphics* g) const{
    if(element->has_coordinates()){
        const kmldom::CoordinatesPtr coords = element->get_coordinates();
        for(size_t i = 0; i < coords->get_coordinates_array_size(); i++){
            const kmlbase::Vec3 vec = coords->get_coordinates_array_at(i);
            g->m_vertices.append(QGeoCoordinate(vec.get_latitude(), vec.get_longitude()));
        }
    }
}

void GraphicsVisitor::visitInternal(const kmldom::LinearRingPtr& element){
    Graphics::GraphicsPtr g(new Graphics(currentStyleId(element), Graphics::PolyLine, element));
    m_graphics.append(g);
    addVertices(element.get(), g.get());
    if(g->m_vertices.length() > 1){
       g->m_vertices.append(g->m_vertices.at(0));
       g->m_vertices.append(g->m_vertices.at(1));
   }
}

void GraphicsVisitor::visitInternal(const kmldom::LineStringPtr& element){
    Graphics::GraphicsPtr g(new Graphics(currentStyleId(element), Graphics::PolyLine, element));
    m_graphics.append(g);
    addVertices(element.get(), g.get());
}

void GraphicsVisitor::visitInternal(const kmldom::PointPtr& element){
    Graphics::GraphicsPtr g (new Graphics(currentStyleId(element), Graphics::Point, element));
    m_graphics.append(g);
    addVertices(element.get(), g.get());
}

void GraphicsVisitor::visitInternal(const kmldom::PolygonPtr &element)
{
    Graphics *g = nullptr;
    if (element->has_outerboundaryis()) {
        const auto obound = element->get_outerboundaryis();
        if (obound->has_linearring()) {
            const auto ring = obound->get_linearring();
            g = new Graphics(currentStyleId(element), Graphics::Polygon, element);
            m_graphics.append(Graphics::GraphicsPtr(g));
            addVertices(ring.get(), g);
        }
    }
    const auto holes = element->get_innerboundaryis_array_size();
    if (g != nullptr && holes > 0) {
        for (size_t i = 0; i < holes; i++) {
            const auto ibound = element->get_innerboundaryis_array_at(i);
            if (ibound->has_linearring()) {
                const auto ring = ibound->get_linearring();
                Graphics *s = new Graphics("", Graphics::Hole, element);
                g->m_subgraphics.append(Graphics::GraphicsPtr(s));
                addVertices(ring.get(), s);
            }
        }
    }
}


void GraphicsVisitor::visitInternal(const kmldom::MultiGeometryPtr &element)
{
    Graphics::GraphicsPtr g(new Graphics(currentStyleId(element), Graphics::MultiGeometry, element));
    m_graphics.append(g);
    const kmldom::MultiGeometry* const multiGeometry = element.get();

    for (size_t i = 0; i < multiGeometry->get_geometry_array_size(); i++) {
        const kmldom::GeometryPtr geometry = multiGeometry->get_geometry_array_at(i);
//        std::cout << "MultiGeometry: geometry->Type() = " << kmlDomTypeToString(geometry->Type()) << std::endl;

        const kmldom::PolygonPtr polygon = kmldom::AsPolygon(geometry);
        if (polygon != nullptr) {
            visitInternal(polygon);
            continue;
        }
        const kmldom::PointPtr point = kmldom::AsPoint(geometry);
        if (point != nullptr) {
            visitInternal(point);
            continue;
        }
        const kmldom::LinearRingPtr ring = kmldom::AsLinearRing(geometry);
        if (ring != nullptr) {
            visitInternal(ring);
            continue;
        }
        const kmldom::LineStringPtr string = kmldom::AsLineString(geometry);
        if (string != nullptr) {
            visitInternal(string);
            continue;
        }

        const kmldom::MultiGeometryPtr multiGeometry2 = kmldom::AsMultiGeometry(geometry);
        if (multiGeometry2 != nullptr) {
            std::cout << "Multigeometry2 ";
            visitInternal(multiGeometry2);
            continue;
        }

        std::cout << "Unsupported MultiGeometry: geometry->Type() = " << kmlDomTypeToString(geometry->Type()) << std::endl;
    }

}

void GraphicsVisitor::visitInternal(const kmldom::ExtSymbolInfoPtr &element) {
//    Graphics::GraphicsPtr g(new Graphics(currentStyleId(element), Graphics::Point, element));
//    if (element->has_coordinates()) {
//        const kmlbase::Vec3 vec = element->get_coordinates();
//        g->m_vertices.append(QGeoCoordinate(vec.get_latitude(), vec.get_longitude()));
//    }

    Graphics::GraphicsPtr g (new Graphics(currentStyleId(element), Graphics::SvgIconLabel, nullptr));
    g->m_vertices.append(QGeoCoordinate(element->get_coordinates().get_latitude(), element->get_coordinates().get_longitude()));
    m_graphics.append(g);
    g->setExtraData(QString::fromStdString(element->get_code()));
    std::cout << "ExtSymbolInfo: " << element->get_code() << std::endl;
}

void GraphicsVisitor::VisitPlacemark(const kmldom::PlacemarkPtr &element)
{
    // std::cout << "GraphicsVisitor::VisitPlacemark" << std::endl;
    if (element->has_geometry()) {
        const kmldom::GeometryPtr &geometry = element->get_geometry();
        // std::cout << "Placemark: geometry->Type() = " << kmlDomTypeToString(geometry->Type()) << std::endl;

        const kmldom::PolygonPtr polygon = kmldom::AsPolygon(geometry);
        if (polygon != nullptr) {
            visitInternal(polygon);
            return;
        }
        const kmldom::PointPtr point = kmldom::AsPoint(geometry);
        if (point != nullptr) {
            visitInternal(point);
            return;
        }
        const kmldom::LinearRingPtr ring = kmldom::AsLinearRing(geometry);
        if (ring != nullptr) {
            visitInternal(ring);
            return;
        }
        const kmldom::LineStringPtr string = kmldom::AsLineString(geometry);
        if (string != nullptr) {
            visitInternal(string);
            return;
        }

        const kmldom::MultiGeometryPtr multiGeometry = kmldom::AsMultiGeometry(geometry);
        if (multiGeometry != nullptr) {
            visitInternal(multiGeometry);
            return;
        }

        std::cout << "Unsupported Placemark: geometry->Type() = " << kmlDomTypeToString(geometry->Type()) << std::endl;
    } else {
        std::cout << "Placemark: no geometry" << element->Type() << std::endl;
    }
}

void GraphicsVisitor::VisitExtSymbolInfo(const kmldom::ExtSymbolInfoPtr &element)
{
    visitInternal(element);
}

void GraphicsVisitor::VisitMultiGeometry(const kmldom::MultiGeometryPtr &element)
{
    // std::cout << "not impl GraphicsVisitor::VisitMultiGeometry" << std::endl;
    StyleVisitor::VisitMultiGeometry(element);
}

void GraphicsVisitor::VisitPolygon(const kmldom::PolygonPtr &element)
{
    // std::cout << "GraphicsVisitor::VisitPolygon" << std::endl;
    // const kmldom::PolygonPtr polygon = kmldom::AsPolygon(element);
    // visitInternal(polygon);
    StyleVisitor::VisitPolygon(element);
}

void GraphicsVisitor::VisitFolder(const kmldom::FolderPtr &element)
{
    std::cout << "GraphicsVisitor::VisitFolder" << std::endl;
    const kmldom::FolderPtr folder = kmldom::AsFolder(element);
    if (folder->has_name()) {
        std::cout << "folder->get_name() = " << folder->get_name() << std::endl;
    }
    std::cout << "folder->get_feature_array_size() = " << folder->get_feature_array_size() << std::endl;

    StyleVisitor::VisitFolder(element);
}
