// Included only by MainWnd.cpp: Shape export registrations must stay in one TU.
namespace {
constexpr qsizetype ShapeFileLimit = 256 * 1024 * 1024;
// Preserve fields omitted by the legacy GTL archive, including nested entities.
template<class Archive> void ShapeSupplement(Archive& archive,gtl::shape::xShape& shape) {
    using namespace gtl::shape;
    shape.m_cookie.reset();
    if(auto* spline=dynamic_cast<xSpline*>(&shape))archive & spline->m_ptsFit;
    if(auto* hatch=dynamic_cast<xHatch*>(&shape)){archive & hatch->m_bDouble;for(auto& b:hatch->m_boundaries)ShapeSupplement(archive,b);}
    if(auto* insert=dynamic_cast<xInsert*>(&shape)){archive & static_cast<xShape&>(*insert);archive & insert->m_pt;}
    if(auto* drawing=dynamic_cast<xDrawing*>(&shape))for(auto& layer:drawing->m_layers)ShapeSupplement(archive,layer);
    if(auto* layer=dynamic_cast<xLayer*>(&shape))for(auto& child:layer->m_shapes)ShapeSupplement(archive,child);
}
void ClearCookies(gtl::shape::xShape& shape) {
    using namespace gtl::shape;shape.m_cookie.reset();
    if(auto* drawing=dynamic_cast<xDrawing*>(&shape))for(auto& l:drawing->m_layers)ClearCookies(l);
    if(auto* layer=dynamic_cast<xLayer*>(&shape))for(auto& child:layer->m_shapes)ClearCookies(child);
    if(auto* hatch=dynamic_cast<xHatch*>(&shape))for(auto& b:hatch->m_boundaries)ClearCookies(b);
}
void WriteShapeFile(QString const& filename, gtl::shape::xDrawing const& source) {
    auto drawing=source;
    ClearCookies(drawing);
    std::ostringstream buffer(std::ios::binary);
    {boost::archive::binary_oarchive archive(buffer);archive << drawing;ShapeSupplement(archive,drawing);}
    auto bytes=buffer.str();
    if(bytes.size()>ShapeFileLimit)throw std::runtime_error("Drawing exceeds .shape size limit");
    QByteArray payload(bytes.data(),static_cast<qsizetype>(bytes.size()));
    QSaveFile file(filename);
    if(!file.open(QIODevice::WriteOnly))throw std::runtime_error(file.errorString().toStdString());
    QByteArray header("GTL-SHAPE-1\n");
    auto digest=QCryptographicHash::hash(payload,QCryptographicHash::Sha256);
    if(file.write(header)!=header.size() || file.write(digest)!=digest.size() || file.write(payload)!=payload.size() || !file.commit())
        throw std::runtime_error(file.errorString().toStdString());
}
std::shared_ptr<gtl::shape::xDrawing> ReadShapeFile(QString const& filename) {
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly))throw std::runtime_error(file.errorString().toStdString());
    if(file.size()>ShapeFileLimit+44 || file.size()<44)throw std::runtime_error("Invalid .shape file size");
    if(file.read(12)!=QByteArray("GTL-SHAPE-1\n"))throw std::runtime_error("Unsupported .shape format version");
    auto digest=file.read(32),payload=file.readAll();
    if(QCryptographicHash::hash(payload,QCryptographicHash::Sha256)!=digest)throw std::runtime_error("Damaged .shape file (checksum mismatch)");
    std::istringstream buffer(std::string(payload.constData(),payload.size()),std::ios::binary);
    auto drawing=std::make_shared<gtl::shape::xDrawing>();
    {boost::archive::binary_iarchive archive(buffer);archive >> *drawing;ShapeSupplement(archive,*drawing);}
    if(buffer.peek()!=std::char_traits<char>::eof())throw std::runtime_error("Unexpected trailing .shape data");
    size_t count=0;
    for(auto const& layer:drawing->m_layers)count+=layer.m_shapes.size();
    if(count>1000000)throw std::runtime_error("Too many .shape entities");
    return drawing;
}
}
