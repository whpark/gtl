# CADViewer

Build `CADViewer.vcxproj` in the GTL solution (x64, Qt Widgets/OpenGLWidgets).
The application uses `gtl.dxf`, `gtl.dwg`, `gtl.shape` and `gtl.qt`.

- **Ctrl+O** opens a DXF or DWG file. A failed read keeps the previous drawing.
- **Entities** groups converted display geometry by layer. These rows are Shape
  objects, not a complete list of native DWG database objects; block expansion and
  curve/fill approximation can produce multiple rows for one source entity.
- Select a row to see common properties and the Shape geometry description in
  **Properties**, and a yellow geometry overlay and bounds in the view.
- **Escape** clears selection. **Home** fits the drawing. Both docks can be
  restored from the **View** menu.

DWG omission/approximation diagnostics appear after loading and remain available
in the geometry-details tooltip. Unsupported reader geometry cannot be displayed.
Text rendering follows the existing LayeredMatView renderer's font limitations.
CAD line-weight codes are retained for inspection and converted to screen pixels
in a separate display copy. Selection overlays do not modify source properties.

Verification: open a DXF and a DWG, select and clear entity rows, reopen a second
file, and attempt a missing file. Check that properties and overlays follow the
current row, no previous selection remains after reopening, and a failed load
preserves the current drawing.
