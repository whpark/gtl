# gtl.dwg

Native DWG reader; no DWG SDK, external converter or DXF intermediate file.
The first implementation follows the `gtl.dxf` API and project conventions and
returns the existing `gtl::shape::xDrawing` type.

## Current scope

- AC1014 (R14) and AC1015 (R2000): section directory, classes, object map,
  object boundaries, header/class/map/object CRCs, handles and layer records.
- AC1018 (R2004): classes, handle map and object records decoded from restored
  sections; missing-dictionary flags, indexed/RGB colors, explicit vertex,
  attribute and block ownership lists. `ReadDWG` and `ReadDWGShape` are enabled
  for the geometry listed below, with the same omission diagnostics as R14/R2000.
- AC1021 (R2007), AC1024 (R2010), AC1027 (R2013) and AC1032 (R2018):
  separate data/string/handle streams, UTF-16 strings, version-specific object
  type/size prefixes, classes and explicit ownership lists. The drawing APIs
  support the same basic geometry subset for all seven recognized signatures.
- Geometry: LINE, CIRCLE, ARC, POINT, LWPOLYLINE (including class-based type
  lookup), 2D/3D POLYLINE and VERTEX. Linked vertex ownership and SEQEND records
  are validated. File strings have trailing NUL terminators removed; the Shape adapter converts layer names
  to wide strings using the DWG codepage identifier before R2007. R2007 and later
  document strings are UTF-8, decoded from validated UTF-16 surrogate pairs.
- Model-space geometry converts to `gtl.shape`; colors, visibility, layer
  membership and lineweight enumeration are preserved. Drawing bounds are
  calculated from the converted geometry.
- ELLIPSE: WCS center/major axis, normal, axis ratio and eccentric-anomaly
  parameters. XY ellipses/arcs preserve their parameterization under nested,
  reflected and non-uniform block transforms; tilted/degenerate curves produce
  omission diagnostics.
- SPLINE: control points, knots, raw weights, fit points/tangents and R2013+
  scenario fields are retained. Non-rational control splines (including constant
  positive weights) convert to `xSpline` with transformed WCS control points.
  Shape rendering now uses supplied knots and their parameter domain. Spline
  bounds use the control-point hull, not exact curve extrema.
- Rational splines: homogeneous de Boor evaluation and adaptive polyline sampling.
  Fit-point splines: chord, square-root or uniform parameter interpolation, with
  endpoint tangent constraints when provided. Interpolation is bounded to 256 fit
  points and degree 16; rational evaluation is bounded to degree 64. Reconstructed
  fit curves may differ from the originating CAD application's fitting solver.
- Header variables: versioned typed fields, independent header CRC/sentinels,
  insertion units (`INSUNITS`) and `MEASUREMENT`. `headerVariables` exposes system
  values; space-specific names end in `_MSPACE`/`_PSPACE`, date/time pairs use
  `_DAY`/`_DAYS` and `_MILLISECONDS`. Source units are retained without automatic
  rescaling. Unnamed/reserved header fields are not interpreted as CAD properties.
- TEXT, ATTRIB, ATTDEF and MTEXT: native strings, positions, alignment, dimensions,
  styles/font filenames, attribute tags/prompts/flags and embedded R2018 multiline
  attributes. INSERT attributes use their parent-space placement; nonconstant
  ATTDEF records are replaced by the actual attributes. Shape emits `xText` and
  `xMText`; font availability, rich formatting and wrapping depend on the renderer.
- LTYPE: names, descriptions, dash records, complex-glyph metadata and string areas.
  Layer/ByBlock/explicit linetype selection, entity scale and global `LTSCALE`
  produce bounded dash strokes. Complex glyphs remain in the document; their
  dash strokes are drawn with an omission diagnostic for the glyphs themselves.
- DIMENSION: ordinate, linear, aligned, angular (three-point/two-line), radius and
  diameter definitions, text, measurement, styles and anonymous block references.
  Cached anonymous blocks convert with the dimension's placement and scale.
  A missing cache is reported; regenerating a dimension from DIMSTYLE is not done.
- 2D SOLID/TRACE records, including dimension arrowheads, retain all four OCS
  corners and convert through the same boundary/scanline fill path as solid hatches.
- HATCH: polyline/bulge, line, circular/elliptical arc and spline paths, boundary
  handles, pattern definitions, seed points and gradient metadata. Pattern lines
  are clipped against contours using the hatch island style. Solid and gradient
  fills use configurable scanline approximations; gradient colors use a linear
  interpolation of endpoint colors, not every CAD-specific gradient function.
- Paper space: select model, paper or both with `sShapeOptions::space`, optionally
  restrict paper geometry to a `paperBlock` handle. Viewport model projections,
  plot transforms and viewport-specific clipping are not reconstructed.
- BLOCK/INSERT and MINSERT: linked block membership, base points, nested transforms,
  rotated array spacing, extrusion coordinates, layer 0 and ByBlock inheritance.
  Non-uniformly scaled XY circles/arcs become ellipses. INSERT cycles and conversion
  limits produce diagnostics, while broken entity links/owners fail the read.
- AC1018, AC1024, AC1027 and AC1032: R2004-family LZ decompression, decoded
  file header, global page map, named section map, raw/compressed pages, sparse
  zero pages, file CRC32 and system/data page checksums.
- AC1021: R2007 LZ decompression with literal permutations, interleaved and
  non-interleaved Reed-Solomon page decoding, named sections, header CRC64,
  compressed/uncompressed system CRC64 and data-page CRC64/checksums.
- `ReadDWGContainer` returns the restored bytes of all logical sections for
  AC1018 through AC1032, independently of drawing conversion.

This is a partial drawing reader, not a complete CAD database implementation.
3D solids and custom object payloads are not implemented. Materials and visual
styles are not reconstructed. Text bounds currently use the Shape insertion-point
bound rather than font-metric glyph extents.
Unknown payloads have their boundaries and CRC checked and are recorded in the
diagnostics with the source type and handle. Their raw bytes are not retained.
No byte-preserving round trip or writing API is provided.

`ToShape` omits external/unloaded blocks, fitted polylines, tilted circular curves,
and bulged polylines under tilted or non-uniform transforms, with diagnostics.
Straight segments retain 3D coordinates. Widths and 3D thickness are reported when converting to centerlines.
Entity transparency and unresolved color-book references are reported as omitted;
RGB and indexed colors are converted, including inherited layer/ByBlock colors.
Fit reconstruction, sampled curves and scanline fills are explicitly reported as
approximations. Full source curve/fill definitions remain available in the document.
Such a conversion must not be treated as an exact rendering of the input drawing.
Inspect the report when using the convenience API.

## API

```cpp
#include "gtl/dwg/dwg.h"

gtl::dwg::sReadReport report;
if (auto drawing = gtl::dwg::ReadDWGShape(path, &report)) {
    // Use drawing as with gtl::dxf::ReadDXFShape.
    // report.diagnostics describes unsupported/omitted data.
} else {
    // report.error, report.message and report.version explain the failure.
}

gtl::dwg::xDWG dwg;
if (dwg.ReadDWG(path)) {
    auto const& document = dwg.GetDocument();
    auto drawing = gtl::dwg::ToShape(dwg, &report);
}

gtl::dwg::sShapeOptions options;
options.space = gtl::dwg::eSPACE::paper;
options.curveTolerance = 0.01;     // Drawing units, sampled chord deviation.
options.solidHatchSpacing = 0.25;  // Drawing units between scanlines.
// options.hatchBoundaryOnly = true; // Preserve only hatch boundary geometry.
auto paper = gtl::dwg::ReadDWGShape(path, &report, options);

// AC1018 through AC1032: section-level access, before object interpretation.
if (auto container = gtl::dwg::ReadDWGContainer(path, &report)) {
    auto const& objectBytes = container->sections.at("AcDb:AcDbObjects").data;
    auto const& handles = container->sections.at("AcDb:Handles").data;
}
```

`ReadDWG` clears prior data before each attempt. Unsupported versions, I/O errors
and malformed input return `false`; successful reading does not imply complete
entity coverage. `ToShape` throws `std::logic_error` on an unloaded drawing.
The current in-memory reader rejects files larger than 512 MiB before allocation.
The optional third argument to `ToShape` and `ReadDWGShape`, `sShapeOptions`, limits
block nesting (default and hard ceiling 64), emitted entities (default 100,000),
and graph/array visits (default 1,000,000). A limit returns a partial drawing with
a diagnostic; it does not change the successful parse status. Empty MINSERT arrays
also consume the visit budget.
`maxCurveSegments` defaults to 65,536 and `maxHatchSegments` to 100,000. Hatch and
dash expansion also obey the drawing-wide entity/visit budgets. `curveTolerance`
defaults to 0.01 drawing units and `solidHatchSpacing` to 1.0. Subdivision checks
sampled chord deviations, not a certified global geometric error bound. Text
layout, dash phase under non-uniform transforms and fit interpolation can require
application-specific rendering rules beyond the retained source parameters.

`ReadDWGContainer` accepts an optional `sContainerOptions` third argument. Defaults
are 512 MiB input, 512 MiB cumulative decoded section/system buffers, and one
million page-map entries. Limits return `resource_limit`; malformed compressed
streams, checksum failures and password-encrypted data are rejected. The API
returns no partial container on failure. Temporary page buffers require additional
memory beyond the cumulative decoded-byte budget.

Reed-Solomon parity is checked for complete codewords; damaged codewords are
rejected rather than repaired. Some non-interleaved writers omit the final random
RS padding, preventing verification of that last codeword's parity. Its actual
payload is still verified by both CRC64 and the data checksum. Redundant header
copies and random checking-sequence fields are not used for recovery or validated.

The parser in `detail/` depends only on the C++ standard library. The public API
and Shape adapter live together in `dwg.cpp`, as in `gtl.dxf`: existing Shape
headers instantiate Boost serialization registration and cannot currently be
included in multiple implementation translation units of the same DLL.

## Build and tests

The Visual Studio solution contains `gtl.dwg` and `test_dwg` (x64 Debug/Release).
CMake keeps DWG opt-in, with no new external dependencies:

```powershell
cmake -S . -B build-dwg -DGTL_BUILD_DWG=ON -DGTL_BUILD_SHAPE=ON
cmake --build build-dwg --config Debug --target test.dwg
ctest --test-dir build-dwg -C Debug --output-on-failure
```

Existing unrelated UI module options can be disabled when configuring a reader-only
build. `GTL_BUILD_TEST_DWG` controls the test executable. The usual GTL/vcpkg
dependencies and Catch2 are still required.
The Shape CMake target selects the matching Debug/Release TinySpline C++ library
and refreshes its same-named runtime DLL in the shared output directory when
switching configurations. Debug and Release executables in that directory should
be run with their matching build's runtime; they cannot share both DLL ABIs at once.
The DWG CMake test executables use `test.dwg.x64D.exe` / `test.dwg.x64R.exe`
to avoid overwriting each other; CTest selects the configured executable.

Tests use `src/test_dwg/DWG` by default (resolved independently of the process
working directory). Set `GTL_DWG_TEST_DIR` to select another corpus. A missing
default corpus is an explicit skip; a missing configured directory is a failure.
Only `.dwg`/`.DWG` files are inspected; files are never renamed or rewritten.

Unit tests use independently encoded small parser fixtures and known bit patterns
to check coordinates, colors, CP949 layer names, bounds, malformed inputs,
truncation and stale-state prevention without requiring the external corpus.
R14/R2000 graph fixtures cover linked vertices, block bases and nested arrays,
layer inheritance, non-uniform scales, rotated ellipse bounds, broken ownership,
cycles and conversion budgets.
R2004 fixtures additionally cover explicit ownership order, duplicate/missing
vertices, attributes, true colors, class bitlong fields and object CRC failures
inside a valid compressed container.
R2007/R2010/R2013/R2018 fixtures check independent split streams, Korean and
supplementary Unicode characters, malformed string sizes and surrogate pairs,
true colors and public Shape conversion for the R2004-family containers.
Ellipse/spline fixtures cover all seven signatures, WCS axes, reflected and
non-uniform transforms, arc bounds, malformed spline counts/knots, fit points,
weights and actual canvas output for non-uniform knots and negative ellipse sweeps.
Further independent fixtures cover TEXT/ATTRIB/ATTDEF, MTEXT and HATCH across all
seven revisions, paper-space selection, dimension block placement, LTYPE dash
coordinates, a rational quarter-circle, fit interpolation, hatch holes/styles and
bounded expansion, and SOLID/TRACE corner order. Corpus checks also require header variables and style/linetype
tables, and independently corrupt a restored header to verify its CRC rejection.
Compression tests cover independent literal/back-reference byte vectors, a small
independently constructed R2004-family container, corruption, truncation and size
limits. The corpus additionally verifies restored sections for all five modern
signatures; R2007 corruption tests exercise header/system/data Reed-Solomon paths.
The fixtures are not full CAD-authored DWG databases. All 25 supplied DWG files
pass object reading and conversion checks; drawings with no supported geometry
may produce an empty drawing with omission diagnostics. A passing corpus test
does **not** mean that all source geometry can be rendered.

## Remaining fidelity limits

The requested reader categories above have read/conversion paths. Complete CAD
rendering would additionally require dimension regeneration, font/rich-text
layout, complex linetype glyphs, exact solid/gradient fills, viewport projection
and more custom/3D entities. These are not silently treated as exact conversions;
inspect diagnostics and keep source definitions when precise fidelity matters.

Format reference: [ODA Open Design Specification for DWG, 5.4.1](https://www.opendesign.com/files/guestdownloads/OpenDesign_Specification_for_.dwg_files.pdf),
chapters 2–7, 10, 20 and 23. Implementation is written in this project, not
copied from a DWG SDK. Codepage identifiers were cross-checked against the
[DraftSight encoding enumeration](https://help.solidworks.com/2025/english/api/draftsightapi/Interop.dsAutomation~Interop.dsAutomation.dsEncoding_e.html).
