// Animate the green solid from the left (frame 0) to the right (frame 30) of "viewer-test", time at frame 0.
for (var i = 1; i <= app.project.numItems; i++) {
    var it = app.project.item(i);
    if (it instanceof CompItem && it.name == "viewer-test") {
        var p = it.layer("green").property("Position");
        p.setValueAtTime(0, [480, 540]);
        p.setValueAtTime(1, [1440, 540]);
        it.time = 0;
    }
}
