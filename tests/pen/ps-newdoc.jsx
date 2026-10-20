// Pen pressure test: blank white canvas, black foreground, Brush tool.
app.preferences.rulerUnits = Units.PIXELS;
var d = app.documents.add(1600, 1000, 72, "pen-pressure", NewDocumentMode.RGB, DocumentFill.WHITE);
var c = new SolidColor(); c.rgb.red = 0; c.rgb.green = 0; c.rgb.blue = 0;
app.foregroundColor = c;
var r = new ActionReference(); r.putClass(stringIDToTypeID("paintbrushTool"));
var a = new ActionDescriptor(); a.putReference(charIDToTypeID("null"), r);
executeAction(charIDToTypeID("slct"), a, DialogModes.NO);
