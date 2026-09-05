# gtl.dwg

Native DWG reader; no DWG SDK, external converter or DXF intermediate file.
The first implementation follows the `gtl.dxf` API and project conventions and
returns the existing `gtl::shape::xDrawing` type.

## Current scope

- AC1014 (R14) and AC1015 (R2000): section directory, classes, object map,
  object boundaries, header/class/map/object CRCs, handles and layer records.
- Geometry: LINE, CIRCLE, ARC, POINT, LWPOLYLINE (including class-based type
  lookup), 2D/3D POLYLINE and VERTEX. Linked vertex ownership and SEQEND records
  are validated. File strings have trailing NUL terminators removed; the Shape adapter converts layer names
  to wide strings using the DWG codepage identifier.
- Model-space geometry converts to `gtl.shape`; colors, visibility, layer
  membership and lineweight enumeration are preserved. Drawing bounds are
  calculated from the converted geometry.
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
  AC1018 through AC1032. These versions' object/string/handle streams are not
  decoded into drawing entities yet: `ReadDWG`/`ReadDWGShape` still explicitly
  return `eREAD_ERROR::unsupported_version` for them.

This is a partial drawing reader, not a complete CAD database implementation.
Header variables/units, text/attributes, native ELLIPSE entities, splines, hatches,
dimensions, paper space and custom object payloads
are not implemented. Linetype definitions/styles are not reconstructed.
Unknown payloads have their boundaries and CRC checked and are recorded in the
diagnostics with the source type and handle. Their raw bytes are not retained.
No byte-preserving round trip or writing API is provided.

`ToShape` omits external/unloaded blocks, fitted polylines, tilted circular curves,
and bulged polylines under tilted or non-uniform transforms, with diagnostics.
Straight segments retain 3D coordinates. Widths and 3D thickness are reported when converting to centerlines.
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
Compression tests cover independent literal/back-reference byte vectors, a small
independently constructed R2004-family container, corruption, truncation and size
limits. The corpus additionally verifies restored sections for all five modern
signatures; R2007 corruption tests exercise header/system/data Reed-Solomon paths.
The fixture is not a full CAD-authored DWG database. Corpus tests distinguish
supported reads from explicit unsupported-version results; a passing corpus
test does **not** mean that every supplied version can be rendered.

## Next implementation stages

1. Decode modern object/class records from the restored sections, including
   version-specific object types and separate data/string/handle streams.
2. Enable each version's drawing API after geometry and ownership regression tests.
3. Header variables, linetypes, text, ellipses and splines; more reference fixtures
   with independently known geometry and expected omission reports.

Format reference: [ODA Open Design Specification for DWG, 5.4.1](https://www.opendesign.com/files/guestdownloads/OpenDesign_Specification_for_.dwg_files.pdf),
chapters 2–7, 10, 20 and 23. Implementation is written in this project, not
copied from a DWG SDK. Codepage identifiers were cross-checked against the
[DraftSight encoding enumeration](https://help.solidworks.com/2025/english/api/draftsightapi/Interop.dsAutomation~Interop.dsAutomation.dsEncoding_e.html).
