// Report the GPU renderer and switch the project to software rendering; result to C:\users\Public\ae-viewer-probe.txt
var s = "";
try { s += "gpuAccelType " + String(app.project.gpuAccelType) + "\n"; } catch (e) { s += "gpuAccelType err " + e.toString() + "\n"; }
try { s += "available " + String(app.availableGPUAccelTypes) + "\n"; } catch (e) { s += "available err " + e.toString() + "\n"; }
try { app.project.gpuAccelType = GpuAccelType.SOFTWARE; s += "now " + String(app.project.gpuAccelType) + "\n"; } catch (e) { s += "set err " + e.toString() + "\n"; }
var f = new File("C:/users/Public/ae-viewer-probe.txt"); f.open("w"); f.write(s); f.close();
