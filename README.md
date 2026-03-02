# Grafický Engine Demo

Demo projekt(y), který používá můj [Grafický Engine](https://github.com/bagons/graphicengine)

## Dema

**DEMO0** - Unit testy

**DEMO1** - Crytek Sponza + First Person Controller
- *Ovládání:* **WSAD**, **SPACE** = Skok, **SHIFT** = Běh, **F11** = fullscreen, **ESC** = uvolnění myší, **LEVÉ TLAČÍTKO MYŠI** = znovu zamknout myš
- *PS*: chvíli může trvat, než se velký model načte

**DEMO2** - Velmi jednoduché demo s rotující kostkou, která má na sobě aplikovanou Normal Mapu. Pomocí **MEZERY** jde přepínat Normal Mapa (s ní / bez ní)


## Instalace

Stačí zkompilovat projekt pomocí CMakeLists.txt souboru.

```bash
git clone https://github.com/bagons/ge-demos.git
cd ge-demos
cmake .
cmake --build .
```

## Požadavky

Může se stát, že nebudete mít všechny požadavky na vytvoření okna, jsou to [požadavky knihovny GLFW](https://www.glfw.org/docs/latest/compat.html), kterou engine používá. Naštěstí vás na to upozorní a po případné instalaci nedostatků vše funguje.