# Windows DLL map for Adobe apps

Which Windows functions Adobe's software calls, and what Wine 11.20 with these patches gives it. Made by reading the import table of every binary in the Adobe install folders (a scanner run over the installed apps) and checking each import against Wine's exports .

**Scanned 2026-10-31:** 5664 binaries from 13 install groups (Acrobat, After Effects, Audition, Bridge, Common, Creative Cloud, Illustrator, InDesign, Lightroom, Lightroom Classic, Media Encoder, Photoshop, Premiere Pro), importing from 894 DLL names. 138 of those are DLLs Wine provides, with 6677 distinct functions imported from them. The other names are Adobe's own DLLs, or third-party ones.

**What this map proves, and what it doesn't.** It checks that a function exists and doesn't abort. It says nothing about whether every function behaves like Windows. Behaviour is covered by app test suites, by `tests/gapmap` for the functions below, and, where Windows behaviour had to be worked out, by the clean-room method in `patches/0097`: a behaviour description checked against Windows, no Microsoft code copied.

## Gaps found: 174

| result | functions |
|---|---|
| Implemented or forwarded (0101) | 27 |
| Answers "nothing here" (0101) | 18 |
| Fails cleanly (0102) | 59 |
| Not in Wine: belongs to DXVK / vkd3d | 3 |
| Not Wine's to supply: Microsoft VC++ runtime | 67 |

Only 3 of the 107 gaps in Wine-supplied DLLs stay open. `tests/gapmap` passes 23 of 23 on the patched runtime, and the regression tests shows no new failures.

## Implemented or forwarded (0101)

Does the work, or passes it to the Wine function that does.

| DLL | function | was | binaries | used by |
|---|---|---|---:|---|
| advapi32.dll | `RegDeleteKeyTransactedW` | missing | 2 | Common |
| advapi32.dll | `RegOpenKeyTransactedW` | missing | 14 | Acrobat, After Effects, Audition, Bridge, Common, Illustrator, InDesign, Lightroom, Lightroom Classic, Media Encoder, Photoshop, Premiere Pro |
| kernel32.dll | `CreateFileMappingNumaA` | missing | 220 | Media Encoder, Premiere Pro |
| mfplat.dll | `MFCreateMediaBufferWrapper` | stub | 11 | Acrobat, After Effects, Audition, Bridge, Common, Illustrator, InDesign, Media Encoder, Photoshop, Premiere Pro |
| ole32.dll | `HWND_UserFree64` | missing | 2 | Acrobat |
| ole32.dll | `HWND_UserMarshal64` | missing | 2 | Acrobat |
| ole32.dll | `HWND_UserSize64` | missing | 2 | Acrobat |
| ole32.dll | `HWND_UserUnmarshal64` | missing | 2 | Acrobat |
| oleaut32.dll | `BSTR_UserFree64` | missing | 2 | Acrobat |
| oleaut32.dll | `BSTR_UserMarshal64` | missing | 2 | Acrobat |
| oleaut32.dll | `BSTR_UserSize64` | missing | 2 | Acrobat |
| oleaut32.dll | `BSTR_UserUnmarshal64` | missing | 2 | Acrobat |
| oleaut32.dll | `LPSAFEARRAY_UserFree64` | missing | 1 | Acrobat |
| oleaut32.dll | `LPSAFEARRAY_UserMarshal64` | missing | 1 | Acrobat |
| oleaut32.dll | `LPSAFEARRAY_UserSize64` | missing | 1 | Acrobat |
| oleaut32.dll | `LPSAFEARRAY_UserUnmarshal64` | missing | 1 | Acrobat |
| oleaut32.dll | `VARIANT_UserFree64` | missing | 2 | Acrobat |
| oleaut32.dll | `VARIANT_UserMarshal64` | missing | 2 | Acrobat |
| oleaut32.dll | `VARIANT_UserSize64` | missing | 2 | Acrobat |
| oleaut32.dll | `VARIANT_UserUnmarshal64` | missing | 2 | Acrobat |
| propsys.dll | `VariantCompare` | stub | 13 | Acrobat, After Effects, Audition, Bridge, Common, Illustrator, InDesign, Lightroom Classic, Media Encoder, Photoshop, Premiere Pro |
| spoolss.dll | `ClosePrinter` | stub | 2 | Acrobat |
| spoolss.dll | `GetJobW` | stub | 2 | Acrobat |
| spoolss.dll | `GetPrinterW` | stub | 2 | Acrobat |
| spoolss.dll | `OpenPrinterW` | stub | 2 | Acrobat |
| spoolss.dll | `SetJobW` | stub | 2 | Acrobat |
| wkscli.dll | `NetWkstaGetInfo` | stub | 1 | Premiere Pro |

## Answers "nothing here" (0101)

Returns what a Windows machine without that feature returns: no packages, no pointer devices, no private namespaces, no enclaves.

| DLL | function | was | binaries | used by |
|---|---|---|---:|---|
| kernel32.dll | `AddSIDToBoundaryDescriptor` | missing | 3 | After Effects, Media Encoder, Premiere Pro |
| kernel32.dll | `ClosePrivateNamespace` | missing | 3 | After Effects, Media Encoder, Premiere Pro |
| kernel32.dll | `CreatePrivateNamespaceW` | missing | 3 | After Effects, Media Encoder, Premiere Pro |
| kernel32.dll | `DeleteBoundaryDescriptor` | missing | 3 | After Effects, Media Encoder, Premiere Pro |
| kernel32.dll | `FindPackagesByPackageFamily` | missing | 1 | Common |
| kernel32.dll | `GetPackageApplicationIds` | missing | 1 | Common |
| kernel32.dll | `IsEnclaveTypeSupported` | missing | 1 | Photoshop |
| kernel32.dll | `OpenPackageInfoByFullName` | missing | 1 | Common |
| kernel32.dll | `OpenPrivateNamespaceW` | missing | 3 | After Effects, Media Encoder, Premiere Pro |
| kernel32.dll | `PackageFamilyNameFromId` | missing | 1 | Common |
| kernel32.dll | `UnregisterApplicationRecoveryCallback` | missing | 3 | Illustrator, InDesign, Lightroom Classic |
| powrprof.dll | `PowerReadACValue` | missing | 10 | Acrobat, After Effects, Audition, Bridge, Common, Illustrator, InDesign, Media Encoder, Photoshop, Premiere Pro |
| user32.dll | `GetPointerDevice` | missing | 1 | Common |
| user32.dll | `GetPointerInputTransform` | missing | 2 | Common |
| user32.dll | `SkipPointerFrameMessages` | missing | 5 | After Effects, Illustrator, Media Encoder, Photoshop, Premiere Pro |
| userenv.dll | `DeriveAppContainerSidFromAppContainerName` | missing | 4 | Common, Lightroom Classic |
| wininet.dll | `InternetFreeCookies` | missing | 1 | Common |
| wininet.dll | `InternetGetCookieEx2` | missing | 1 | Common |

## Fails cleanly (0102)

WinUSB, Bluetooth GATT, ESE, WebSocket, filter manager, event tracing, TPM and some crypto calls. They used to abort the process when called; now they return the error a machine without that subsystem gives. **The subsystems themselves do not work.**

| DLL | function | was | binaries | used by |
|---|---|---|---:|---|
| atmlib.dll | `ATMGetFontPathsA` | stub | 1 | Acrobat |
| atmlib.dll | `ATMGetMenuNameA` | stub | 1 | Acrobat |
| atmlib.dll | `ATMGetMenuNameW` | stub | 1 | Acrobat |
| atmlib.dll | `ATMGetPostScriptNameA` | stub | 1 | Acrobat |
| atmlib.dll | `ATMGetPostScriptNameW` | stub | 1 | Acrobat |
| bluetoothapis.dll | `BluetoothGATTGetDescriptors` | stub | 10 | Acrobat, After Effects, Audition, Bridge, Common, Illustrator, InDesign, Media Encoder, Photoshop, Premiere Pro |
| bluetoothapis.dll | `BluetoothGATTRegisterEvent` | stub | 10 | Acrobat, After Effects, Audition, Bridge, Common, Illustrator, InDesign, Media Encoder, Photoshop, Premiere Pro |
| bluetoothapis.dll | `BluetoothGATTSetCharacteristicValue` | stub | 10 | Acrobat, After Effects, Audition, Bridge, Common, Illustrator, InDesign, Media Encoder, Photoshop, Premiere Pro |
| bluetoothapis.dll | `BluetoothGATTSetDescriptorValue` | stub | 10 | Acrobat, After Effects, Audition, Bridge, Common, Illustrator, InDesign, Media Encoder, Photoshop, Premiere Pro |
| bluetoothapis.dll | `BluetoothGATTUnregisterEvent` | stub | 10 | Acrobat, After Effects, Audition, Bridge, Common, Illustrator, InDesign, Media Encoder, Photoshop, Premiere Pro |
| crypt32.dll | `CryptDecryptMessage` | stub | 1 | Acrobat |
| crypt32.dll | `CryptExportPublicKeyInfoFromBCryptKeyHandle` | missing | 2 | Photoshop |
| esent.dll | `JetAttachDatabase2W` | stub | 13 | Acrobat, After Effects, Audition, Bridge, Common, Illustrator, InDesign, Lightroom Classic, Media Encoder, Photoshop, Premiere Pro |
| esent.dll | `JetBeginSessionW` | stub | 13 | Acrobat, After Effects, Audition, Bridge, Common, Illustrator, InDesign, Lightroom Classic, Media Encoder, Photoshop, Premiere Pro |
| esent.dll | `JetCloseDatabase` | stub | 1 | Common |
| esent.dll | `JetCloseTable` | stub | 13 | Acrobat, After Effects, Audition, Bridge, Common, Illustrator, InDesign, Lightroom Classic, Media Encoder, Photoshop, Premiere Pro |
| esent.dll | `JetCreateInstanceW` | stub | 13 | Acrobat, After Effects, Audition, Bridge, Common, Illustrator, InDesign, Lightroom Classic, Media Encoder, Photoshop, Premiere Pro |
| esent.dll | `JetDetachDatabase2W` | stub | 1 | Common |
| esent.dll | `JetEndSession` | stub | 1 | Common |
| esent.dll | `JetGetSystemParameterW` | stub | 1 | Common |
| esent.dll | `JetGetTableColumnInfoW` | stub | 13 | Acrobat, After Effects, Audition, Bridge, Common, Illustrator, InDesign, Lightroom Classic, Media Encoder, Photoshop, Premiere Pro |
| esent.dll | `JetInit` | stub | 13 | Acrobat, After Effects, Audition, Bridge, Common, Illustrator, InDesign, Lightroom Classic, Media Encoder, Photoshop, Premiere Pro |
| esent.dll | `JetMove` | stub | 13 | Acrobat, After Effects, Audition, Bridge, Common, Illustrator, InDesign, Lightroom Classic, Media Encoder, Photoshop, Premiere Pro |
| esent.dll | `JetOpenDatabaseW` | stub | 13 | Acrobat, After Effects, Audition, Bridge, Common, Illustrator, InDesign, Lightroom Classic, Media Encoder, Photoshop, Premiere Pro |
| esent.dll | `JetOpenTableW` | stub | 13 | Acrobat, After Effects, Audition, Bridge, Common, Illustrator, InDesign, Lightroom Classic, Media Encoder, Photoshop, Premiere Pro |
| esent.dll | `JetRetrieveColumn` | stub | 13 | Acrobat, After Effects, Audition, Bridge, Common, Illustrator, InDesign, Lightroom Classic, Media Encoder, Photoshop, Premiere Pro |
| esent.dll | `JetSetSystemParameterW` | stub | 13 | Acrobat, After Effects, Audition, Bridge, Common, Illustrator, InDesign, Lightroom Classic, Media Encoder, Photoshop, Premiere Pro |
| esent.dll | `JetTerm` | stub | 13 | Acrobat, After Effects, Audition, Bridge, Common, Illustrator, InDesign, Lightroom Classic, Media Encoder, Photoshop, Premiere Pro |
| fltlib.dll | `FilterGetMessage` | stub | 1 | Common |
| fltlib.dll | `FilterReplyMessage` | stub | 1 | Common |
| fltlib.dll | `FilterSendMessage` | stub | 1 | Common |
| ncrypt.dll | `NCryptCreateClaim` | stub | 1 | Photoshop |
| ncrypt.dll | `NCryptDeriveKey` | stub | 2 | Acrobat |
| ncrypt.dll | `NCryptSecretAgreement` | stub | 2 | Acrobat |
| rpcrt4.dll | `I_RpcOpenClientProcess` | missing | 1 | Common |
| rpcrt4.dll | `NdrStubCall3` | missing | 1 | Acrobat |
| tbs.dll | `Tbsi_Get_TCG_Log_Ex` | stub | 1 | Photoshop |
| tdh.dll | `TdhGetEventInformation` | stub | 1 | Common |
| tdh.dll | `TdhGetProperty` | stub | 1 | Common |
| tdh.dll | `TdhGetPropertySize` | stub | 1 | Common |
| websocket.dll | `WebSocketBeginClientHandshake` | stub | 1 | Lightroom |
| websocket.dll | `WebSocketBeginServerHandshake` | stub | 1 | Lightroom |
| websocket.dll | `WebSocketCompleteAction` | stub | 1 | Lightroom |
| websocket.dll | `WebSocketCreateServerHandle` | stub | 1 | Lightroom |
| websocket.dll | `WebSocketEndClientHandshake` | stub | 1 | Lightroom |
| websocket.dll | `WebSocketEndServerHandshake` | stub | 1 | Lightroom |
| websocket.dll | `WebSocketGetAction` | stub | 1 | Lightroom |
| websocket.dll | `WebSocketReceive` | stub | 1 | Lightroom |
| websocket.dll | `WebSocketSend` | stub | 1 | Lightroom |
| wevtapi.dll | `EvtUpdateBookmark` | stub | 1 | Common |
| winusb.dll | `WinUsb_ControlTransfer` | stub | 13 | Acrobat, After Effects, Audition, Bridge, Common, Illustrator, InDesign, Lightroom Classic, Media Encoder, Photoshop, Premiere Pro |
| winusb.dll | `WinUsb_GetAssociatedInterface` | stub | 13 | Acrobat, After Effects, Audition, Bridge, Common, Illustrator, InDesign, Lightroom Classic, Media Encoder, Photoshop, Premiere Pro |
| winusb.dll | `WinUsb_GetOverlappedResult` | stub | 13 | Acrobat, After Effects, Audition, Bridge, Common, Illustrator, InDesign, Lightroom Classic, Media Encoder, Photoshop, Premiere Pro |
| winusb.dll | `WinUsb_Initialize` | stub | 13 | Acrobat, After Effects, Audition, Bridge, Common, Illustrator, InDesign, Lightroom Classic, Media Encoder, Photoshop, Premiere Pro |
| winusb.dll | `WinUsb_ReadPipe` | stub | 13 | Acrobat, After Effects, Audition, Bridge, Common, Illustrator, InDesign, Lightroom Classic, Media Encoder, Photoshop, Premiere Pro |
| winusb.dll | `WinUsb_ResetPipe` | stub | 13 | Acrobat, After Effects, Audition, Bridge, Common, Illustrator, InDesign, Lightroom Classic, Media Encoder, Photoshop, Premiere Pro |
| winusb.dll | `WinUsb_SetCurrentAlternateSetting` | stub | 13 | Acrobat, After Effects, Audition, Bridge, Common, Illustrator, InDesign, Lightroom Classic, Media Encoder, Photoshop, Premiere Pro |
| winusb.dll | `WinUsb_WritePipe` | stub | 13 | Acrobat, After Effects, Audition, Bridge, Common, Illustrator, InDesign, Lightroom Classic, Media Encoder, Photoshop, Premiere Pro |
| wmvcore.dll | `WMCreateIndexer` | stub | 7 | After Effects, Audition, Bridge, Media Encoder, Photoshop, Premiere Pro |

## Not in Wine: belongs to DXVK / vkd3d

Imported by name, not provided by Wine; the D3D DLLs in a prefix are DXVK's and vkd3d's, so a fix belongs there.

| DLL | function | was | binaries | used by |
|---|---|---|---:|---|
| d3d11.dll | `CreateDirect3D11DeviceFromDXGIDevice` | missing | 9 | Illustrator, InDesign, Lightroom, Lightroom Classic |
| d3d11.dll | `CreateDirect3D11SurfaceFromDXGISurface` | missing | 6 | Illustrator, InDesign, Lightroom, Lightroom Classic |
| d3d12.dll | `D3D12GetInterface` | missing | 1 | Acrobat |

## Not Wine's to supply: Microsoft VC++ runtime

Apps ship these DLLs themselves or get them from the VC++ redistributable. Wine's builtins are not used.

| DLL | functions |
|---|---|
| concrt140.dll | 3 |
| msvcp140.dll | 12 |
| msvcr100.dll | 2 |
| msvcr110.dll | 17 |
| msvcr120.dll | 19 |
| msvcr90.dll | 1 |
| vccorlib140.dll | 7 |
| vcruntime140.dll | 6 |
