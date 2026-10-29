# VolUX – Cost Optimiser

A desktop application built with **C++17** and **SFML 3** that finds the cheapest dimensions for a closed container (box or cylinder) of a given volume, when the top/bottom and side materials have different costs per square metre. Results are drawn to scale next to a 1-metre stick so you can see how big the container actually is.

---

## What It Does

You enter:

1. The **volume** the container must hold (cubic metres)
2. The **cost of the top & bottom faces** (PHP per square metre)
3. The **cost of the side faces** (PHP per square metre)

The program computes the dimensions that **minimise the total material cost** while keeping the volume fixed, then shows the dimensions, surface area, and minimum cost, along with a scaled drawing.

---

## Features

### Two shape modes

| Shape | Description | Optimal dimension |
|-------|-------------|-------------------|
| **Box** | Closed box with a square base (width = depth) | `w = ∛(Cs · V / Cb)`, `h = V / w²` |
| **Cylinder** | Closed cylinder with circular top and bottom | `r = ∛(Cs · V / (2π · Cb))`, `h = V / (π r²)` |

Where `V` is volume, `Cb` is the cost of the top/bottom, and `Cs` is the cost of the sides.

If only one of the two costs is entered (the other is 0), the optimiser falls back to the cost-neutral shape: a cube for the box, and `r = ∛(V/π)` for the cylinder.

### Results shown

- Optimal dimensions (width/height/depth, or radius/height)
- Total surface area
- **Minimum cost** (highlighted in gold, in PHP)
- Result summary card with volume check and height ratio (`h/w` or `h/r`)

### Scale visualisation

- Isometric 3-D box and cylinder rendered with shaded faces
- Dimension labels drawn on the shape
- A **1-metre stick** with centimetre markings beside the shape for scale reference
- Automatic scaling so large or small results stay on screen, with the current scale (`1m = N px`) displayed

### Saving and managing records

- Save any result to **`box_records.txt`** with a numbered entry and a timestamp
- Each record stores the shape, volume, both costs, dimensions, surface area, and minimum cost
- Built-in **Records screen** to browse all saved entries
- Select a record and **delete** it, with a confirmation prompt
- Remaining records are automatically renumbered after a deletion
- Scrolling via the arrow keys or mouse wheel

### Input handling and validation

- Numbers only (digits and a single decimal point)
- Backspace supported; Tab cycles between fields
- Clear error messages for:
  - Volume ≤ 0
  - Negative costs
  - Both costs being 0
  - Invalid number input

### Interface

- Dark-themed UI with a left input panel and a right visualisation panel
- Fully usable by **mouse or keyboard**
- Cross-platform font loading (Linux, macOS, and Windows font paths are tried in turn)

---

## Controls

### Shape selection screen

| Input | Action |
|-------|--------|
| `1` or click the Box card | Open the Box optimiser |
| `2` or click the Cylinder card | Open the Cylinder optimiser |
| `R` or click *View Records* | Open saved records |
| `Esc` | Quit |

### Optimiser screen

| Input | Action |
|-------|--------|
| Type digits / `.` | Enter a value in the active field |
| `Tab` or click a field | Switch field |
| `Enter` or click *Compute* | Calculate the optimum |
| `S` or click *Save* | Save the result to file |
| `R` or click *Records* | Open saved records |
| `Esc` or click *Back* | Return to shape selection (inputs are cleared) |

### Records screen

| Input | Action |
|-------|--------|
| Click a card | Select / deselect a record |
| `D` or click *Delete* | Ask to delete the selected record |
| `Y` / `N` | Confirm / cancel deletion |
| `↑` / `↓` or mouse wheel | Scroll |
| `Esc` or click *Back* | Return to the previous screen |

---

## Code Structure

The program is a single file (`main.cpp`) organised into clear classes:

| Component | Responsibility |
|-----------|----------------|
| `BoxResult`, `CylResult` | Plain structs holding computed results |
| `Optimizer` | Pure math: static functions that compute optimal box and cylinder dimensions |
| `RecordManager` | All file I/O: save, load, delete (with renumbering), and count records |
| `AppState` | Current screen, input strings, results, and records-screen state |
| `Renderer` | All SFML drawing: buttons, fields, isometric box, cylinder, meter stick, and each screen |
| `App` | Owns the window, font, and state; runs the event loop and handles keyboard, mouse, text, and scroll events |

---

## Saved Record Format

Records are appended to `box_records.txt` in the working directory:

```
[#1] 2026-10-29 14:32:10
Shape       : BOX
Volume      : 1.0000 cu.m
Cost Base   : PHP 50.0000 /sq.m
Cost Side   : PHP 30.0000 /sq.m
Width  (w)  : 0.8434 m
Height (h)  : 1.4057 m
Depth  (d)  : 0.8434 m
Surface Area: 6.3300 sq.m
Min Cost    : PHP 247.34
--------------------------------------------
```

> The file is created in the program's **working directory**. If you delete the `build` folder and rebuild, any records stored there are deleted too, so back them up first.

---

## Building

**Requirements:** a C++17 compiler, CMake, and SFML 3 (Graphics, Window, System, and Audio components if linked).

```bash
cmake -B build
cmake --build build
```

Run the resulting `VolUX` executable from inside the `build` folder (or wherever your SFML DLLs are located on Windows).

---

## Notes

- Dimensions are in **metres**, areas in **square metres**, and costs in **Philippine pesos (PHP)**.
- Both shape modes store their records in the same file, `box_records.txt`.