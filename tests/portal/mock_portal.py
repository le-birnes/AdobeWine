#!/usr/bin/env python3
# Minimal org.freedesktop.portal.FileChooser for testing comdlg32's portal path without a desktop.
# Needs dbus-python + PyGObject (Ubuntu: python3-dbus python3-gi; run with the system python).
#
#   mock_portal.py <log.json> <uri> [<uri> ...]
#
# Answers every OpenFile/SaveFile with Response(0, {uris, choices}) and appends the request
# (method, title, options) to <log.json>, one JSON object per line. Choices are answered as a user
# would change them: every check button is toggled, every list gets its last item.
import json
import sys

import dbus
import dbus.service
from dbus.mainloop.glib import DBusGMainLoop
from gi.repository import GLib

BUS = "org.freedesktop.portal.Desktop"
PATH = "/org/freedesktop/portal/desktop"
IFACE = "org.freedesktop.portal.FileChooser"


def plain(v):
    if isinstance(v, (dbus.Array, list)):
        if v.signature == "y":
            return bytes(v).rstrip(b"\0").decode("utf-8", "replace")
        return [plain(x) for x in v]
    if isinstance(v, (dbus.Struct, tuple)):
        return [plain(x) for x in v]
    if isinstance(v, (dbus.Dictionary, dict)):
        return {str(k): plain(x) for k, x in v.items()}
    if isinstance(v, dbus.Boolean):
        return bool(v)
    if isinstance(v, (dbus.String, dbus.ObjectPath)):
        return str(v)
    if isinstance(v, (dbus.UInt32, dbus.Int32, dbus.UInt64, dbus.Int64, dbus.Byte)):
        return int(v)
    return v


class Request(dbus.service.Object):
    @dbus.service.signal("org.freedesktop.portal.Request", signature="ua{sv}")
    def Response(self, code, results):
        pass


class FileChooser(dbus.service.Object):
    def __init__(self, conn, log, uris):
        super().__init__(conn, PATH)
        self.conn, self.log, self.uris, self.n = conn, log, uris, 0

    def answer(self, method, sender, title, options):
        entry = {"method": method, "title": str(title), "options": plain(options)}
        with open(self.log, "a") as f:
            f.write(json.dumps(entry) + "\n")
        token = str(options.get("handle_token", "t%d" % self.n))
        self.n += 1
        path = "/org/freedesktop/portal/desktop/request/%s/%s" % (sender[1:].replace(".", "_"), token)
        req = Request(self.conn, path)
        choices = []
        for cid, _label, items, initial in options.get("choices", []):
            if len(items) == 0:
                choices.append((cid, "false" if initial == "true" else "true"))
            else:
                choices.append((cid, items[-1][0]))
        results = {"uris": dbus.Array(self.uris, signature="s")}
        if choices:
            results["choices"] = dbus.Array([dbus.Struct(c, signature="ss") for c in choices], signature="(ss)")

        def emit():
            req.Response(dbus.UInt32(0), dbus.Dictionary(results, signature="sv"))
            return False
        GLib.timeout_add(200, emit)  # after the caller has subscribed to the request path
        return dbus.ObjectPath(path)

    @dbus.service.method(IFACE, in_signature="ssa{sv}", out_signature="o", sender_keyword="sender")
    def OpenFile(self, parent, title, options, sender=None):
        return self.answer("OpenFile", sender, title, options)

    @dbus.service.method(IFACE, in_signature="ssa{sv}", out_signature="o", sender_keyword="sender")
    def SaveFile(self, parent, title, options, sender=None):
        return self.answer("SaveFile", sender, title, options)


def main():
    DBusGMainLoop(set_as_default=True)
    bus = dbus.SessionBus()
    name = dbus.service.BusName(BUS, bus, do_not_queue=True)  # noqa: F841 (keeps the name)
    FileChooser(bus, sys.argv[1], sys.argv[2:])
    print("mock portal ready", flush=True)
    GLib.MainLoop().run()


if __name__ == "__main__":
    main()
