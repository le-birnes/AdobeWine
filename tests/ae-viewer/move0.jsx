// Go back to frame 0 and move the green solid to the bottom-right quarter (layer change only, no time change).
for (var i = 1; i <= app.project.numItems; i++) {
    var it = app.project.item(i);
    if (it instanceof CompItem && it.name == "viewer-test") {
        it.time = 0;
        it.layer("green").property("Position").setValue([1440, 810]);
    }
}
