/* AdobeWine CC installer polyfill */
/*
 * Prepended to CCDInstaller.js by scripts/cc-installer-polyfill.sh. Adds the three things
 * the Creative Cloud web installer 2.14.0.82 needs and Wine 11.18's mshtml/jscript lack:
 * Object.entries/values/assign, Element remove/replaceWith/before/after, and a settable
 * input.indeterminate. Each part only runs if the browser lacks it, so it does nothing
 * once Wine patches 0055-0057 are in the runtime. Strictly ES5 (the page runs in IE11 mode).
 * Based on NickPittas' polyfill.js in https://github.com/le-birnes/AdobeWine/issues/2
 */
(function () {
  function def(o, n, f) {
    if (!o || o[n]) return;
    try {
      Object.defineProperty(o, n, { value: f, writable: true, configurable: true, enumerable: false });
    } catch (e) {
      o[n] = f;
    }
  }
  function ownKeys(o) {
    var r = [], k;
    if (o == null) throw new TypeError('Cannot convert undefined or null to object');
    o = Object(o);
    for (k in o) if (Object.prototype.hasOwnProperty.call(o, k)) r.push(k);
    return r;
  }

  def(Object, 'entries', function (o) {
    var k = ownKeys(o), r = [], i;
    for (i = 0; i < k.length; i++) r.push([k[i], o[k[i]]]);
    return r;
  });
  def(Object, 'values', function (o) {
    var k = ownKeys(o), r = [], i;
    for (i = 0; i < k.length; i++) r.push(o[k[i]]);
    return r;
  });
  def(Object, 'assign', function (t) {
    var to, i, s, k, j;
    if (t == null) throw new TypeError('Cannot convert undefined or null to object');
    to = Object(t);
    for (i = 1; i < arguments.length; i++) {
      s = arguments[i];
      if (s == null) continue;
      k = ownKeys(s);
      for (j = 0; j < k.length; j++) to[k[j]] = s[k[j]];
    }
    return to;
  });

  if (typeof document === 'undefined') return;

  function toNode(a) {
    return (a && a.nodeType) ? a : document.createTextNode(String(a));
  }
  function frag(args) {
    var f = document.createDocumentFragment(), i;
    for (i = 0; i < args.length; i++) f.appendChild(toNode(args[i]));
    return f;
  }
  var protos = [], i, P;
  if (typeof Element !== 'undefined') protos.push(Element.prototype);
  if (typeof CharacterData !== 'undefined') protos.push(CharacterData.prototype);
  if (typeof DocumentType !== 'undefined') protos.push(DocumentType.prototype);
  for (i = 0; i < protos.length; i++) {
    P = protos[i];
    def(P, 'remove', function () {
      if (this.parentNode) this.parentNode.removeChild(this);
    });
    def(P, 'replaceWith', function () {
      var p = this.parentNode, n = this.nextSibling, f;
      if (!p) return;
      f = frag(arguments);
      if (this.parentNode === p) p.replaceChild(f, this);
      else p.insertBefore(f, n && n.parentNode === p ? n : null);
    });
    def(P, 'before', function () {
      var p = this.parentNode;
      if (p) p.insertBefore(frag(arguments), this);
    });
    def(P, 'after', function () {
      var p = this.parentNode;
      if (p) p.insertBefore(frag(arguments), this.nextSibling);
    });
  }

  /* Wine 11.18 throws E_NOTIMPL when input.indeterminate is set */
  try {
    document.createElement('input').indeterminate = false;
  } catch (e) {
    try {
      Object.defineProperty(HTMLInputElement.prototype, 'indeterminate', {
        configurable: true,
        enumerable: true,
        get: function () { return !!this.__adobewineIndeterminate; },
        set: function (v) { this.__adobewineIndeterminate = !!v; }
      });
    } catch (e2) {}
  }
})();
