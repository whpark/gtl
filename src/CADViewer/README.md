# CADViewer

Build `CADViewer.vcxproj` in the GTL solution (Debug/Release, x64,
Qt Widgets/OpenGLWidgets). The application uses `gtl.dxf`, `gtl.dwg`,
`gtl.shape` and `gtl.qt`. DraftKit and biscuit are not runtime dependencies.
Static controls are defined in `MainWnd.ui`, `OperationsDlg.ui` and
`SelectionFilterDlg.ui`. Operations and Selection Filter are independent modeless
dialogs, opened from View. Closing or pressing Escape hides them; reopening retains
their values. Their positions/sizes persist, and the drawing remains interactive.

## Naming conventions

Keep CADViewer naming consistent with GTL when adding or refactoring code:

- Classes use `x` + PascalCase (`xMainWnd`, `xSelectionCanvas`); plain data
  structs use PascalCase (`EditState`, `EditRecord`).
- Enums use the GTL `e` prefix; semantic aliases use PascalCase (`EntityId`,
  `RevisionId`). Keep conventional aliases such as `this_t` and `base_t`.
- Functions use PascalCase and describe their work (`CloneEntityRecords`,
  `RefreshSelectionPresentation`). Qt overrides retain Qt's spelling.
- Private instance members use `m_` + camelCase, including UI wrappers (`m_ui`).
  Locals, parameters and plain data fields use camelCase. ID collections name
  their contents (`m_entityIds`, `m_workingSetIds`, `m_hiddenIds`).
- UI object names use camelCase and consistent role prefixes (`actionWorkingSet`,
  `actionHistory`, `actionLog`). Update Designer connections and test lookups
  together when renaming them. Persisted dock/dialog object names are compatibility
  keys and require migration if changed.
- Files use the primary type's name without the `x` prefix (`MainWnd.h/.cpp`).
  Keep existing GTL API names and established static/global names when integrating
  with them; naming cleanup does not require a repository-wide rename.

`EntityId` and `RevisionId` document different meanings but currently alias the
same integer type; they do not enforce type separation at compile time.

## Files

- **Ctrl+O** or file drop opens DXF, DWG or `.shape`. Failed reads retain the
  current document. Unsaved edits offer Save / Discard / Cancel on open/close.
- **Ctrl+S / Ctrl+Shift+S** saves the complete drawing as `.shape`. DXF/DWG inputs
  are imported geometry; saving uses `.shape`, never overwrites the CAD input.
- **Files > Import entities** appends another `.shape` at its original coordinates
  as one undoable edit. Imported line-type definitions accompany their entities;
  conflicting definitions receive new names.
- **Export selection** and **Working Set > Export** write independent `.shape`
  drawings. **Save PNG** captures the drawing widget as currently displayed.

`.shape` is CADViewer's own versioned format, unrelated to DraftViewer `.dkit`.
Version 1 contains `GTL-SHAPE-1\n`, a 32-byte SHA-256 payload digest, then a
Boost binary archive of the GTL drawing and supplemental spline fit points,
hatch double flags and INSERT data omitted by the legacy archive. Nested
entities receive the same supplements. Files are limited to 256 MiB of payload.
Saving uses `QSaveFile` so the destination is replaced only after writing succeeds.
This is a native GTL archive: portability across unrelated Boost/compiler versions
is not promised. New incompatible schema versions need an explicit format reader.

The file stores drawing geometry, source properties, layers and line types.
Selection, session entity IDs, view-only checkbox visibility, Working Set,
clipboard and edit history are session state, not saved workspace state.

## Selection and display

- **Entities** groups converted Shape objects by layer. These are not all native
  DWG database records: block expansion and curve/fill approximation can produce
  multiple rows for one CAD source entity.
- Layer checkboxes toggle children; partially checked layers have mixed visibility.
  Hidden entities remain inspectable, but are excluded from mouse selection and
  highlighting. Checkbox visibility affects only the view.
- Left drag **left to right** selects fully contained entities; **right to left**
  selects intersecting entities. Middle drag pans. Curves use sampled strokes
  with a target chord deviation of 0.25 screen pixel and bounded subdivision.
- Tree selection and drawing selection are synchronized. Properties groups
  multiple selected entities; thicker yellow outlines show visible selected geometry,
  without a filled selection rectangle. Region-drag feedback remains visible while dragging.
- Double-click an entity to fit it at the center, or a layer to fit its visible
  children. Points retain current zoom. **Home** fits; **Escape** clears selection
  and cancels point picking.
- **Selection filter** supports type, layer and minimum/maximum size. Size is
  `max(boundary width, boundary height)` in drawing units; maximum 0 is unlimited.
  Enable **Apply to mouse selection** to apply the same filter to region selection.
  Select all / invert excludes hidden entities.
- **Working Set** retains entity IDs through layer moves, transforms and history.
  Exploded/broken pieces inherit membership; joined paths combine membership.
  Add selected entities, remove entries, clear, reselect or export the set.
- **View** controls text visibility, **방향표시** (enabled by default), dark/light drawing
  background and panels. Direction display marks only selected visible entities:
  green arrowhead = start, red arrowhead = end, oriented along path tangents.
  Closed paths offset the end arrow forward to keep both marks visible. Arrowheads
  retain their screen size when zooming; points/text have no direction marks.
  Panel placement persists; **Reset layout** restores the
  entity/property docks and tabbed Working Set / History / Log.

## Editing

Choose **Selection**, **Working Set** or **Whole drawing** as the operation target.
Only parameters needed by the chosen operation are displayed. **Pick X/Y** reads
CAD coordinates from the drawing; Z remains the entered value.

| Operation | Parameters / behavior |
| --- | --- |
| Translate | X/Y/Z displacement |
| Rotate | Counterclockwise XY angle in degrees |
| Scale | Positive uniform factor |
| Mirror X/Y/Z | Reflection about the selected coordinate plane |
| Transform center | Entered X/Y/Z, origin, or target bounding-box center |
| Reverse | Reverse path direction, including spline knots and tangents |
| Set Layer / Color / Line Weight | Layer name, QColor name or `#RRGGBB`, CAD weight code |
| Join | Connected open lines/arcs/polylines; positive endpoint tolerance; matching style/layer required; arc bulges preserved |
| Explode | Polyline segments become lines/arcs with inherited properties |
| Set Start | Closed polyline: vertex index, nearest vertex, or nearest outline point; open polyline: nearest endpoint mode |
| Split At | Insert the nearest point on a polyline outline; preserve geometry and split arc bulges |
| Break At Vertex | Separate an open polyline at an interior vertex, or open a closed polyline at that vertex |
| Delete | Remove target entities |
| Paste | Clipboard copies, X/Y/Z offset from original coordinates, rows/columns and X/Y spacing |

**Ctrl+C** copies selected entities. **Ctrl+V** opens Paste parameters; Apply creates
its rectangular array. **Delete** removes the current selection. Text input fields
keep their normal editing shortcuts. Each paste/edit is limited to 100,000 result
entities. Operations work on copies and reject invalid parameters before committing.
Polyline-only operations skip other types; Join rejects unsupported/disconnected
paths. Unsupported or invalid operations are reported in **Log**.

**Ctrl+Z / Ctrl+Y** undo/redo up to 50 edits. **History** can jump to a retained
state or clear history. New edits after undo discard the redo branch. Selection,
visibility and Working Set references restore with each edit state. `*` in the
window title indicates unsaved geometry/property changes.

## Rendering scope and tests

GTL's existing reader/Shape/LayeredMatView rendering scope still applies: unsupported
reader geometry cannot be recovered by this editor, and text follows the current
renderer font/layout limitations. This is a 2D CAD geometry editor, not full CAD
model/layout editing or a DWG/DXF writer. CAD line-weight codes stay in the source
and convert to pixel widths only in the display copy. DWG omission/approximation
reports remain available in **Log**.

After building the Debug x64 GTL dependencies, run:

```powershell
python src/CADViewer/tests/run.py
```

Use `--msbuild` and `--qt` to override installed toolchain locations. The runner
builds an isolated smoke executable and uses private settings/offscreen Qt.
It verifies DXF/DWG loading, region/tree selection, visibility, fit, transformations,
style edits, arrays, history, Working Set inheritance, filters, arc-aware geometry
operations, `.shape` round trips and corruption rejection, selection/set exports,
import, PNG export, dock tabs, modeless dialog close/reopen behavior and cancellation of unsaved changes. Logs and
screenshots are written under `build-dwg/verification/viewer-smoke`.
