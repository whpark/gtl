# CAD entity support

The biscuit.shape/dxf/dwg additions were reviewed against a read-only snapshot
of the biscuit working tree on 2026-09-16 (HEAD `bcd27d6`, including uncommitted
CAD entity changes), then refreshed for the basic-rendering additions. GTL retains headers, DLLs, `xDrawing`/`xLayer`, and Boost
archives. The existing DWG integrity checks, compression readers, spline/hatch
conversion and block expansion remain in use.

## Native geometry

- `x3DFace`: four WCS corners and four invisible-edge bits; wireframe rendering.
- `xSolid` / `xTrace`: perimeter-ordered corners (DXF order 0,1,3,2) and a
  thickness vector. The adapters apply OCS and block placement. DWG also keeps
  its existing optional scanline fill output.
- `xRay` / `xXLine`: origin and direction, clipped through `ICanvas::GetClippingRect`
  or `DrawROI`. They have no finite auto-fit bounds. CADViewer supports crossing
  selection; a finite window cannot fully contain an unbounded entity.

## Data-backed geometry

`xCadEntity` stores the source entity name, ordered `sCadGroup` values, optional
`sCadBinary` payload, and an affine placement (`m_origin`, `m_axes`). Transforming
a shape changes the placement, not the original group/payload coordinates.
Use `ToWorld` after interpreting the source coordinate system.

Concrete types cover dimensions (including seven subtypes), ATTDEF/ATTRIB,
LEADER/MLEADER, TOLERANCE, IMAGE/UNDERLAY/WIPEOUT, OLEFRAME/OLE2FRAME,
MLINE/HELIX, 3DSOLID/BODY/REGION/SURFACE/MESH, TABLE/SHAPE/SECTION,
VIEWPORT/LIGHT/SUN and proxy entities. The factory accepts common CAD aliases
such as `MULTILEADER`, `ACAD_TABLE`, surface variants and PDF/DWF/DGN underlays.

DXF conversion preserves repeated group codes, order, string values and binary
chunks. Known complex CAD entities with unfamiliar subclass layouts can retain
their complete group sequence even when typed decoding is unavailable.
DWG conversion retains the entity payload (excluding size prefix and CRC), bit
offsets, DWG version, codepage, object type and handle. CLASSES entity records
select known types or `xProxyEntity`; non-entity classes are not emitted as shapes.
PFACE/MESH additionally retain decoded owned vertices and signed face indices as
an ordered POLYLINE/VERTEX/SEQEND group sequence. Dimensions and attributes keep
their previous visible block/text output alongside the preserved data shape.

## Basic CAD rendering

`GetRenderGeometry()` supplies world-space line segments and text placements.
Drawing, ROI culling and bounds use this geometry, so CADViewer can fit, select
and highlight these entities without a special CAD canvas implementation.

| Type | Basic rendering |
| --- | --- |
| Seven dimension subtypes | Extension/dimension lines, angular arcs, outlined arrows and measurement text |
| LEADER / MULTILEADER | Vertex paths, arrows, optional hooks/doglegs and MLEADER context text |
| TOLERANCE | Multi-row compartment frames and text, including vector position/diameter and circled M/L/S symbols |
| MLINE | Mitered parallel elements and cut intervals |
| HELIX | Sampled constant-radius helix with axis, handedness and pitch |
| TABLE | Unmerged legacy cell grid and text |

DXF dimensions and tables prefer an available display block. DWG dimensions
retain their existing block expansion, and use decoded defining points for basic
rendering when the block is unavailable. `m_bExternalGraphics` suppresses duplicate
fallback geometry after an importer emits the display block.
DWG LEADER, TOLERANCE and MLINE now decode render fields. DWG MULTILEADER,
HELIX and TABLE remain binary-only; their DXF group-backed forms can render.

The renderer caps output at 16,384 segments and 4,096 text placements. Tables
exceeding 4,096 cells and helices exceeding 10,000 turns are rejected. Text bounds
estimate character width; typography depends on the canvas's `Text` implementation.
Full DIMSTYLE layout, custom arrows, spline leaders, merged/linked table cells,
MLINE caps/fills, ACIS evaluation, external images/fonts/underlays and mesh surface
rendering are not implemented. Source fields are preserved.

For unsupported data-backed shapes, the existing `ICanvas::DrawCadEntity` hook
remains available (a no-op by default), with no inferred bounds. Preserved entity
bytes do not constitute a complete, writable CAD object graph.

## Storage and verification

New types support clone/compare and GTL's Boost binary/text archives, including
CADViewer `.shape` storage. Existing enum values remain unchanged. The CAD base uses Boost archive version 1
to store `m_bExternalGraphics`; version 0 from the previous GTL implementation
remains readable. Existing non-CAD archive layouts are unchanged. New files containing these classes require the updated reader;
no compatibility with biscuit or DraftViewer file formats is implied.
Rebuild dependent binaries because public structures and `ICanvas` changed.

Tests in `test.dxf`, `test.dwg` and CADViewer's smoke runner cover archive
round-trips, repeated/binary groups, native coordinate conversion, clipping,
hidden edges, dynamic classes, render fields across seven DWG revisions, geometry
limits, cached display blocks, legacy CAD archives and editor workflows.
