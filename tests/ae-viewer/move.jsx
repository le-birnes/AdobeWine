// Move the green solid of "viewer-test" to the top-left quarter and go to frame 10, so the viewer must redraw.
for (var i = 1; i <= app.project.numItems; i++) {
    var it = app.project.item(i);
    if (it instanceof CompItem && it.name == "viewer-test") {
        it.layer("green").property("Position").setValue([480, 270]);
        it.time = 10 / 30;
    }
}
