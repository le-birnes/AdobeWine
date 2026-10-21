// Turn off "Hardware Accelerated Viewers v2" (all bit depths); result to C:\users\Public\ae-viewer-probe.txt
var s = "";
var depths = ["8bpc", "16bpc", "32bpc"];
for (var i = 0; i < depths.length; i++) {
    var sec = "Hardware Accelerated Viewers v2 (" + depths[i] + ")";
    try { app.preferences.savePrefAsBool(sec, "Enable Always", false, PREFType.PREF_Type_MACHINE_SPECIFIC); s += sec + " off\n"; }
    catch (e) { s += sec + " err " + e.toString() + "\n"; }
}
try { app.preferences.reload(); s += "reloaded\n"; } catch (e) { s += "reload err " + e.toString() + "\n"; }
var f = new File("C:/users/Public/ae-viewer-probe.txt"); f.open("w"); f.write(s); f.close();
