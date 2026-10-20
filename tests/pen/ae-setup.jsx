// Pen pressure test: comp with a white solid, opened in the Layer panel for the Brush tool.
var comp = app.project.items.addComp("pen-pressure", 1600, 1000, 1, 10, 30);
var solid = comp.layers.addSolid([1, 1, 1], "canvas", 1600, 1000, 1);
comp.openInViewer();
solid.openInViewer();
