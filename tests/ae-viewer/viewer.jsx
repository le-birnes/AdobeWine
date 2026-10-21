// AE viewer test: a comp with a red solid, a green half-size solid and a text layer, opened in the
// Composition viewer. The viewer should show red with a green square and white text.
var comp = app.project.items.addComp("viewer-test", 1920, 1080, 1, 10, 30);
comp.layers.addSolid([1, 0, 0], "red", 1920, 1080, 1);
var g = comp.layers.addSolid([0, 1, 0], "green", 960, 540, 1);
var t = comp.layers.addText("AdobeWine viewer");
t.property("Source Text").setValue(new TextDocument("AdobeWine viewer"));
comp.openInViewer();
