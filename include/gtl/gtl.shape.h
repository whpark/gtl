#pragma once

import gtl;
import gtl.shape;

#include "boost/json.hpp"
#include "boost/variant.hpp"
#include "boost/ptr_container/ptr_deque.hpp"
#include "boost/ptr_container/serialize_ptr_deque.hpp"
#include "boost/serialization/vector.hpp"
#include "boost/serialization/string.hpp"
#include "boost/serialization/optional.hpp"
#include "boost/serialization/map.hpp"
#include "boost/serialization/variant.hpp"
#include "boost/serialization/serialization.hpp"
#include "boost/archive/text_iarchive.hpp"
#include "boost/archive/text_oarchive.hpp"
#include "boost/archive/binary_iarchive.hpp"
#include "boost/archive/binary_oarchive.hpp"
#include "boost/serialization/export.hpp"

BOOST_CLASS_EXPORT_GUID(gtl::shape::xShape, "shape")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xLayer, "layer")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xDot, "dot")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xLine, "line")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xPolyline, "polyline")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xPolylineLW, "lwpolyline")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xCircle, "circleXY")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xArc, "arcXY")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xEllipse, "ellipseXY")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xSpline, "spline")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xText, "text")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xMText, "mtext")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xHatch, "hatch")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xDrawing, "drawing")
