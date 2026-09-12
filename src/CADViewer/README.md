# CADViewer

Build `CADViewer.vcxproj` in the GTL solution (x64, Qt Widgets/OpenGLWidgets).
The application uses `gtl.dxf`, `gtl.dwg`, `gtl.shape` and `gtl.qt`.

- **Ctrl+O** opens a DXF or DWG file. A failed read keeps the previous drawing.
- **Entities** groups converted display geometry by layer. These rows are Shape
  objects, not a complete list of native DWG database objects; block expansion and
  curve/fill approximation can produce multiple rows for one source entity.
- Layer checkboxes toggle all child entities; a partial check indicates mixed visibility.
- Toggle an entity checkbox to show or hide it. Hidden entities remain inspectable
  but do not receive a selection overlay. Visibility changes affect only the view.
- Drag in the drawing with the left mouse button: left-to-right selects fully
  contained entities; right-to-left selects intersecting entities. Middle drag pans.
  Hidden entities are excluded. The tree supports multiple selection and Properties
  groups the selected entities; all visible selected entities receive an overlay.
  Curve hit testing uses sampled strokes (target chord deviation 0.25 screen pixel,
  bounded subdivision); text uses the current renderer's text footprint.
- Select a row to see common properties and the Shape geometry description in
  **Properties**, and a yellow geometry overlay and bounds in the view.
- Double-click an entity to fit it at the view center; double-click a layer to fit
  its visible entities. Hidden entities remain hidden. Points keep the current zoom.
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
